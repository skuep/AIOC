#############################################################################################################################
# file:  CMakeLists.txt
# brief: Template "CMakeLists.txt" for building of executables and static libraries.
#
# usage: Edit "VARIABLES"-section to suit project requirements.
#        For debug build:
#          cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Debug -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
#          make -C Debug VERBOSE=1 -j $(nproc)
#        For release build:
#          cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Release -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
#          make -C Release VERBOSE=1 -j $(nproc)
#############################################################################################################################
cmake_minimum_required(VERSION 3.20)

###################### CONSTANTS ######################################
set (PROJECT_TYPE_EXECUTABLE          "exe")
set (PROJECT_TYPE_STATIC_LIBRARY      "static-lib")
set (MCPU_CORTEX_M0				      "-mcpu=cortex-m0")
set (MCPU_CORTEX_M0PLUS				  "-mcpu=cortex-m0plus")
set (MCPU_CORTEX_M3				      "-mcpu=cortex-m3")
set (MCPU_CORTEX_M4				      "-mcpu=cortex-m4")
set (MCPU_CORTEX_M7				      "-mcpu=cortex-m7")
set (MCPU_CORTEX_M33				  "-mcpu=cortex-m33")
set (MCPU_CORTEX_M55				  "-mcpu=cortex-m55")
set (MCPU_CORTEX_M85				  "-mcpu=cortex-m85")
set (MFPU_FPV4_SP_D16                 "-mfpu=fpv4-sp-d16")
set (MFPU_FPV5_D16                    "-mfpu=fpv5-d16")
set (RUNTIME_LIBRARY_REDUCED_C        "--specs=nano.specs")
set (RUNTIME_LIBRARY_STD_C            "")
set (RUNTIME_LIBRARY_SYSCALLS_MINIMAL "--specs=nosys.specs")
set (RUNTIME_LIBRARY_SYSCALLS_NONE    "")
set (MFLOAT_ABI_SOFTWARE              "-mfloat-abi=soft")
set (MFLOAT_ABI_HARDWARE              "-mfloat-abi=hard")
set (MFLOAT_ABI_MIX                   "-mfloat-abi=softfp")
#######################################################################

###################### VARIABLES ######################################
set (PROJECT_NAME             "aioc-fw")
set (PROJECT_TYPE             "exe")
set (LINKER_SCRIPT            "../stm32f30_flash.ld")
set (MCPU                     "-mcpu=Cortex-M4")
set (MFPU                 "-mfpu=fpv4-sp-d16")
set (MFLOAT_ABI               "")
set (RUNTIME_LIBRARY          "--specs=nano.specs")
set (RUNTIME_LIBRARY_SYSCALLS "--specs=nosys.specs")


