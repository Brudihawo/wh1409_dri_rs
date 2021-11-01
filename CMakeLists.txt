cmake_minimum_required(VERSION 3.10)

project(wh1409_driver)

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -Wall")

add_executable(parse_text ./src/parse_text.c)
set_target_properties(parse_text PROPERTIES OUTPUT_NAME "wh1409parse_ts")
target_sources(
  parse_text PUBLIC
  ./src/logging.c
  ./src/pen.c
  ./src/globals.c
)
target_include_directories(parse_text PUBLIC ./src)

add_executable(usbmon ./src/monitor_usb.c)
target_sources(
  usbmon PUBLIC
  ./src/logging.c
  ./src/pen.c
  ./src/util.c
  ./src/globals.c
)
target_include_directories(usbmon PUBLIC ./src)
set_target_properties(usbmon PROPERTIES OUTPUT_NAME "wh1409_usbmon")
target_link_libraries(usbmon -lusb-1.0 -ludev)

message(STATUS "BUILD TYPE: ${CMAKE_BUILD_TYPE}")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_compile_definitions("DEBUG")
  message(STATUS "Adding preprocessor definition '#define DEBUG' in debug mode")
endif()
