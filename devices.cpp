#include "devices.h"
#include "messages.h"
#include "exceptions.h"

#include "logging.h"

#include <sstream>
#include <time.h>
#include <netinet/in.h>

std::string DeviceChannel::read_entropy(const size_t size)
{
    GetEntropy instance;
    instance.set_size(size);

    boost::shared_ptr<Entropy> result =
        boost::dynamic_pointer_cast<Entropy>(call(instance, MESSAGE_TYPE(GetEntropy)));

    return result->entropy();
}

std::string DeviceChannel::read_address(const std::vector<int> &address_n, const int index)
{
    GetAddress instance;
    for (std::vector<int>::const_iterator it = address_n.begin(); it != address_n.end(); it++)
        instance.add_address_n(*it);

    boost::shared_ptr<Address> result =
        boost::dynamic_pointer_cast<Address>(call(instance, MESSAGE_TYPE(GetAddress)));

    return result->address();
}

std::string DeviceChannel::read_master_public_key()
{
    GetMasterPublicKey instance;

    boost::shared_ptr<MasterPublicKey> result =
        boost::dynamic_pointer_cast<MasterPublicKey>(call(instance,
                                                          MESSAGE_TYPE(GetMasterPublicKey)));

    return result->key();
}

std::pair<std::vector<std::string>, std::string>
DeviceChannel::sign_tx(const std::vector<TxInput> &inputs,
                       const std::vector<TxOutput> &outputs)
{
    std::vector<std::string> signatures;
    std::stringstream serialized_tx;

    // initial transaction sign call
    SignTx sign;
    sign.set_inputs_count(inputs.size());
    sign.set_outputs_count(outputs.size());

    boost::shared_ptr<TxRequest> request =
        boost::dynamic_pointer_cast<TxRequest>(call(sign, MESSAGE_TYPE(SignTx)));

    for (;;) { // loop while the device needs some inputs or outputs

        if (request->has_serialized_tx())
            serialized_tx << request->serialized_tx();

        if (request->has_signature() && request->signed_index() >= 0)
            signatures[request->signed_index()] = request->signature();

        if (request->request_index() < 0)
            break;

        switch (request->request_type()) {
        case TXINPUT:
            request = boost::dynamic_pointer_cast<TxRequest>
                (call(inputs[request->request_index()], MESSAGE_TYPE(TxInput)));
            break;
        case TXOUTPUT:
            request = boost::dynamic_pointer_cast<TxRequest>
                (call(outputs[request->request_index()], MESSAGE_TYPE(TxOutput)));
            break;
        }
    }

    return std::make_pair(signatures, serialized_tx.str());
}

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

    FBLOG_INFO("open()", "Initializing");
    call(Initialize(), MESSAGE_TYPE(Initialize));
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

void DeviceChannel::write(const PB::Message &message, const char type)
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

boost::shared_ptr<PB::Message> DeviceChannel::read()
{
    unsigned short type;
    unsigned long length;
    read_header(type, length);

    FBLOG_INFO("read()", "Type and Length");
    FBLOG_INFO("read()", type);
    FBLOG_INFO("read()", length);

    unsigned char msgbuf[length];
    read_bytes(msgbuf, length);

#define RETURN_INSTANCE_IF_IS(Z)                                        \
    if (type == MESSAGE_TYPE(Z)) {                                      \
        boost::shared_ptr<PB::Message> message(new Z);                  \
        message->ParseFromArray(msgbuf, length);                        \
        return message;                                                 \
    }

    RETURN_INSTANCE_IF_IS(Initialize);
    RETURN_INSTANCE_IF_IS(Ping);
    RETURN_INSTANCE_IF_IS(Success);
    RETURN_INSTANCE_IF_IS(Failure);
    RETURN_INSTANCE_IF_IS(GetEntropy);
    RETURN_INSTANCE_IF_IS(Entropy);
    RETURN_INSTANCE_IF_IS(GetMasterPublicKey);
    RETURN_INSTANCE_IF_IS(MasterPublicKey);
    RETURN_INSTANCE_IF_IS(LoadDevice);
    RETURN_INSTANCE_IF_IS(ResetDevice);
    RETURN_INSTANCE_IF_IS(SignTx);
    RETURN_INSTANCE_IF_IS(SimpleSignTx);
    RETURN_INSTANCE_IF_IS(Features);
    RETURN_INSTANCE_IF_IS(PinMatrixRequest);
    RETURN_INSTANCE_IF_IS(PinMatrixAck);
    RETURN_INSTANCE_IF_IS(PinMatrixCancel);
    RETURN_INSTANCE_IF_IS(TxRequest);
    RETURN_INSTANCE_IF_IS(TxInput);
    RETURN_INSTANCE_IF_IS(TxOutput);
    RETURN_INSTANCE_IF_IS(ApplySettings);
    RETURN_INSTANCE_IF_IS(ButtonRequest);
    RETURN_INSTANCE_IF_IS(ButtonAck);
    RETURN_INSTANCE_IF_IS(ButtonCancel);
    RETURN_INSTANCE_IF_IS(GetAddress);
    RETURN_INSTANCE_IF_IS(Address);
    RETURN_INSTANCE_IF_IS(SettingsType);
    RETURN_INSTANCE_IF_IS(XprvType);
    RETURN_INSTANCE_IF_IS(CoinType);

    FBLOG_FATAL("read()", "Unknown type");
    throw ReadError();

#undef RETURN_INSTANCE_IF_IS
}

// TODO: optional timeout
boost::shared_ptr<PB::Message> DeviceChannel::call(const PB::Message &message,
                                                   const char type)
{
    FBLOG_INFO("call()", "Writing to device");
    write(message, type);

    FBLOG_INFO("call()", "Reading from device");
    boost::shared_ptr<PB::Message> result = read();

    if (result->GetTypeName() == "ButtonRequest") {
        return call(ButtonAck(), MESSAGE_TYPE(ButtonAck));
    }
    else if (result->GetTypeName() == "PinMatrixRequest") {
        // TODO: pass it through like any other result, let JS handle this
        PinMatrixAck instance;
        instance.set_pin("pin");
        return call(instance, MESSAGE_TYPE(PinMatrixAck));
    }
    else if (result->GetTypeName() == "Failure") {
        boost::shared_ptr<Failure> failure =
            boost::dynamic_pointer_cast<Failure>(result);

        FBLOG_WARN("call()", "Failure");
        FBLOG_WARN("call()", failure->code());
        FBLOG_WARN("call()", failure->message());

        switch (failure->code()) {
        case 4:
            throw ActionCanceled();
            break;
        case 6:
            throw PinInvalid();
            break;
        default:
            throw FB::script_error(failure->message());
        }
    }

    FBLOG_INFO("call()", "Returning result");
    return result;
}
