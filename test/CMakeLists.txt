add_executable(test_nk_cairo
  test_nk_cairo.c
  )
target_link_libraries(test_nk_cairo
  twclient::nkcairo
  )

add_executable(test_nk_egl
  test_nk_egl.c
  )
target_link_libraries(test_nk_egl
  twclient::nkegl
  )

add_executable(test_queue
  test_queue.c
  )
target_link_libraries(test_queue
  twclient
  )

add_executable(test_inotify
  test_event_queue.c
  )
target_link_libraries(test_inotify
  twclient::twclient
  )

add_executable(test_udev
  test_udev.c
  )
target_link_libraries(test_udev
  twclient::twclient
  )

add_executable(test_stbimage
  test_stb_image.c
  )
target_link_libraries(test_stbimage
  Wayland::Client
  twclient::icons
  )

add_executable(test_desktop_entry
  test_dot_desktop.c
  )
target_link_libraries(test_desktop_entry
  twclient::icons
  )

add_executable(test_readtheme
  test_read_theme.c
  )
target_link_libraries(test_readtheme
  Wayland::Client
  twclient::icons
  )

add_executable(test_font_config
  test_fontconfig.c
  )
target_link_libraries(test_font_config
  Fontconfig::Fontconfig
  Freetype::Freetype
  Cairo::Cairo
  ctypes
  )
