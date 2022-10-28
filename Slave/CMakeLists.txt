cmake_minimum_required( VERSION 3.12 )
include( $ENV{PICO_SDK_PATH}/pico_sdk_init.cmake )
pico_sdk_init()
project(slave)

add_executable(slave slave.c)
target_link_libraries(slave pico_stdlib pico_cyw43_arch_lwip_threadsafe_background)
pico_add_extra_outputs(slave)