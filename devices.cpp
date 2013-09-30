#include "devices.h"
#include "messages.h"
#include "exceptions.h"
#include "trezor.pb.h"

#include "logging.h"

#include <sstream>
#include <time.h>
#include <netinet/in.h>

void DeviceChannel::open(const DeviceDescriptor &desc)
{
    const unsigned char uart[] = {0x41, 0x01};
    const unsigned char txrx[] = {0x43, 0x03};

    FBLOG_INFO("open()", "Opening device");
    _hid_device = hid_open(desc.vendor_id,
                           desc.product_id,
                           desc.serial_number.c_str());

    // TODO: handle _hid_device == NULL

    FBLOG_INFO("open()", "Sending features");
    hid_send_feature_report(_hid_device, uart, 2); // enable UART
    hid_send_feature_report(_hid_device, txrx, 2); // purge TX/RX FIFOs
}

void DeviceChannel::close()
{
    FBLOG_INFO("close()", "Closing device");
    hid_close(_hid_device);
}

void DeviceChannel::write_bytes(const unsigned char *bytes, const size_t length)
{
    for (size_t i = 0; i < length; i += 63) {
        unsigned char chunk[1 + 63] = {0}; // extra byte for report number
        memcpy(&chunk[1], &bytes[i], std::min(sizeof(chunk) - 1, length - i));
        chunk[0] = sizeof(chunk) - 1;

        FBLOG_INFO("write_bytes()", "Writing chunk");
        const int res = hid_write(_hid_device, chunk, sizeof(chunk));
        
        // TODO: handle error on res < 0
    }
}

void DeviceChannel::read_bytes(unsigned char *bytes, const size_t length)
{
    const time_t timeout_time = time(0) + _read_timeout;

    while (_buffer_length < length) {
        unsigned char chunk[1 + 63]; // extra byte for report number
        const time_t ct = time(0);
        const time_t rt = timeout_time>ct ? timeout_time-ct : 0; // remaining seconds
        const int res = hid_read_timeout(_hid_device, chunk, sizeof(chunk), rt * 1000);

        if (res < 0) { // read return -1 on error
            FBLOG_FATAL("read_bytes()", "Read error");
            FBLOG_FATAL("read_bytes()", hid_error(_hid_device));
            throw ReadError();
        }
        else if (res < sizeof(chunk)) { // anything less than a full chunk is a timeout
            FBLOG_WARN("read_bytes()", "Timed out");
            throw ReadTimeout();
        }
        else {
            FBLOG_INFO("read_bytes()", "Reading chunk");
            memcpy(&_buffer[_buffer_length], &chunk[1], sizeof(chunk) - 1);
            _buffer_length += sizeof(chunk) - 1;
        }
    }

    size_t i;
    for (i = 0; i < length; i++) // copy to the result
        bytes[i] = _buffer[i];
    for (; i < _buffer_length; i++) // shift the buffer
        _buffer[i - length] = _buffer[i];
    _buffer_length -= length;

    FBLOG_INFO("read_bytes()", "Buffer length");
    FBLOG_INFO("read_bytes()", _buffer_length);
}

void DeviceChannel::read_header(unsigned short &type, unsigned long &length)
{
    unsigned char header[6];

    read_bytes(header, 1);
    while (header[0] != '#') {
        FBLOG_WARN("read_header()", "Warning: Aligning to magic characters");
        read_bytes(header, 1);
    }

    read_bytes(header, 1);
    if (header[0] != '#') {
        FBLOG_FATAL("read_header()", "Second magic character is broken");
        throw ReadError();
    }

    FBLOG_INFO("read_header()", "Reading type and length");
    read_bytes(header, 6);
    type = ntohs(header[0] | (header[1] << 8));
    length = ntohl(header[2] | (header[3] << 8) | (header[4] << 16) | (header[5] << 24));
}

void DeviceChannel::write(const PB::Message &message, const unsigned short type)
{
    const size_t msgsize = message.ByteSize();
    const size_t bufsize = 2 + 2 + 4 + msgsize; // ## + type + length + message

    unsigned char buf[bufsize];
    buf[0] = '#';
    buf[1] = '#';

    const unsigned short type_ = htons(type);
    buf[3] = type_ >> 8;
    buf[2] = type_ & 0xFF;

    const unsigned long length_ = htonl(msgsize);
    buf[7] = length_ >> 24;
    buf[6] = (length_ >> 16) & 0xFF;
    buf[5] = (length_ >> 8) & 0xFF;
    buf[4] = length_ & 0xFF;

    message.SerializeToArray(&buf[8], msgsize);
    write_bytes(buf, bufsize);
}

std::pair<boost::shared_ptr<PB::Message>, unsigned short> DeviceChannel::read()
{
    unsigned short type;
    unsigned long length;
    read_header(type, length);

    FBLOG_INFO("read()", "Type and Length");
    FBLOG_INFO("read()", type);
    FBLOG_INFO("read()", length);

    unsigned char msgbuf[length];
    read_bytes(msgbuf, length);

    boost::shared_ptr<PB::Message> message = message_of_type(type);
    message->ParseFromArray(msgbuf, length);

    return std::make_pair(message, type);
}

std::pair<boost::shared_ptr<PB::Message>, unsigned short>
DeviceChannel::call(const PB::Message &message, const unsigned short type)
{
    write(message, type);
    return read();
}
