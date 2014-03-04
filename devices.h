#pragma once

#include <string>
#include <vector>

#ifdef _MSC_VER
#  include <cstdint>
#else
#  include <stdint.h>
#endif

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include "hidapi.h"
#include "messages.h"

extern boost::mutex hidapi_mutex;

// Buffer for reading from a HID device.
class HIDBuffer
{
public:
    static const size_t BUFFER_SIZE = 2048; // in bytes

private:
    uint8_t _buffer[BUFFER_SIZE];
    size_t _buffer_length; // in bytes
    time_t _read_timeout; // in seconds

public:
    HIDBuffer(time_t read_timeout = 15)
        : _buffer_length(0),
          _read_timeout(read_timeout) {}

public:
    void read(hid_device *dev, uint8_t *bytes, size_t length, bool timeout = true);
    void write(hid_device *dev, const uint8_t *bytes, size_t length);
};

// Channel for communicating with a trezor device.
class DeviceChannel : public boost::noncopyable
{
private:
    HIDBuffer *_buffer; // borrowed
    hid_device *_device; // owned

public:
    DeviceChannel(const std::string &path, HIDBuffer *buffer)
        : _buffer(buffer) { open(path); } // opens on construction
    virtual ~DeviceChannel() { close(); } // closes on destruction

public:
    std::auto_ptr<PB::Message> read(bool timeout = true);
    void write(const PB::Message &message);

private:
    void read_header(uint16_t *type, uint32_t *length, bool timeout);
    void open(const std::string &path);
    void close();
};
