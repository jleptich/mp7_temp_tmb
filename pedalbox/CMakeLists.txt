# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)
 
# ---- Board and SDK ----
set(PICO_BOARD pico)                     # original Raspberry Pi Pico (RP2040)
 
# pico_sdk_import.cmake must sit next to this file.
# Copy it from <pico-sdk>/external/pico_sdk_import.cmake
include(pico_sdk_import.cmake)
 
project(thermo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)               # C++20 for designated initializers (.priority = ...)
 
pico_sdk_init()
 
# ---- Sources ----
# Picks up every .c and .cpp file in this folder (main, init, can2040, ...).
# If you add a new source file, re-run cmake so it gets found.
file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
 
add_executable(thermo ${PROJECT_SOURCES})
 
target_include_directories(thermo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
 
# ---- SDK libraries (one per hardware/xxx.h you include) ----
target_link_libraries(thermo
    pico_stdlib
    hardware_gpio
    hardware_irq
    hardware_pwm
    hardware_uart
    hardware_watchdog
    hardware_timer
    hardware_dma
    hardware_pio                         # can2040 runs on a PIO block
    hardware_clocks
)
 
# ---- stdio ----
pico_enable_stdio_usb(thermo 1)          # printf over USB
pico_enable_stdio_uart(thermo 0)         # keep UART stdio off GP0/GP1 (START2 / EOC2)
 
# ---- Outputs: .uf2 (drag-and-drop), .bin, .hex, .map ----
pico_add_extra_outputs(thermo)
