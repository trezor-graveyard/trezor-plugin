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
#include <sstream>
#include <exception>
#include <stdlib.h>

#include <google/protobuf/message.h>

#include "hidapi.h"
#include "APITypes.h"
#include "logging.h"

#define READ_BUFFER_SIZE 2048

class TrezorDevice {
public:
    TrezorDevice(unsigned short vid, unsigned short pid) :
        vendor_id(vid), product_id(pid), serial_number(std::wstring()), hid(0) {}
    TrezorDevice(std::map<std::string, FB::variant> device) :
        vendor_id(device["vendor_id"].convert_cast<unsigned short>()), 
        product_id(device["product_id"].convert_cast<unsigned short>()), 
        serial_number(device["serial_number"].convert_cast<std::wstring>()),
        timeout(10), hid(0)
    {
        for(size_t i = 0; i < READ_BUFFER_SIZE; i++)
            buffer[i] = 0;
        buffer_length = 0;
    }
    TrezorDevice(struct hid_device_info *info):
        vendor_id(info->vendor_id), product_id(info->product_id), 
        serial_number(info->serial_number), hid(0) {}
    
    ~TrezorDevice() { close(); }
    
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
    
    std::wstring asString() {
        std::wstringstream stream;
        stream << std::hex << vendor_id << ":" << std::hex << product_id << ":" << serial_number;
        return stream.str();
    }
    
    void setTimeout(time_t _timeout) { timeout = _timeout; }
    time_t getTimeout() const { return timeout; }
    std::string getUUID() const {
        std::stringstream stream;
    
        for (size_t i = 0; i < uuid.length(); i++) {
            stream << std::hex << (short(uuid[i]) & 0xFF);
        }
        
        return stream.str();
    }
    
    void open();
    void close();
    
    std::string get_entropy(const size_t size);
    std::string get_uuid();
    std::string get_address(const std::vector<int> address_n, const int index = 0);
    std::string get_master_public_key();
    
protected:
    void init();
    google::protobuf::Message *read(bool useTimeout);
    void write(const google::protobuf::Message &message, const char type);
    google::protobuf::Message *call(const google::protobuf::Message &message, 
        const char type, bool useTimeout = true);
    
    unsigned char buffer[READ_BUFFER_SIZE];
    size_t buffer_length;
    
    hid_device *hid;
    time_t timeout;
    
    unsigned short vendor_id;
    unsigned short product_id;
    std::wstring serial_number;
    
private:
    std::string session_id;
    std::string uuid;
    std::string generateSessionId(const short length);
    
    void rawRead(unsigned long length, unsigned char *result, bool useTimeout);
    unsigned short headerRead(unsigned long &length, bool useTimeout);
};

typedef std::vector<TrezorDevice> TrezorDevices;

#endif	/* DEVICES_H */
