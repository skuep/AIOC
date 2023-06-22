[![](https://dcbadge.vercel.app/api/server/wCbXu9R95C?style=flat&theme=default-inverted)](https://discord.gg/wCbXu9R95C)
[![](https://img.shields.io/github/stars/skuep/AIOC)](https://github.com/skuep/AIOC/stargazers)
[![](https://img.shields.io/github/v/release/skuep/AIOC?sort=semver)](https://github.com/skuep/AIOC/releases)
[![](https://img.shields.io/github/license/skuep/AIOC)](https://github.com/skuep/AIOC/blob/master/LICENSE.md)

# AIOC
This is the Ham Radio *All-in-one-Cable*. **It is currently in beta testing phase - Be wary!** Please read this README carefully before ordering anything.

![AIOC with Wouxun and Direwolf](doc/images/k1-aioc-wouxun.jpg?raw=true "AIOC with Wouxun and Direwolf")

## What does it do?
The AIOC is a small adapter with a USB-C connector that enumerates itself as a sound-card (e.g. for APRS purposes), a virtual tty ("COM Port") for programming and asserting the PTT (Push-To-Talk) as well as a CM108 compatible HID endpoint for CM108-style PTT (new in firmware version 1.2.0).

You can watch the videos of the *Temporarily Offline* and *HAM RADIO DUDE* YouTube channels below.

[![All In One Cable AIOC - Ham Nuggets Season 4 Episode 8 S04E08](http://img.youtube.com/vi/RZjoPNe634o/0.jpg)](http://www.youtube.com/watch?v=RZjoPNe634o "All In One Cable AIOC - Ham Nuggets Season 4 Episode 8 S04E08")
[![Your BAOFENG Programming Cable Sucks! - Get This! - AIOC All in One Cable](http://img.youtube.com/vi/xRCmXQYRLE0/0.jpg)](http://www.youtube.com/watch?v=xRCmXQYRLE0 "Your BAOFENG Programming Cable Sucks! - Get This! - AIOC All in One Cable")

# Features #
- Based on easy-to-hack STM32F302 with internal ADC/DAC (Programmable via USB bootloader using [DFU](#how-to-program))
- Easy to upload firmware updates and changes
- Easy to solder custom connectors
- Cheap & Hackable, compact and durable design (DIY overmolded enclosure is currently TBD)
### Standard Interface Firmware ###
- Digital mode USB interface (similar to digirig, mobilinkd, etc...)
- Programming Cable Function via virtual Serial Port
- No annoying prolific drivers or tinkering required. Just plug in, and go.
- CAT Control for radios if paired with custom connector
- Dual PTT Support
### *NEW* Virtual VOX / Automatic PTT Firmware ###
- Easy, plug & play setup (no confusing COM ports)
- Automatically triggers PTT when audio is detected (similar to a SignaLink)
- Compatible with APRSDroid
- Does **not** support radio programming or cat control
- Currently a 10ms hardcoded timeout (after 10ms of no digital fsk audio from PC, it drops PTT)
- Currently in early beta and is a work in progress

# Compatibility #
### Software
  - [Direwolf](#notes-on-direwolf) as AX.25 modem/APRS en+decoder/...
  - [APRSdroid](#notes-on-aprsdroid) as APRS en+decoder
  - [CHIRP](#notes-on-chirp) for programming
  - ... and more

### Tested Radios (so far)
  - Wouxun UV-9D Mate (CHIRP + APRS)
  - Baofeng UV-5R (CHIRP + APRS)
  - BTECH 6X2 (CHIRP)
  - Quansheng UV-K5 (CHIRP + APRS)

## Future Work ##
- Overmolded enclosure design (DIY using 3D-Printed mold and Resin/Hotglue)
- Possible a console or the something similar to change settings such as CM108 style vs Automatic PTT style firmware on the fly.
- Maybe integrate a TNC Modem with KISS interface? (I am not sure if that is worth the effort)

![Top side of PCB](doc/images/k1-aioc-photo.jpg?raw=true "Top side of PCB")

## Buy a pre-made AIOC
[![Buy the AIOC](https://i.imgur.com/0zJsVmk.png)](https://w1btr.com/index.php/product-category/accessories/ "Purchase an AIOC from Lucas W1BTR")

Lucas W1BTR sells AIOCs with tons of options and some chosen accessories on his website https://w1btr.com

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

This gives you 5 (or more) SMD assembled AIOC. The only thing left to do is soldering on the TRS connectors (see [here](#how-to-build)).
The total bill should be around 60$ US for 5 pieces plus tax and shipping from China.

## How To Assemble
This is the process I use for building. See photographs in ``images`` folder.
- You need to use Monacor PG-204P and PG-203P or compatible TRS connectors (2 solder lugs and a big tab for the sleeve connection)
- Cut the 2.5mm and 3.5mm TRS sleeve tab where the hole is located
- Put both TRS connectors into the 3d-printed solder guide (or a cheap HT that you don't mind potentially damaging). Make sure, that they are seated all the way in. If the holes in the solder guide are too small, you can ream them using a 2.5mm and 3.5mm drill bit.
- Insert the AIOC PCB into the solder guide
- Solder sleeve tab on the back side for both TRS connectors first
- Turn around PCB and solder remaining solder lugs

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
- **Short outermost pins on the programming header**. This will set the device into bootloader mode in the next step (not required for firmware updates).
- Connect USB-C cable to the AIOC PCB
- Use a tool like ``dfu-util`` to program the firmware binary from the GitHub Releases page like this:
  ````
  dfu-util -a 0 -s 0x08000000 -D aioc-fw-x-y-z.bin
  ````
  __Note__ that a ``libusb`` driver is required for this. On Windows there are additional steps required as shown [here](https://yeswolf.github.io/dfu) (*DFuSe Utility and dfu-util*). On other operating systems (e.g. Linux, MacOS), this just works ™ (provided libusb and dfu-util is installed on your system).
- Remove short from first step, unplug and replug the device, it should now enumerate as the AIOC device

### Firmware updating
Once the AIOC has firmware loaded onto it, it can be re-programmed without the above BOOT sequence by following these steps.
- Run ``dfu-util`` like this
  ````
  dfu-util -d 1209:7388 -a 0 -s 0x08000000:leave -D aioc-fw-x-y-z.bin
  ````
__Note__ Updating requires firmware version >= 1.2.0. For older firmwares, the initial programming sequence above including shorting the outermost pins is required for updating the firmware.

This will reboot the AIOC into the bootloader automatically and perform the programming. 
After that, it automatically reboots the AIOC into the newly programmed firmware.

__Note__ Should you find yourself with a bricked AIOC, use the initial programming sequence above.

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

To send data, the beta Automatic PTT firmware must be used, as APRSDroid does not support PTT control so sending data is not possible with the standard firmware.

This limitation is shared with the Digirig and other similar interfaces until PTT support is added to APRSDroid - See https://github.com/ge0rg/aprsdroid/issues/324.
My previous experience is, that the Android kernel brings support for ttyACM devices (which is perfect for the AIOC) so implementing this feature for APRSdroid should theoretically be no problem.

## Notes on CHIRP
CHIRP is a very popular open-source programming software that supports a very wide array of HT radios. You can use CHIRP just as you would like with a regular programming cable.

Download:
  - Start CHIRP
  - Select Radio->Download from Radio
  - Select the AIOC COM/ttyACM port and start

Upload:
  - Select Radio->Upload to Radio
  - That's it!
