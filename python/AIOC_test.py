################################################################################
# skuep AOIC python script tweaked to work in Windows.
# Ken McMullan, 2E0UMK, 9th Feb 2026
# For now, I've left the original code, but commented.
################################################################################

import sys
import hid
from struct import Struct
from enum import IntEnum, IntFlag

class Register(IntEnum):
    MAGIC = 0x00
    USBID = 0x08
    AIOC_IOMUX0 = 0x24
    AIOC_IOMUX1 = 0x25
    CM108_IOMUX0 = 0x44
    CM108_IOMUX1 = 0x45
    CM108_IOMUX2 = 0x46
    CM108_IOMUX3 = 0x47
    SERIAL_CTRL = 0x60
    SERIAL_IOMUX0 = 0x64
    SERIAL_IOMUX1 = 0x65
    SERIAL_IOMUX2 = 0x66
    SERIAL_IOMUX3 = 0x67
    AUDIO_RX = 0x72
    AUDIO_TX = 0x78
    VPTT_LVLCTRL = 0x82
    VPTT_TIMCTRL = 0x84
    VCOS_LVLCTRL = 0x92
    VCOS_TIMCTRL = 0x94
    FOXHUNT_CTRL = 0xA0
    FOXHUNT_MSG0 = 0xA2
    FOXHUNT_MSG1 = 0xA3
    FOXHUNT_MSG2 = 0xA4
    FOXHUNT_MSG3 = 0xA5

class Command(IntFlag):
    NONE = 0x00
    WRITESTROBE = 0x01
    DEFAULTS = 0x10
    REBOOT = 0x20
    RECALL = 0x40
    STORE = 0x80

class PTTSource(IntFlag):
    NONE = 0x00000000
    CM108GPIO1 = 0x00000001
    CM108GPIO2 = 0x00000002
    CM108GPIO3 = 0x00000004
    CM108GPIO4 = 0x00000008
    SERIALDTR = 0x00000100
    SERIALRTS = 0x00000200
    SERIALDTRNRTS = 0x00000400
    SERIALNDTRRTS = 0x00000800
    VPTT = 0x00001000

class CM108ButtonSource(IntFlag):
    NONE = 0x00000000
    IN1 =  0x00010000
    IN2 =  0x00020000
    VCOS = 0x01000000

class RXGain(IntEnum):
    RXGAIN1X = 0x00000000
    RXGAIN2X = 0x00010000
    RXGAIN4X = 0x00020000
    RXGAIN8X = 0x00030000
    RXGAIN16X = 0x00040000

class TXBoost(IntEnum):
   TXBOOSTOFF = 0x00000000
   TXBOOSTON = 0x00000100

def read(device, address):
    # Set address and read
    request = Struct('<BBBL').pack(0, Command.NONE, address, 0x00000000)
    device.send_feature_report(request)
    data = device.get_feature_report(0, 7)
    data_bytes = bytes(data)  # convert list of ints to bytes on Windows
#    _, _, _, value = Struct('<BBBL').unpack(data)
    _, _, _, value = Struct('<BBBL').unpack(data_bytes)
    return value

def write(device, address, value):
    data = Struct('<BBBL').pack(0, Command.WRITESTROBE, address, value)
    device.send_feature_report(data)

def cmd(device, cmd):
    data = Struct('<BBBL').pack(0, cmd, 0x00, 0x00000000)
    device.send_feature_report(data)

def dump(device):
    for r in Register:
        print(f'Reg. {r.value:02x}: {read(device, r.value):08x}')

# aioc = hid.Device(vid=0x1209, pid=0x7388)
# aioc = hid.device(vid=0x1209, pid=0x7388)
aioc = hid.device()
aioc.open(0x1209, 0x7388)

magic = Struct("<L").pack(read(aioc, Register.MAGIC))

if (magic != b'AIOC'):
    print(f'Unexpected magic: {magic}')
    sys.exit(-1)

# print(f'Manufacturer: {aioc.manufacturer}')
print(f'Manufacturer: {aioc.get_manufacturer_string()}')
# print(f'Product: {aioc.product}')
print(f'Product: {aioc.get_product_string()}')
# print(f'Serial No: {aioc.serial}')
print(f'Serial: {aioc.get_serial_number_string()}')
print(f'Magic: {magic}')

if False:
    # Load the hardware defaults
    print(f'Loading Defaults...')
    cmd(aioc, Command.DEFAULTS)

#if False:
if True:
    # Dump all known registers
    dump(aioc)

if False:
    # Set RX settings
    rxgain = RXGain.RXGAIN4X
    print(f'Setting Audio RX gain to {str(rxgain)}')
    write(aioc, Register.AUDIO_RX, rxgain)

if False:
    # Set TX settings
    txboost = TXBoost.TXBOOSTON
    print(f'Setting Audio TX boost to {str(txboost)}')
    write(aioc, Register.AUDIO_TX, txboost)

if False:
    # Set Fox Hunt settings
    interval = 10  # beacon interval in seconds, make sure it is large enough
    id_msg = "TEST FOXHUNT"
    volume = 32768 # 0 to 65535
    wpm = 20 # words per minute

    # convert the string to ASCII bytes and pad it out with nulls to 16 characters
    id_bytes = bytearray(id_msg, 'ascii')
    while len(id_bytes) < 16:
        id_bytes.append(0)

    # convert the bytearray to four uint32s
    id0, id1, id2, id3 = Struct('<LLLL').unpack(id_bytes)

    print(f'Setting Fox Hunt interval to {interval} seconds and ID to {id_msg}')
    write(aioc, Register.FOXHUNT_CTRL, (volume << 16) | (wpm << 8) | (interval << 0))
    write(aioc, Register.FOXHUNT_MSG0, id0)
    write(aioc, Register.FOXHUNT_MSG1, id1)
    write(aioc, Register.FOXHUNT_MSG2, id2)
    write(aioc, Register.FOXHUNT_MSG3, id3)

if False:
    # Store settings into flash
    print(f'Storing...')
    cmd(aioc, Command.STORE)

if False:
    # Reboot AIOC
    print(f'Rebooting...')
    cmd(aioc, Command.REBOOT)
