
# Initial Environment
Running the configuration script currently requires python and the hid module. Unfortunately there are several hid module compatability issues so to minimize chances for errors and confusion, it's best to use a common environment.

1) Setup a raspberry pi using the Raspberry Pi OS (Legacy, 32-bit) Bullseye release.
Make sure you have an internet connection, you can do this by setting up wifi settings before you write the sd card.
2) Run the following on the pi:
````
sudo apt-get install dfu-util
sudo pip install hid
sudo apt install libfuse2 libhidapi-hidraw0 libhidapi-libusb0
````
3) Download and install the v1.3.0 AIOC release
````
wget https://github.com/skuep/AIOC/releases/download/v1.3.0/aioc-fw-1.3.0.bin
dfu-util -d 1209:7388 -a 0 -s 0x08000000:leave -D aioc-fw-1.3.0.bin
````
You may have to follow the initual programming instructions including shorting the outermost pins and run this instead: dfu-util -a 0 -s 0x08000000:leave -D aioc-fw-1.3.0.bin

4) Download the needed script and update if necessary, this gets the base script for editing and the virtual PTT enable script
````
wget https://github.com/guywire/AIOC/blob/master/Scripts/aioc-scBase.py
wget https://github.com/guywire/AIOC/blob/master/Scripts/aioc-scVPTT.py
````

5) run the script, this one enables virtual PTT
````
sudo python aioc-scVPTT.py
````

6) unplug the AIOC and try it out

