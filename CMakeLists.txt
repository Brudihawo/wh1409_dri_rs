cmake_minimum_required(VERSION 3.10)

project(wh1409_driver)

add_executable(parse_text ./src/parse_text.c)
target_sources(parse_text PUBLIC ./src/logging.c)
target_include_directories(parse_text PUBLIC ./src)
