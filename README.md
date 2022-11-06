# AIOC
This is the Ham Radio *All-in-one-Cable*

![AIOC with Wouxun and Direwolf](doc/images/k1-aioc-wouxun.jpg?raw=true "AIOC with Wouxun and Direwolf")

## What does it do?
The AIOC is a small adapter with a USB-C connector that enumerates itself as a sound-card (e.g. for APRS purposes) 
and a virtual tty ("COM Port") for programming and asserting the PTT (Push-To-Talk).

## Features ##
- Digital mode interface (similar to digirig)
- Programming Cable Function via virtual Serial Port
- Compact form-factor (DIY molded enclosure TBD)
- Based on easy-to-handle STM32F302 using internal ADC/DAC
- Tested with Wouxun UV-9D Mate and Baofeng UV-5R
- Should theoretically work with APRSdroid on Android, however (yet) without PTT support

## Future Work ##
- Enclosure (DIY using 3D-Printed mold and Resin)
- Finalizing Rev. B Schematic
- Maybe integrate a TNC Modem with KISS interface? (I am not sure if that is worth the effort)
