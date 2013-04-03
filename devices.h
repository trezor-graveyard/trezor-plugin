/* 
 * File:   devices.h
 * Author: arembish
 *
 * Created on April 1, 2013, 6:34 PM
 */

#ifndef DEVICES_H
#define	DEVICES_H

#include <vector>
#include <string>
#include <stdlib.h>

#include "hidapi.h"
#include "APITypes.h"

class TrezorDevice {
public:
    TrezorDevice(unsigned short vid, unsigned short pid) :
        vendor_id(vid), product_id(pid), serial_number(std::wstring()) {}
    TrezorDevice(struct hid_device_info *info):
        vendor_id(info->vendor_id), product_id(info->product_id), serial_number(L"S/N") {}
    
    unsigned short vendor_id;
    unsigned short product_id;
    std::wstring serial_number;
    
    inline bool isLikeMe(struct hid_device_info *info) {
        return info->vendor_id == vendor_id && info->product_id == product_id;
    }
    
    FB::VariantMap asMap() {
        FB::VariantMap result;
        result["vendor_id"] = vendor_id;
        result["product_id"] = product_id;
        result["serial_number"] = serial_number;
        
        return result;
    }
};

typedef std::vector<TrezorDevice> TrezorDevices;

#endif	/* DEVICES_H */
