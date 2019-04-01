# Boilerplate code, which pulls in the Zephyr build system.
cmake_minimum_required(VERSION 3.13.1)
# set(CMAKE_SYSTEM_NAME Generic)
set(CONF_FILE prj.conf)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(Lost60)

#zephyr_library_include_directories($ENV{ZEPHYR_BASE}/samples/bluetooth)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  ../gatt/hog.c
  ../gatt/bas.c
  )

zephyr_library_include_directories($ENV{ZEPHYR_BASE}/samples/bluetooth)

# Add your source file to the "app" target. This must come after
# the boilerplate code, which defines the target.
#target_sources(app PRIVATE src/main.c)