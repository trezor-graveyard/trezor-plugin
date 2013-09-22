/*
 * File:   devices.h
 * Author: arembish
 *
 * Created on April 1, 2013, 6:34 PM
 */

#ifndef DEVICES_H
#define DEVICES_H

#include <vector>
#include <string>

#include <google/protobuf/message.h>

#include "APITypes.h"
#include "hidapi.h"
#include "trezor.pb.h"

namespace PB = google::protobuf;

//
// HID device information structure.
//
// We need this to pluck certain information out of hid_device_info,
// copying the struct itself is not safe.
//
struct DeviceDescriptor
{
public:
    unsigned short vendor_id;
    unsigned short product_id;
    std::wstring serial_number; // optional

public:
    DeviceDescriptor(const unsigned short vendor_id_,
                     const unsigned short product_id_)
        : vendor_id(vendor_id_),
          product_id(product_id_),
          serial_number() {};
    DeviceDescriptor(const struct hid_device_info &info)
        : vendor_id(info.vendor_id),
          product_id(info.product_id),
          serial_number(info.serial_number) {};
    virtual ~DeviceDescriptor() {};

public:
    bool is_of_same_product(const DeviceDescriptor &desc) const {
        return (desc.vendor_id == vendor_id)
            && (desc.product_id == product_id);
    }
};

//
// Channel for communicating with a trezor device, intended to be used in
// a RAII manner.
//
class DeviceChannel : public boost::noncopyable
{
private:
    unsigned char _buffer[64 * 32]; // buffer for 64-byte chunks
    size_t _buffer_length; // bytes, length of buffered data
    time_t _read_timeout; // whole seconds, timeout for entire message read call
    hid_device *_hid_device; // HID device handle

public:
    DeviceChannel(const DeviceDescriptor &desc)
        : _buffer_length(0),
          _read_timeout(10),
          _hid_device(0) { open(desc); } // opens on construction
    virtual ~DeviceChannel() { close(); } // closes on destruction

public:
    // device domain methods
    std::string read_master_public_key();
    std::string read_entropy(const size_t size);
    std::string read_address(const std::vector<int> &address_n, const int index);
    std::pair<std::vector<std::string>, std::string>
        sign_tx(const std::vector<TxInput> &inputs,
                const std::vector<TxOutput> &outputs);

private:
    // opening/closing channel
    void open(const DeviceDescriptor &desc);
    void close();

    // i/o primitives
    void write_bytes(const unsigned char *bytes, const size_t length);
    void read_bytes(unsigned char *bytes, const size_t length);
    void read_header(unsigned short &type, unsigned long &length);

    // protobuf messages rpc
    void write(const PB::Message &message, const char type);
    boost::shared_ptr<PB::Message> read();
    boost::shared_ptr<PB::Message> call(const PB::Message &message, const char type);
};

#endif  /* DEVICES_H */
