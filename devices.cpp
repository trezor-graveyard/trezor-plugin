#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "devices.h"
#include "messages.h"
#include "bitkey.pb.h"
#include "JSExceptions.h"

#include <netinet/in.h>
#include <algorithm>
#include <sstream>

std::string TrezorDevice::generateSessionId(const short length) {
    std::string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890"
        "!@#$%^&*()"
        "`~-_=+[{]{\\|;:'\",<.>/? ");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    std::stringstream result;
    
    for(int i = 0; i < length; ++i) {
        result << chars[index_dist(rng)];
    }
    
    return result.str();
}

void TrezorDevice::open() {
    FBLOG_INFO("open()", "Opening device.");
    const unsigned char uart[] = {0x41, 0x01};
    const unsigned char txrx[] = {0x43, 0x03};
    
    hid = hid_open(vendor_id, product_id, serial_number.c_str());
    FBLOG_INFO("open()", "Sending features.");
    hid_send_feature_report(hid, uart, 2); // enable UART
    hid_send_feature_report(hid, txrx, 2); // purge TX/RX FIFOs
    
    FBLOG_INFO("open()", "Session generating");
    session_id = generateSessionId(16);
    FBLOG_INFO("open()", session_id.c_str());
    
    Initialize initialize;
    initialize.set_session_id(session_id.c_str());
    
    FBLOG_INFO("open()", "Initialization");
    Features *_features = dynamic_cast<Features *>(call(initialize, MESSAGE_TYPE(Initialize)));
    FBLOG_INFO("open()", _features->session_id().c_str());
    UUID *_uuid = dynamic_cast<UUID *>(call(GetUUID(), MESSAGE_TYPE(GetUUID)));
    uuid = _uuid->uuid();
    
    delete _features;
    delete _uuid;
}

void TrezorDevice::close() {
    hid_close(hid);
}

void TrezorDevice::rawRead(unsigned long length, unsigned char *result) {
    while (buffer_length < length) {
        unsigned char part[65] = {0};
        int length = hid_read_timeout(hid, part, 64, 1);
        part[length] = 0;
        
        if (length && part[0] <= 63) {
            FBLOG_INFO("rawRead()", "Reading chunk");
            FBLOG_INFO("rawRead()", int(part[0]));
            for (size_t i = 1; i <= part[0]; i++) {
                FBLOG_INFO("rawRead()", int(part[i]));
                buffer[buffer_length++] = part[i];
            }
        }
    }
    
    size_t i;
    for (i = 0; i < length; i++) {
        result[i] = buffer[i];
    }
    for (; i < buffer_length; i++) {
        buffer[i - length] = buffer[i];
    }
    buffer_length -= length;
}

unsigned short TrezorDevice::headerRead(unsigned long &length) {
    unsigned char header[7] = {0};
    rawRead(1, header);
    while (header[0] != '#') {
        FBLOG_WARN("headerRead()", "Warning: Aligning to magic characters");
        rawRead(1, header);
    }

    rawRead(1, header);
    if (header[0] != '#') {
        FBLOG_FATAL("headerRead()", "Second magic character is broken");
        return 255; 
    }

    rawRead(6, header);
    unsigned short type = ntohs(header[0] | (header[1] << 8));
    length = ntohl(header[2] | (header[3] << 8) | (header[4] << 16) | (header[5] << 24));
    
    FBLOG_INFO("headerRead()", "Returning type and length");
    return type;
}

google::protobuf::Message *TrezorDevice::read() {
    unsigned long length = 0;
    unsigned short type = headerRead(length);
    
    FBLOG_INFO("read()", "Type and Length");
    FBLOG_INFO("read()", type);
    FBLOG_INFO("read()", length);
    
    unsigned char *umessage = new unsigned char[length];
    rawRead(length, umessage);
    
    #define RETURN_INSTANCE_IF_IS(Z) if (type == MESSAGE_TYPE(Z)) { \
        FBLOG_INFO("read()", type); \
        Z *message = new Z; \
        message->ParseFromArray(umessage, length); \
        delete []umessage; \
        return message; \
    }
    
    RETURN_INSTANCE_IF_IS(Initialize);
    RETURN_INSTANCE_IF_IS(Success);
    RETURN_INSTANCE_IF_IS(Failure);
    RETURN_INSTANCE_IF_IS(GetUUID);
    RETURN_INSTANCE_IF_IS(UUID);
    RETURN_INSTANCE_IF_IS(GetEntropy);
    RETURN_INSTANCE_IF_IS(Entropy);
    RETURN_INSTANCE_IF_IS(Features);
    RETURN_INSTANCE_IF_IS(ButtonRequest);
    RETURN_INSTANCE_IF_IS(ButtonAck);
    RETURN_INSTANCE_IF_IS(ButtonCancel);
    
    #undef RETURN_INSTANCE_IF_IS
}

void TrezorDevice::write(const google::protobuf::Message& message, const char type) {
    char buffer[256] = {0};
    buffer[0] = '#';
    buffer[1] = '#';
    
    unsigned short _type = htons(type);
    buffer[3] = _type >> 8;
    buffer[2] = _type & 0xFF;
    
    unsigned long _length = htonl(message.ByteSize());
    buffer[7] = _length >> 24;
    buffer[6] = (_length >> 16) & 0xFF;
    buffer[5] = (_length >> 8) & 0xFF;
    buffer[4] = _length & 0xFF;
    
    std::string smessage;
    message.SerializeToString(&smessage);
    
    for (size_t i = 0; i < smessage.length(); i++)
        buffer[8 + i] = smessage[i];
    size_t total = smessage.length() + 8;
    buffer[total] = 0x0;
    
    for (size_t i = 0; i < total; i += 63) {
        FBLOG_INFO("write()", "Writing chunk");
        char part[65] = {0};
        part[0] = MIN(63, total - i * 63);
        for (size_t j = i; j < total && j <= 63; j++)
            part[j - i + 1] = buffer[j];
        
        part[part[0] + 1] = 0;
        
        hid_write(hid, (unsigned char *)part, part[0] + 1);
    }
}

google::protobuf::Message *TrezorDevice::call(const google::protobuf::Message& message, const char type) {
    FBLOG_INFO("call()", "Writing to device");
    write(message, type);
    FBLOG_INFO("call()", "Reading from device");
    google::protobuf::Message *result = read();
    
    if (result->GetTypeName() == "ButtonRequest") {
        return call(ButtonAck(), MESSAGE_TYPE(ButtonAck));
    } else if (result->GetTypeName() == "Failure") {
        int32_t code = reinterpret_cast<Failure *>(result)->code();
        switch (code) {
            case 4:
                throw FB::script_error("Action cancelled by user");
                break;
        }
    }
    
    FBLOG_INFO("call()", "Returning result");
    return result;
}

std::string TrezorDevice::get_entropy(const size_t size) {
    GetEntropy instance;
    instance.set_size(size);
    FBLOG_INFO("get_entropy()", "get_entropy");
    
    Entropy *result = dynamic_cast<Entropy *>(call(instance, MESSAGE_TYPE(GetEntropy)));
    FBLOG_INFO("call()", "Getting entropy");
    std::string entropy(result->entropy());
    std::stringstream stream;
    
    for (size_t i = 0; i < size; i++) {
        stream << std::hex << (short(entropy[i]) & 0xFF);
    }
    
    FBLOG_INFO("call()", stream.str().c_str());
    delete result;
    return stream.str();
}
