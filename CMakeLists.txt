cmake_minimum_required(VERSION 3.13.1)

set(BOARD,nrf52840_pca10056)
set(CONF_FILE,prj.conf)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(lost60)

target_sources(app PRIVATE
  src/main.c
  src/bas.c
  src/hog.c
)