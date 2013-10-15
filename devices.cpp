#include "devices.h"
#include "messages.h"
#include "exceptions.h"
#include "utils.h"

#include "logging.h"

#include <sstream>
#include <time.h>
#include <netinet/in.h>

void
DeviceChannel::open(const DeviceDescriptor &desc)
{
    const unsigned char uart[] = {0x41, 0x01};
    const unsigned char txrx[] = {0x43, 0x03};

    FBLOG_INFO("open()", "Opening device");
    _hid_device = hid_open(desc.vendor_id(),
                           desc.product_id(),
                           utils::utf8_decode(desc.serial_number()).c_str());

    if (!_hid_device) {
        FBLOG_FATAL("open()", "Failed to open device");
        throw OpenError();
    }

    FBLOG_INFO("open()", "Sending features");
    hid_send_feature_report(_hid_device, uart, 2); // enable UART
    hid_send_feature_report(_hid_device, txrx, 2); // purge TX/RX FIFOs
    hid_set_nonblocking(_hid_device, 0); // block on read
}

void
DeviceChannel::close()
{
    FBLOG_INFO("close()", "Closing device");
    hid_close(_hid_device);
}

void
DeviceChannel::write_bytes(const unsigned char *bytes, size_t length)
{
    FBLOG_INFO("write_bytes()", "Starting to write n bytes");
    FBLOG_INFO("write_bytes()", length);

    for (size_t i = 0; i < length; i += 63) {
        unsigned char chunk[1 + 63] = {0}; // extra byte for report number
        memcpy(&chunk[1], &bytes[i], std::min(sizeof(chunk) - 1, length - i));
        chunk[0] = sizeof(chunk) - 1;

        FBLOG_INFO("write_bytes()", "Writing chunk");
        const int res = hid_write(_hid_device, chunk, sizeof(chunk));

        if (res < sizeof(chunk)) {
            FBLOG_FATAL("write_bytes()", "Write error");
            FBLOG_FATAL("write_bytes()", hid_error(_hid_device));
            throw WriteError(utils::utf8_encode(hid_error(_hid_device)));
        }
    }
}

void
DeviceChannel::read_bytes(unsigned char *bytes, size_t length, bool timeout)
{
    const time_t start_time = time(0);

    FBLOG_INFO("read_bytes()", "Starting to read n bytes");
    FBLOG_INFO("read_bytes()", length);

    FBLOG_INFO("read_bytes()", "Start buffer length");
    FBLOG_INFO("read_bytes()", _buffer_length);

    // buffer up enough chunks
    while (_buffer_length < length) {
        unsigned char chunk[1 + 63]; // extra byte for report number
        const int res = hid_read_timeout(_hid_device, chunk, sizeof(chunk), 10);

        if (res > 0) {
            FBLOG_INFO("read_bytes()", "Buffering n bytes");
            FBLOG_INFO("read_bytes()", res);
            for (size_t i = 1; i < res; i++, _buffer_length++)
                _buffer[_buffer_length] = chunk[i];
        }

        else if (res == 0 && timeout) { // check timeout
            time_t curr_time = time(0);
            if ((curr_time - start_time) > _read_timeout) {
                FBLOG_WARN("read_bytes()", "Timed out");
                throw ReadTimeout();
            }
        }

        else if (res < 0) { // read returns -1 on error
            FBLOG_FATAL("read_bytes()", "Read error");
            FBLOG_FATAL("read_bytes()", hid_error(_hid_device));
            throw ReadError(utils::utf8_encode(hid_error(_hid_device)));
        }
    }

    // read from the buffer
    size_t i;
    for (i = 0; i < length; i++) // copy to the result
        bytes[i] = _buffer[i];
    for (; i < _buffer_length; i++) // shift the buffer
        _buffer[i - length] = _buffer[i];
    _buffer_length -= length;

    FBLOG_INFO("read_bytes()", "End buffer length");
    FBLOG_INFO("read_bytes()", _buffer_length);
}

void
DeviceChannel::read_header(uint16_t *type, uint32_t *length, bool timeout)
{
    unsigned char header[6];

    read_bytes(header, 1, timeout);
    while (header[0] != '#') {
        FBLOG_WARN("read_header()", "Warning: Aligning to magic characters");
        read_bytes(header, 1, timeout);
    }

    read_bytes(header, 1, timeout);
    if (header[0] != '#') {
        FBLOG_FATAL("read_header()", "Second magic character is broken");
        throw ReadError("Failed to read header");
    }

    FBLOG_INFO("read_header()", "Reading type and length");
    read_bytes(header, 6, timeout);
    *type = ntohs((header[0] << 0) | (header[1] << 8));
    *length = ntohl((header[2] << 0) | (header[3] << 8) |
                    (header[4] << 16) | (header[5] << 24));
}

void
DeviceChannel::write(const PB::Message &message)
{
    const uint16_t type = message_type(message);
    const size_t msgsize = message.ByteSize();
    const size_t bufsize = 2 + 2 + 4 + msgsize; // ## + type + length + message

    unsigned char buf[bufsize];
    buf[0] = '#';
    buf[1] = '#';

    const uint16_t type_ = htons(type);
    buf[3] = type_ >> 8;
    buf[2] = type_ & 0xFF;

    const uint32_t length_ = htonl(msgsize);
    buf[7] = length_ >> 24;
    buf[6] = (length_ >> 16) & 0xFF;
    buf[5] = (length_ >> 8) & 0xFF;
    buf[4] = length_ & 0xFF;

    message.SerializeToArray(&buf[8], msgsize);
    write_bytes(buf, bufsize);
}

std::auto_ptr<PB::Message>
DeviceChannel::read(bool timeout)
{
    uint16_t type;
    uint32_t length;
    read_header(&type, &length, timeout);

    FBLOG_INFO("read()", "Type and Length");
    FBLOG_INFO("read()", type);
    FBLOG_INFO("read()", length);

    // check length looks valid before allocating
    if (length > sizeof(_buffer)) {
        FBLOG_FATAL("read()", "Message length is too big, probably invalid");
        throw ReadError("Invalid data");
    }

    unsigned char msgbuf[length];
    read_bytes(msgbuf, length, timeout);

    std::string name = message_name(type);
    std::auto_ptr<PB::Message> message = create_message(name);
    message->ParseFromArray(msgbuf, length);

    return message;
}
