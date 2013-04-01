#!/usr/bin/python
import hid
import time
import struct
import binascii

print "Opening device"
h = hid.device(0x08f7, 0x0002)
#08f7:0002

print "Manufacturer: %s" % h.get_manufacturer_string()
print "Product: %s" % h.get_product_string()
print "Serial No: %s" % h.get_serial_number_string()

# try non-blocking mode by uncommenting the next line
#h.set_nonblocking(1)

# try writing some data to the device
LED_COLOR_RED=0x40
LED_COLOR_GREEN=0x80
LED_COLOR_YELLOW=0x00

while True:
    h.write([0x1d, LED_COLOR_RED, 16, 0, 0, 0, 0, 0])
    time.sleep(0.1)
    h.write([0x1d, LED_COLOR_GREEN, 16, 0, 0, 0, 0, 0])
    time.sleep(0.1)

    pkt = h.read(8)
    if not pkt:
        print 'nejsou data'
        continue
    (mip, cnt, tmp1, _, _) = struct.unpack("<bbHHH", struct.pack("<8B", *pkt))

    if tmp1 > 0:    
        print tmp1 / 126.74 - 5.4 

h.close()

'''
   
print "Closing device"
h.close()
'''
