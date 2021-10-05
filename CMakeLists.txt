cmake_minimum_required(VERSION 3.10)

project(wh1409_driver)

add_executable(parse_text ./src/parse_text.c)
set_target_properties(parse_text PROPERTIES OUTPUT_NAME "wh1409parse_ts")
target_sources(
  parse_text PUBLIC
  ./src/logging.c
  ./src/pen.c
)

target_include_directories(parse_text PUBLIC ./src)

add_executable(usbmon ./src/monitor_usb.c)
set_target_properties(usbmon PROPERTIES OUTPUT_NAME "wh1409_usbmon")
target_link_libraries(usbmon -lusb-1.0)
