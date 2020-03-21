/*
 * client.h - general client header
 *
 * Copyright (c) 2019 Xichen Zhou
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef TW_CLIENT_H
#define TW_CLIENT_H

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <pthread.h>
#include <poll.h>
#include <GL/gl.h>
//hopefully this shit is declared
#include <wayland-client.h>
#include <sys/timerfd.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-names.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <wayland-util.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>

#include "event_queue.h"
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * global context for one application (can be shared with multiple surface)
 *
 * It contains almost everything and app_surface should have a reference of
 * this.
 *
 * This struct is indeed rather big (currently 488 bytes), we would want to fit
 * in L1 Cache
 */
struct tw_globals {
	struct wl_compositor *compositor;
	struct wl_display *display;
	struct wl_shm *shm;
	struct wl_data_device_manager *wl_data_device_manager;
	enum wl_shm_format buffer_format;
	struct wl_inputs {
		struct wl_seat *wl_seat;
		struct wl_keyboard *wl_keyboard;
		struct wl_pointer *wl_pointer;
		struct wl_touch *wl_touch;
		struct wl_data_device *wl_data_device;
		struct wl_data_offer *wl_data_offer;
		struct itimerspec repeat_info;
		char name[64]; //seat name
		uint32_t millisec;
		uint32_t serial;
		uint32_t mime_offered;
		//keyboard
		struct {
			struct xkb_context *kcontext;
			struct xkb_keymap  *keymap;
			struct xkb_state   *kstate;
			struct wl_surface *keyboard_focused;
			//state
			uint32_t modifiers;
			bool key_pressed;
			xkb_keycode_t keycode;
			xkb_keysym_t keysym;
		};
		//pointer, touch use this as well
		struct {
			char cursor_theme_name[64];
			struct wl_cursor *cursor;
			struct wl_cursor_theme *cursor_theme;
			struct wl_surface *cursor_surface;
			struct wl_buffer *cursor_buffer;
			struct wl_callback_listener cursor_done_listener;
			struct wl_surface *pointer_focused; //the surface that cursor is on
			//state
			uint32_t btn;
			bool btn_pressed;
			uint32_t enter_serial;
			uint32_t pointer_events;
			int16_t sx, sy; //screen coordinates
			int16_t dx, dy; //relative motion
			uint32_t dx_axis, dy_axis; //axis advance
			short w;
		};

	} inputs;

	//application theme settings
	struct tw_theme_color theme;
	struct tw_event_queue event_queue;
};


/* here you need a data structure that glues the anonoymous_buff and wl_buffer wl_shm_pool */
int tw_globals_announce(struct tw_globals *globals,
			struct wl_registry *wl_registry,
			uint32_t name,
			const char *interface,
			uint32_t version);

//unless you have better method to setup the theme, I think you can simply set it up my hand

/* Constructor */
void tw_globals_init(struct tw_globals *globals, struct wl_display *display);
/* destructor */
void tw_globals_release(struct tw_globals *globals);

static inline void
tw_globals_dispatch_event_queue(struct tw_globals *globals)
{
	tw_event_queue_run(&globals->event_queue);
}

bool is_shm_format_valid(uint32_t format);

//we can have several input code
void tw_globals_get_input_state(const struct tw_globals *globals);
//you will have a few functions

void tw_globals_receive_data_offer(struct wl_data_offer *offer,
				   struct wl_surface *surface,
				   bool drag_n_drop);



#ifdef __cplusplus
}
#endif



#endif /* EOF */
