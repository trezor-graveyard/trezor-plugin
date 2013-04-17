#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "devices.h"
#include "messages.h"
#include "bitkey.pb.h"
#include "exceptions.h"
#include "JSAPI.h"
#include "boost/foreach.hpp"

#include <time.h>
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
    
    init();
}

void TrezorDevice::init() {
    FBLOG_INFO("open()", "Session generating");
    session_id = generateSessionId(16);
    FBLOG_INFO("open()", session_id.c_str());
    
    Initialize initialize;
    initialize.set_session_id(session_id.c_str());
    
    Features *_features = dynamic_cast<Features *>(call(initialize, MESSAGE_TYPE(Initialize)));
    delete _features;
    
    uuid = get_uuid();
    FBLOG_INFO("open()", "Init complete");
}

void TrezorDevice::close() {
    if (hid) {
        hid_close(hid);
        
        for (size_t i = 0; i < buffer_length; i++) {
            buffer[i] = 0;
        }
        buffer_length = 0;
    }
}

void TrezorDevice::rawRead(unsigned long length, unsigned char *result, bool useTimeout) {
    time_t start; 
    time(&start);
    
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
        } else if (!length) {
            time_t current;
            time(&current);
            
            if ((current - start) > timeout) {
                FBLOG_WARN("headerRead()", "Timed out");
                throw ReadTimeout();
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
    FBLOG_INFO("rawRead()", "Buffer length");
    FBLOG_INFO("rawRead()", buffer_length);
}

unsigned short TrezorDevice::headerRead(unsigned long &length, bool useTimeout) {
    unsigned char header[7] = {0};
    rawRead(1, header, useTimeout);
    while (header[0] != '#') {
        FBLOG_WARN("headerRead()", "Warning: Aligning to magic characters");
        rawRead(1, header, useTimeout);
    }

    rawRead(1, header, useTimeout);
    if (header[0] != '#') {
        FBLOG_FATAL("headerRead()", "Second magic character is broken");
        return 255; 
    }

    rawRead(6, header, useTimeout);
    unsigned short type = ntohs(header[0] | (header[1] << 8));
    length = ntohl(header[2] | (header[3] << 8) | (header[4] << 16) | (header[5] << 24));
    
    FBLOG_INFO("headerRead()", "Returning type and length");
    return type;
}

google::protobuf::Message *TrezorDevice::read(bool useTimeout) {
    unsigned long length = 0;
    unsigned short type = headerRead(length, useTimeout);
    
    FBLOG_INFO("read()", "Type and Length");
    FBLOG_INFO("read()", type);
    FBLOG_INFO("read()", length);
    
    unsigned char umessage[READ_BUFFER_SIZE];
    for (size_t i = 0; i <= length; i++) {
        umessage[i] = 0;
    }
    
    rawRead(length, umessage, useTimeout);
    
    #define RETURN_INSTANCE_IF_IS(Z) if (type == MESSAGE_TYPE(Z)) { \
        FBLOG_INFO("read()", type); \
        Z *message = new Z; \
        message->ParseFromArray(umessage, length); \
        return message; \
    }
    
    RETURN_INSTANCE_IF_IS(Initialize);
    RETURN_INSTANCE_IF_IS(Success);
    RETURN_INSTANCE_IF_IS(Failure);
    RETURN_INSTANCE_IF_IS(GetUUID);
    RETURN_INSTANCE_IF_IS(UUID);
    RETURN_INSTANCE_IF_IS(GetEntropy);
    RETURN_INSTANCE_IF_IS(Entropy);
    RETURN_INSTANCE_IF_IS(GetMasterPublicKey);
    RETURN_INSTANCE_IF_IS(MasterPublicKey);
    RETURN_INSTANCE_IF_IS(Features);
    RETURN_INSTANCE_IF_IS(ButtonRequest);
    RETURN_INSTANCE_IF_IS(ButtonAck);
    RETURN_INSTANCE_IF_IS(ButtonCancel);
    RETURN_INSTANCE_IF_IS(GetAddress);
    RETURN_INSTANCE_IF_IS(Address);
    
    throw FB::script_error("Unknown type");
    
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

google::protobuf::Message *TrezorDevice::call(const google::protobuf::Message& message, const char type, bool useTimeout) {
    FBLOG_INFO("call()", "Writing to device");
    write(message, type);
    FBLOG_INFO("call()", "Reading from device");
    google::protobuf::Message *result = read(useTimeout);
    
    if (result->GetTypeName() == "ButtonRequest") {
        return call(ButtonAck(), MESSAGE_TYPE(ButtonAck), false);
    } else if (result->GetTypeName() == "Failure") {
        Failure *f = reinterpret_cast<Failure *>(result);
        uint32_t code = f->code();
        std::string message = f->message();
        delete f;
        
        switch (code) {
            case 4:
                throw ActionCanceled();
                break;
        }
        
        FBLOG_ERROR("call()", "Failure");
        FBLOG_ERROR("call()", code);
        FBLOG_ERROR("call()", message);
        throw FB::script_error(message);
    }
    
    FBLOG_INFO("call()", "Returning result");
    return result;
}

std::string TrezorDevice::get_entropy(const size_t size) {
    GetEntropy instance;
    instance.set_size(size);
    
    Entropy *result = dynamic_cast<Entropy *>(call(instance, MESSAGE_TYPE(GetEntropy)));
    std::string entropy(result->entropy());
    std::stringstream stream;
    
    for (size_t i = 0; i < size; i++) {
        stream << std::hex << (short(entropy[i]) & 0xFF);
    }
    
    delete result;
    return stream.str();
}

std::string TrezorDevice::get_uuid() {
    UUID *result = dynamic_cast<UUID *>(call(GetUUID(), MESSAGE_TYPE(GetUUID)));
    std::string uuid(result->uuid());
    delete result;
    
    return uuid;
}

std::string TrezorDevice::get_address(const std::vector<int> address_n, const int index) {
    GetAddress instance;
    FBLOG_INFO("call()", "get_address");
    BOOST_FOREACH(const int n, address_n) {
        instance.add_address_n(n);
    }
    FBLOG_INFO("call()", "/get_address");
    
    Address *result = dynamic_cast<Address *>(call(instance, MESSAGE_TYPE(GetAddress)));
    std::string address(result->address());
    std::stringstream stream;
    
    for (size_t i = 0; i < address.length(); i++) {
        stream << std::hex << (short(address[i]) & 0xFF);
    }
    
    delete result;
    return stream.str();
}

std::string TrezorDevice::get_master_public_key() {
    MasterPublicKey *result = dynamic_cast<MasterPublicKey *>(call(GetMasterPublicKey(), MESSAGE_TYPE(GetMasterPublicKey)));
    std::string key(result->key());
    std::stringstream stream;
    
    for (size_t i = 0; i < key.length(); i++) {
        stream << std::hex << (short(key[i]) & 0xFF);
    }
    
    delete result;
    return key;
}
