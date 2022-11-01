cmake_minimum_required( VERSION 3.12 )
include( $ENV{PICO_SDK_PATH}/pico_sdk_init.cmake )
project(slave)
pico_sdk_init()

add_executable(slave slave.c)
target_link_libraries(slave pico_stdlib pico_cyw43_arch_lwip_poll)
pico_add_extra_outputs(slave)