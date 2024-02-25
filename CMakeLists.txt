
project(Pico-FreeRTOS C CXX ASM)


add_executable(firmware
    main.c    
)
target_link_libraries( firmware 
        pico_stdlib
        hardware_gpio 
        hardware_pwm
        freertos
    
 )
 pico_add_extra_outputs(firmware)

pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 0)
