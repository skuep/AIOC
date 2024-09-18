[![](https://dcbadge.vercel.app/api/server/wCbXu9R95C?style=flat&theme=default-inverted)](https://discord.gg/wCbXu9R95C)
[![](https://img.shields.io/github/stars/skuep/AIOC)](https://github.com/skuep/AIOC/stargazers)
[![](https://img.shields.io/github/v/release/skuep/AIOC?sort=semver)](https://github.com/skuep/AIOC/releases)
[![](https://img.shields.io/github/license/skuep/AIOC)](https://github.com/skuep/AIOC/blob/master/LICENSE.md)

# AIOC
This is the Ham Radio *All-in-one-Cable*. **It is currently still being tested!** Please read this README carefully before ordering anything.

![AIOC with Wouxun and Direwolf](doc/images/k1-aioc-wouxun.jpg?raw=true "AIOC with Wouxun and Direwolf")

## What does it do?
The AIOC is a small adapter with a USB-C connector that enumerates itself as a sound-card (e.g. for APRS purposes), a virtual tty ("COM Port") for programming and asserting the PTT (Push-To-Talk) as well as a CM108 compatible HID endpoint for CM108-style PTT (new in firmware version 1.2.0).

You can watch the videos of the *Temporarily Offline* and *HAM RADIO DUDE* YouTube channels below.

[![All In One Cable AIOC - Ham Nuggets Season 4 Episode 8 S04E08](http://img.youtube.com/vi/RZjoPNe634o/0.jpg)](http://www.youtube.com/watch?v=RZjoPNe634o "All In One Cable AIOC - Ham Nuggets Season 4 Episode 8 S04E08")
[![Your BAOFENG Programming Cable Sucks! - Get This! - AIOC All in One Cable](http://img.youtube.com/vi/xRCmXQYRLE0/0.jpg)](http://www.youtube.com/watch?v=xRCmXQYRLE0 "Your BAOFENG Programming Cable Sucks! - Get This! - AIOC All in One Cable")

## Features ##
- Cheap & Hackable Digital mode USB interface (similar to digirig, mobilinkd, etc...)
- Programming Cable Function via virtual Serial Port
- Compact form-factor (DIY overmolded enclosure is currently TBD)
- Based on easy-to-hack **STM32F302** with internal ADC/DAC (Programmable via USB bootloader using [DFU](#how-to-program))
- Can support Dual-PTT HTs
- Supports all popular OSes (Linux, Windows and MacOS with limitations [[*]](https://github.com/skuep/AIOC/issues/13))
  
## Compatibility
### Software
  - [Direwolf](#notes-on-direwolf) as AX.25 modem/APRS en+decoder/...
  - [APRSdroid](#notes-on-aprsdroid) as APRS en+decoder
  - [CHIRP](#notes-on-chirp) for programming
  - [VaraFM](#notes-on-varafm)
  - ... and more

### Tested Radios (so far)
  - Wouxun UV-9D Mate (CHIRP + APRS)
  - Baofeng UV-5R (CHIRP + APRS)
  - BTECH 6X2 (CHIRP) 

![Top side of PCB](doc/images/k1-aioc-photo.jpg?raw=true "Top side of PCB")

## How To Fab
- Go to JLCPCB.com and upload the GERBER-k1-aioc.zip package (under ``kicad/k1-aioc/jlcpcb``)
  - Select PCB Thickness 1.2mm (that is what I recommend with the TRS connectors I used)
  - You may want to select LeadFree HASL
  - Select Silkscreen/Soldermask color to your liking
- Check "PCB Assembly"
  - PCBA Type "Economic"
  - Assembly Side "Top Side"
  - Tooling Holes "Added by Customer"
  - Press Confirm
  - Click "Add BOM File" and upload ``BOM-k1-aioc.csv``
  - Click "Add CPL File" and upload ``CPL-k1-aioc.csv``
  - Press Next
  - Look Through components, see if something is missing or problematic and press Next
  - Check everything looks roughly good (rotations are already baked-in and should be correct). Save to Cart

This gives you 5 (or more) SMD assembled AIOC. The only thing left to do is soldering on the TRS connectors (see [here](#how-to-assemble)).
The total bill should be around 60$ US for 5 pieces plus tax and shipping from China.

Note that the following message from JLCPCB is okay and can be ignored.
````
The below parts won't be assembled due to data missing.
H1,H2 designators don't exist in the BOM file.
J2,D3,D4,R17 designators don't exist in the CPL file.
````

Note for people doing their own PCB production: I suggest using the LCSC part numbers in the BOM file as a guide on what to buy (especially regarding the MCU).

## How To Assemble
This is the process I use for building. See photographs in ``images`` folder.
- You need to use **Monacor** ``PG-204P`` and ``PG-203P`` or compatible TRS connectors (2 solder lugs and a big tab for the sleeve connection). **Adafruit** products ``1800`` and ``1798`` are confirmed to work as well.
- Cut the 2.5mm and 3.5mm TRS sleeve tab where the hole is located
- Put both TRS connectors into the 3d-printed solder guide (or a cheap HT that you don't mind potentially damaging). Make sure, that they are seated all the way in. If the holes in the solder guide are too small, you can ream them using a 2.5mm and 3.5mm drill bit.
- Insert the AIOC PCB into the solder guide
- Solder sleeve tab on the back side for both TRS connectors first
- Turn around PCB and solder remaining solder lugs
- Optionally you can 3D print a case for your AIOC. [This model](https://www.thingiverse.com/thing:6144997) has been designed by a third party but is confirmed to work with the AIOC.

__Note__ It is unfortunately quite common that the TRS connectors have intermittent contact after soldering the ring or tip tabs. In this case, it helps to re-melt the solder joint, then using e.g. tweezer slightly assert some sideways force onto the contact and let the solder joint solidify **while applying the pressure on the contact**. This will keep some tension on the area where the tab internally connects to the barrel and thus give a solid (spring loaded) connection.

## How To Build
For building the firmware, clone the repository and initialize the submodules. Create an empty workspace with the STM32CubeIDE and import the project.
  - ``git clone <repositry url>``
  - ``git submodule update --init``
  - Start STM32CubeIDE and create a new workspace under ``<project-root>/stm32``
  - Choose File->Import and import the ``aioc-fw`` project in the same folder without copying
  - Select Project->Build All and the project should build. Use the Release build unless you specifically want to debug an issue

## How To Program
### Initial programming
The following steps are required for initial programming of the AIOC:
- Short outermost pins on the programming header. This will set the device into bootloader mode in the next step.
![Shorting pins for bootloader mode](doc/images/k1-aioc-dfu.jpg?raw=true "Shorting pins for bootloader mode")
- Connect USB-C cable to the AIOC PCB
- Use a tool like ``dfu-util`` to program the firmware binary from the GitHub Releases page like this:
  ````
  dfu-util -a 0 -s 0x08000000 -D aioc-fw-x-y-z.bin
  ````
  __Note__ that a ``libusb`` driver is required for this. On Windows there are additional steps required as shown [here](https://yeswolf.github.io/dfu) (*DFuSe Utility and dfu-util*). On other operating systems (e.g. Linux, MacOS), this just works ™ (provided libusb is installed on your system).
  On Linux (and MacOS), your user either needs to have the permission to use libusb (``plugdev`` group) or you might need to use ``sudo``.
- Remove short from first step, unplug and replug the device, it should now enumerate as the AIOC device

### Firmware updating
Once the AIOC has firmware loaded onto it, it can be re-programmed without the above BOOT sequence by following these steps.

__Note__ This requires firmware version >= 1.2.0. For older firmwares, the initial programming sequence above is required for updating the firmware.
- Run ``dfu-util`` like this
  ````
  dfu-util -d 1209:7388 -a 0 -s 0x08000000:leave -D aioc-fw-x-y-z.bin
  ````

This will reboot the AIOC into the bootloader automatically and perform the programming. 
After that, it automatically reboots the AIOC into the newly programmed firmware.

__Note__ Should you find yourself with a bricked AIOC, use the initial programming sequence above

## How To Use
The serial interface of the AIOC enumerates as a regular COM (Windows) or ttyACM port (Linux) and can be used as such for programming the radio as well as PTT (Asserted on ``DTR=1`` and ``RTS=0``).

__Note__ before firmware version 1.2.0, PTT was asserted by ``DTR=1`` (ignoring RTS) which caused problems with certain radios when using the serial port for programming the radio e.g. using CHIRP.

The soundcard interface of the AIOC gives access to the audio data channels. It has one mono microphone channel and one mono speaker channel and currently supports the following baudrates:
  - 48000 Hz (preferred)
  - 32000 Hz
  - 24000 Hz
  - 22050 Hz (specifically for APRSdroid, has approx. 90 ppm of frequency error)
  - 16000 Hz
  - 12000 Hz
  - 11025 Hz (has approx. 90 ppm of frequency error)
  - 8000 Hz

Since firmware version 1.2.0, a CM108 style PTT interface is available for public testing. This interface works in parallel to the COM-port PTT.
Direwolf on Linux is confirmed working, please report any issues. Note that currently, Direwolf reports some warnings when using the CM108 PTT interface on the AIOC. 
While they are annoying, they are safe to ignore and require changes in the upstream direwolf sourcecode. See https://github.com/wb2osz/direwolf/issues/448 for more details.

## Notes on Direwolf
- Follow the regular setup guide with direwolf to determine the correct audio device to use. 
  For the serial and CM108 PTT interfaces on Linux, you need to set correct permissions on the ttyACM/hidraw devices. Consult Direwolf manual.
- Configure the device as follows
  ````
  [...]
  ADEVICE plughw:<x>,0  # <- Linux
  ADEVICE x 0           # <- Windows
  ARATE 48000
  [...]
  PTT CM108             # <- Use the new CM108 compatible style PTT interface
  PTT <port> DTR -RTS   # <- Alternatively use an old school serial device for PTT
  [...]
  ````

## Notes on APRSdroid
APRSdroid support has been added by AIOC by implementing support for the fixed 22050 Hz sample rate that APRSdroid requires. 
It is important to notice, that the exact sample rate can not be achieved by the hardware, due to the 8 MHz crystal. 
The actual sample rate used is 22052 Hz (which represents around 90 ppm of error). From my testing this does not seem to be a problem for APRS at all.

However, since APRSdroid does not have any PTT control, sending data is currently not possible using the AIOC except using the radio VOX function. See https://github.com/ge0rg/aprsdroid/issues/324.
My previous experience is, that the Android kernel brings support for ttyACM devices (which is perfect for the AIOC) so implementing this feature for APRSdroid should theoretically be no problem.

Ideas such as implementing a digital-modes-spefic VOX-emulation to workaround this problem and let the AIOC activate the PTT automatically are currently being considered. 
Voice your opinion and ideas in the GitHub issues if this seems interesting to you.

## Notes on CHIRP
CHIRP is a very popuplar open-source programming software that supports a very wide array of HT radios. You can use CHIRP just as you would like with a regular programming cable.

Download:
  - Start CHIRP
  - Select Radio->Download from Radio
  - Select the AIOC COM/ttyACM port and start

Upload:
  - Select Radio->Upload to Radio
  - That's it

## Notes on VaraFM
Select "DTR only" for PTT Pin, so that the correct RTS/DTR sequence is generated for PTT

# Known issues
There are known issues with EMI, when using a handheld radio with a monopole (i.e. stock) antenna. In this case, the USB cable will (inadvertently) work as a tiger-tail (counterpoise) and thus, high RF currents go through the USB lines which cause issues with the USB connection. Some people have connected cables between the radio and the AIOC and put a ferrite core on those wires, which seems to reduce those issues. I am trying to find out, which of the wires between the radio and the AIOC produce the problem, so that we might add SMD ferrites on the AIOC in the future

# Future work
I encourage you to check for Pre-Releases announcing upcoming features. Currently we are working on
- **Configurable AIOC**: Change the way the PTT is asserted or the USB VID:PID that the AIOC uses using a Python script. These settings can be stored on the AIOC.
- **Virtual-PTT**: This feature allows your AIOC to be configured to automatically assert the PTT line when it receives TX data from your PC.
- **Virtual-COS**: The AIOC will notify your PC (e.g. using CM108 emulation) that there is audio data on the microphone input.
