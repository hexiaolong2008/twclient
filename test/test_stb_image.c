#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <cairo/cairo.h>
#include <wayland-util.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#include <stb/stb_image.h>
#include <dirent.h>
#include <sys/types.h>
#include <sequential.h>
#include <os/file.h>

union argb {
	//byte order readed
	struct {
		uint8_t r, g, b, a;
	} data;
	uint32_t code;
};

static void
load_image(const char *input_path, const char *output_path)
{
	int w, h, channels;
	//you have no choice but to load rgba, cairo deal with 32 bits only
	unsigned char *pixels = stbi_load(input_path, &w, &h, &channels, STBI_rgb_alpha);


	//whats really loaded here is abgr(in the order of cairo)
	uint32_t *uint_pixels = (uint32_t *)pixels;
	union argb pixel;
	for (int i = 0; i < w * h; i++ ) {
		pixel.code = *(uint_pixels+i);
		*(uint_pixels+i) = ((uint32_t)pixel.data.a << 24) +
			((uint32_t)pixel.data.r << 16) +
			((uint32_t)pixel.data.g << 8) +
			((uint32_t)pixel.data.b);
	}
	cairo_surface_t *image_surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1000, 1000);
	cairo_surface_t *src = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_ARGB32, w, h,
								   cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w));
	cairo_t *cr = cairo_create(image_surf);
	//clean up surface
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
	cairo_set_source_surface(cr, src, 0, 0);
	cairo_rectangle(cr, 0, 0, 1000, 1000);
	cairo_fill(cr);

	cairo_destroy(cr);
	cairo_surface_write_to_png(image_surf, output_path);
	cairo_surface_destroy(image_surf);
	cairo_surface_destroy(src);
	stbi_image_free(pixels);
}


static void
nk_wl_build_theme_images(struct wl_array *handle_pool, struct wl_array *string_pool,
			 const char *output_path)
{
	size_t nimages = handle_pool->size / sizeof(uint64_t);
	stbrp_rect *rects = malloc(sizeof(stbrp_rect) * nimages);
	stbrp_node *nodes = malloc(sizeof(stbrp_node) * 2020);
	stbrp_context context;
	stbrp_init_target(&context, 2000, 2000, nodes, 2020);
	stbrp_setup_allow_out_of_mem(&context, 0);
	cairo_surface_t *image_surface;

	//first pass: get the rects in places.
	for (int i = 0; i < nimages; i++) {
		int pos;
		int x, y, comp; //channels
		pos = *((uint64_t *)handle_pool->data + i);
		const char *path =
			(const char *)string_pool->data + pos;
		stbi_info(path, &x, &y, &comp);
		rects[i].w = x; rects[i].h = y;
	}
	stbrp_pack_rects(&context, rects, nimages);
	image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2000, 2000);
	cairo_t *cr = cairo_create(image_surface);


	//second pass: load images
	for (int i = 0; i < nimages; i++) {
		intptr_t pos;
		int x, y, comp; //channels
		pos = *((uint64_t *)handle_pool->data + i);
		const char *path =
			(const char *)string_pool->data + pos;
		unsigned char *pixels = stbi_load(path, &x, &y, &comp, STBI_rgb_alpha);
		//now we copy the image to desired location
		cairo_surface_t *source =  cairo_image_surface_create_for_data(
			pixels, CAIRO_FORMAT_ARGB32, x, y,
			cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, x));

		//now use cairo to render to sublocations
		cairo_set_source_surface(cr, source, 0, 0);
		cairo_rectangle(cr, rects[i].x, rects[i].y, rects[i].w, rects[i].h);
		cairo_fill(cr);
		/* cairo_paint(cr); */
		/* cairo_surface_flush(image_surface); */
		cairo_surface_destroy(source);
		stbi_image_free(pixels);
	}
	cairo_destroy(cr);
	cairo_surface_write_to_png(image_surface, output_path);
	cairo_surface_destroy(image_surface);
	free(nodes);
	free(rects);
}

static int filter_image(const struct dirent *file)
{
	if (strstr(file->d_name, ".png"))
		return true;
	return false;
}

static void
build_image_strings(const char *path, struct wl_array *handle_pool, struct wl_array *string_pool)
{
	struct dirent **namelist;
	int n;
	long totallen = 0;

	wl_array_init(handle_pool);
	wl_array_init(string_pool);
	n = scandir(path, &namelist, filter_image, alphasort);
	if (n < 0) {
		perror("scandir");
		return;
	}
	wl_array_add(handle_pool, n * sizeof(uint64_t));
	for (int i = 0; i < n; i++) {
		*((uint64_t *)handle_pool->data + i) = totallen;
		totallen += strlen(path) + 1 + strlen(namelist[i]->d_name) + 1;
		wl_array_add(string_pool, totallen);
		sprintf((char *)string_pool->data + *((uint64_t*)handle_pool->data+i),
			"%s/%s",path, namelist[i]->d_name);
		free(namelist[i]);
	}
	free(namelist);
}

int main(int argc, char *argv[])
{
	struct wl_array handle_pool, string_pool;
	build_image_strings(argv[1], &handle_pool, &string_pool);
	nk_wl_build_theme_images(&handle_pool, &string_pool, argv[2]);

	wl_array_release(&handle_pool);
	wl_array_release(&string_pool);
	return 0;
}