set (PROJECT_SOURCES
	# LIST SOURCE FILES HERE
	Src/startup_stm32f302xc.s
	Src/io.c
	Src/led.c
	Src/main.c
	Src/settings.c
	Src/system_stm32f3xx.c
	Src/usb_audio.c
	Src/usb.c
	Src/usb_descriptors.c
	Src/usb_dfu.c
	Src/usb_hid.c
	Src/usb_serial.c

	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_can.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cec.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_comp.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_crc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_crc_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dac.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dac_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dma.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_exti.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_hrtim.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2s.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2s_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_irda.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_iwdg.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_nand.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_nor.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_opamp.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_opamp_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pccard.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pcd.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pcd_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rtc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rtc_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_sdadc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_smartcard.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_smartcard_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_smbus.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_spi.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_spi_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_sram.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tsc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_usart.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_usart_ex.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_wwdg.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_adc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_comp.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_crc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_dac.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_dma.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_exti.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_fmc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_gpio.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_hrtim.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_i2c.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_opamp.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_pwr.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_rcc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_rtc.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_spi.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_tim.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_usart.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_usb.c
	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_utils.c

	Middlewares/Third-Party/tinyusb/src/class/audio/audio_device.c
	Middlewares/Third-Party/tinyusb/src/class/bth/bth_device.c
	Middlewares/Third-Party/tinyusb/src/class/cdc/cdc_device.c
	Middlewares/Third-Party/tinyusb/src/class/cdc/cdc_host.c
	Middlewares/Third-Party/tinyusb/src/class/cdc/cdc_rndis_host.c
	Middlewares/Third-Party/tinyusb/src/class/dfu/dfu_device.c
	Middlewares/Third-Party/tinyusb/src/class/dfu/dfu_rt_device.c
	Middlewares/Third-Party/tinyusb/src/class/hid/hid_device.c
	Middlewares/Third-Party/tinyusb/src/class/hid/hid_host.c
	Middlewares/Third-Party/tinyusb/src/class/midi/midi_device.c
	Middlewares/Third-Party/tinyusb/src/class/msc/msc_device.c
	Middlewares/Third-Party/tinyusb/src/class/msc/msc_host.c
	Middlewares/Third-Party/tinyusb/src/class/net/ecm_rndis_device.c
	Middlewares/Third-Party/tinyusb/src/class/net/ncm_device.c
	Middlewares/Third-Party/tinyusb/src/class/usbtmc/usbtmc_device.c
	Middlewares/Third-Party/tinyusb/src/class/vendor/vendor_device.c
	Middlewares/Third-Party/tinyusb/src/class/vendor/vendor_host.c
	Middlewares/Third-Party/tinyusb/src/class/video/video_device.c
	Middlewares/Third-Party/tinyusb/src/common/tusb_fifo.c
	Middlewares/Third-Party/tinyusb/src/device/usbd.c
	Middlewares/Third-Party/tinyusb/src/device/usbd_control.c
	Middlewares/Third-Party/tinyusb/src/host/hub.c
	Middlewares/Third-Party/tinyusb/src/host/usbh.c
	Middlewares/Third-Party/tinyusb/src/tusb.c
	Middlewares/Third-Party/tinyusb/src/typec/usbc.c
	Middlewares/Third-Party/tinyusb/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c
	Middlewares/Third-Party/tinyusb/src/portable/st/typec/typec_stm32.c
	)

set (PROJECT_DEFINES
	# LIST COMPILER DEFINITIONS HERE
	STM32F302xC
	CFG_TUSB_MCU=OPT_MCU_STM32F3
	USER_VECT_TAB_ADDRESS
    )

set (PROJECT_INCLUDES
	# LIST INCLUDE DIRECTORIES HERE
	Drivers/CMSIS/Include
	Drivers/CMSIS/Device/ST/STM32F3xx/Include
	Drivers/STM32F3xx_HAL_Driver/Inc
	Middlewares/Third-Party/tinyusb/src
	Src
	Inc
    )

############ MODIFY ACCORDING TO REQUIREMENTS) ########################

#######################################################################

################## PROJECT SETUP ######################################
project(${PROJECT_NAME})
enable_language(ASM)

if (${PROJECT_TYPE} MATCHES ${PROJECT_TYPE_EXECUTABLE})
  add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
  add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)
  add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
  )

elseif (${PROJECT_TYPE} MATCHES ${PROJECT_TYPE_STATIC_LIBRARY})
  add_library(${PROJECT_NAME} ${PROJECT_SOURCES})
endif()

add_compile_definitions (${PROJECT_DEFINES})
include_directories (${PROJECT_INCLUDES})

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_STATIC_LIBRARY_SUFFIX ".a")
# Removed ${RUNTIME_LIBRARY} from CMAKE_C_FLAGS
set (CMAKE_C_FLAGS "${MCPU} -std=gnu11 ${MFPU} ${MFLOAT_ABI} -mthumb -Wall -Werror")
# Removed ${RUNTIME_LIBRARY_SYSCALLS} from CMAKE_EXE_LINKER_FLAGS
set (CMAKE_EXE_LINKER_FLAGS "-T${LINKER_SCRIPT} -Wl,-Map=test.map -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group")
set (CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")
