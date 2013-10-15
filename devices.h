/*
 * File:   devices.h
 * Author: arembish
 *
 * Created on April 1, 2013, 6:34 PM
 */

#ifndef TREZOR_DEVICES_H
#define TREZOR_DEVICES_H

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "hidapi.h"
#include "messages.h"
#include "config.pb.h"

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
          _read_timeout(60),
          _hid_device(0) { open(desc); } // opens on construction
    virtual ~DeviceChannel() { close(); } // closes on destruction

public:
    // protobuf rpc
    std::auto_ptr<PB::Message> read(bool timeout = true);
    void write(const PB::Message &message);

private:
    // opening/closing channel
    void open(const DeviceDescriptor &desc);
    void close();

    // i/o primitives
    void write_bytes(const unsigned char *bytes, size_t length);
    void read_bytes(unsigned char *bytes, size_t length, bool timeout);
    void read_header(uint16_t *type, uint32_t *length, bool timeout);
};

#endif // TREZOR_DEVICES_H
