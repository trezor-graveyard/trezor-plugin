#include "exceptions.h"
#include "messages.h"
#include "trezor.pb.h"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/hex.hpp>

#include "logging.h"

// From trezor-emu/trezor/mapping.py,
// commit 3fef5041362fbbe56963964a909c95b0e7e45e39
const std::map<unsigned short, const PB::Descriptor *> message_type_to_descriptor =
    boost::assign::map_list_of
    (0, Initialize::descriptor())
    (1, Ping::descriptor())
    (2, Success::descriptor())
    (3, Failure::descriptor())
    (9, GetEntropy::descriptor())
    (10, Entropy::descriptor())
    (11, GetMasterPublicKey::descriptor())
    (12, MasterPublicKey::descriptor())
    (13, LoadDevice::descriptor())
    (14, ResetDevice::descriptor())
    (15, SignTx::descriptor())
    (16, SimpleSignTx::descriptor())
    (17, Features::descriptor())
    (18, PinMatrixRequest::descriptor())
    (19, PinMatrixAck::descriptor())
    (20, PinMatrixCancel::descriptor())
    (21, TxRequest::descriptor())
    (23, TxInput::descriptor())
    (24, TxOutput::descriptor())
    (25, ApplySettings::descriptor())
    (26, ButtonRequest::descriptor())
    (27, ButtonAck::descriptor())
    (28, ButtonCancel::descriptor())
    (29, GetAddress::descriptor())
    (30, Address::descriptor())
    (31, SettingsType::descriptor())
    (32, XprvType::descriptor())
    (33, CoinType::descriptor())
    (100, DebugLinkDecision::descriptor())
    (101, DebugLinkGetState::descriptor())
    (102, DebugLinkState::descriptor())
    (103, DebugLinkStop::descriptor());

static std::string hex_encode(const std::string &str)
{
    std::ostringstream stream;
    boost::algorithm::hex(str, std::ostream_iterator<char>(stream));
    return stream.str();
}

static std::string hex_decode(const std::string &str)
{
    std::ostringstream stream;
    boost::algorithm::unhex(str, std::ostream_iterator<char>(stream));
    return stream.str();
}

static const PB::Descriptor *descriptor_from_type(const unsigned short type)
{
    const std::map<unsigned short, const PB::Descriptor *>::const_iterator it =
        message_type_to_descriptor.find(type);

    if (it == message_type_to_descriptor.end())
        throw MessageTypeError();
    
    return it->second;
}

static FB::variant serialize_single_field(const PB::Message &message,
                                          const PB::Reflection &ref,
                                          const PB::FieldDescriptor &fd)
{
    switch (fd.cpp_type()) {

    case PB::FieldDescriptor::CPPTYPE_DOUBLE:
        return ref.GetDouble(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_FLOAT:
        return ref.GetFloat(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_INT64:
        return ref.GetInt64(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_UINT64:
        return ref.GetUInt64(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_INT32:
        return ref.GetInt32(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_UINT32:
        return ref.GetUInt32(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_BOOL:
        return ref.GetBool(message, &fd);

    case PB::FieldDescriptor::CPPTYPE_STRING: {
        std::string str = ref.GetString(message, &fd);
        if (fd.type() == PB::FieldDescriptor::TYPE_BYTES)
            return hex_encode(str);
        else
            return str;
    }

    case PB::FieldDescriptor::CPPTYPE_ENUM:
        return ref.GetEnum(message, &fd)->name();

    case PB::FieldDescriptor::CPPTYPE_MESSAGE:
        return message_serialize_as_map(ref.GetMessage(message, &fd));

    default:
        throw std::invalid_argument("Protobuf field of unknown type");
    }
}

static FB::variant serialize_repeated_field(const PB::Message &message,
                                            const PB::Reflection &ref,
                                            const PB::FieldDescriptor &fd)
{
    FB::VariantList result;
    int size = ref.FieldSize(message, &fd);

    switch (fd.cpp_type()) {

    case PB::FieldDescriptor::CPPTYPE_DOUBLE:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedDouble(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_FLOAT:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedFloat(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_INT64:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedInt64(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_UINT64:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedUInt64(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_INT32:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedInt32(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_UINT32:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedUInt32(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_BOOL:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedBool(message, &fd, i));
        break;

    case PB::FieldDescriptor::CPPTYPE_STRING:
        for (int i = 0; i < size; i++) {
            std::string str = ref.GetString(message, &fd);
            if (fd.type() == PB::FieldDescriptor::TYPE_BYTES)
                result.push_back(hex_encode(str));
            else
                result.push_back(str);
        }
        break;

    case PB::FieldDescriptor::CPPTYPE_ENUM:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedEnum(message, &fd, i)->name());

    case PB::FieldDescriptor::CPPTYPE_MESSAGE:
        for (int i = 0; i < size; i++)
            result.push_back(message_serialize_as_map(ref.GetRepeatedMessage(message, &fd, i)));

    default:
        throw std::invalid_argument("Protobuf field of unknown type");
    }

    return result;
}

static void parse_single_field(PB::Message &message,
                               const PB::Reflection &ref,
                               const PB::FieldDescriptor &fd,
                               const FB::variant &val)
{
    switch (fd.cpp_type()) {

    case PB::FieldDescriptor::CPPTYPE_DOUBLE:
        ref.SetDouble(&message, &fd, val.cast<double>()); break;

    case PB::FieldDescriptor::CPPTYPE_FLOAT:
        ref.SetFloat(&message, &fd, val.cast<float>()); break;

    case PB::FieldDescriptor::CPPTYPE_INT64:
    case PB::FieldDescriptor::CPPTYPE_UINT64:
        throw std::invalid_argument("Cannot parse 64-bit protobuf fields");

    case PB::FieldDescriptor::CPPTYPE_INT32:
        ref.SetInt32(&message, &fd, val.cast<int>()); break;

    case PB::FieldDescriptor::CPPTYPE_UINT32:
        ref.SetUInt32(&message, &fd, val.cast<unsigned int>()); break;

    case PB::FieldDescriptor::CPPTYPE_BOOL:
        ref.SetBool(&message, &fd, val.cast<bool>()); break;

    case PB::FieldDescriptor::CPPTYPE_STRING:
        if (fd.type() == PB::FieldDescriptor::TYPE_BYTES)
            ref.SetString(&message, &fd, hex_decode(val.cast<std::string>()));
        else
            ref.SetString(&message, &fd, val.cast<std::string>());
        break;

    case PB::FieldDescriptor::CPPTYPE_ENUM: {
        const PB::EnumDescriptor *ed = fd.enum_type();
        const PB::EnumValueDescriptor *evd = ed->FindValueByName(val.cast<std::string>());
        if (!evd) throw std::invalid_argument("Unknown enum value");
        ref.SetEnum(&message, &fd, evd);
        break;
    }

    case PB::FieldDescriptor::CPPTYPE_MESSAGE: {
        PB::Message *fm = ref.MutableMessage(&message, &fd);
        message_parse_from_map(*fm, val.cast<FB::VariantMap>());
        break;
    }

    default:
        throw std::invalid_argument("Protobuf field of unknown type");
    }
}

static void parse_repeated_field(PB::Message &message,
                                 const PB::Reflection &ref,
                                 const PB::FieldDescriptor &fd,
                                 const FB::VariantList &val)
{
    switch (fd.cpp_type()) {

    case PB::FieldDescriptor::CPPTYPE_DOUBLE:
        for (int i = 0; i < val.size(); i++)
            ref.AddDouble(&message, &fd, val[i].cast<double>());
        break;

    case PB::FieldDescriptor::CPPTYPE_FLOAT:
        for (int i = 0; i < val.size(); i++)
            ref.AddFloat(&message, &fd, val[i].cast<float>());
        break;

    case PB::FieldDescriptor::CPPTYPE_INT64:
    case PB::FieldDescriptor::CPPTYPE_UINT64:
        throw std::invalid_argument("Cannot parse 64-bit protobuf fields");

    case PB::FieldDescriptor::CPPTYPE_INT32:
        for (int i = 0; i < val.size(); i++)
            ref.AddInt32(&message, &fd, val[i].cast<int>());
        break;

    case PB::FieldDescriptor::CPPTYPE_UINT32:
        for (int i = 0; i < val.size(); i++)
            ref.AddUInt32(&message, &fd, val[i].cast<unsigned int>());
        break;

    case PB::FieldDescriptor::CPPTYPE_BOOL:
        for (int i = 0; i < val.size(); i++)
            ref.AddBool(&message, &fd, val[i].cast<bool>());
        break;

    case PB::FieldDescriptor::CPPTYPE_STRING:
        for (int i = 0; i < val.size(); i++) {
            if (fd.type() == PB::FieldDescriptor::TYPE_BYTES)
                ref.AddString(&message, &fd, hex_decode(val[i].cast<std::string>()));
            else
                ref.AddString(&message, &fd, val[i].cast<std::string>());
        }
        break;

    case PB::FieldDescriptor::CPPTYPE_ENUM: {
        const PB::EnumDescriptor *ed = fd.enum_type();
        for (int i = 0; i < val.size(); i++) {
            const PB::EnumValueDescriptor *evd = ed->FindValueByName(val[i].cast<std::string>());
            if (!evd) throw std::invalid_argument("Unknown enum value");
            ref.AddEnum(&message, &fd, evd);
        }
        break;
    }

    case PB::FieldDescriptor::CPPTYPE_MESSAGE:
        for (int i = 0; i < val.size(); i++) {
            PB::Message *fm = ref.MutableRepeatedMessage(&message, &fd, i);
            message_parse_from_map(*fm, val[i].cast<FB::VariantMap>());
        }
        break;

    default:
        throw std::invalid_argument("Protobuf field of unknown type");
    }
}

boost::shared_ptr<PB::Message> message_of_type(const unsigned short type)
{
    PB::MessageFactory *factory = PB::MessageFactory::generated_factory();
    const PB::Descriptor *descriptor = descriptor_from_type(type);
    const PB::Message *prototype = factory->GetPrototype(descriptor);

    return boost::shared_ptr<PB::Message> (prototype->New());
}

boost::shared_ptr<PB::Message> message_of_type_and_map(const unsigned short type,
                                                       const FB::VariantMap &map)
{
    boost::shared_ptr<PB::Message> message = message_of_type(type);
    message_parse_from_map(*message, map);
    return message;
}

FB::VariantMap message_serialize_as_map(const PB::Message &message)
{
    FB::VariantMap result;
    
    const PB::Descriptor *md = message.GetDescriptor();
    const PB::Reflection *ref = message.GetReflection();

    for (int i = 0; i < md->field_count(); i++) {
        const PB::FieldDescriptor *fd = md->field(i);

        if (fd->is_repeated())
            result[fd->name()] = serialize_repeated_field(message, *ref, *fd);

        else if (ref->HasField(message, fd))
            result[fd->name()] = serialize_single_field(message, *ref, *fd);
    }

    return result;
}

void message_parse_from_map(PB::Message &message, const FB::VariantMap &map)
{
    const PB::Descriptor *md = message.GetDescriptor();
    const PB::Reflection *ref = message.GetReflection();

    for (int i = 0; i < md->field_count(); i++) {
        const PB::FieldDescriptor *fd = md->field(i);
        const FB::VariantMap::const_iterator it = map.find(fd->name());

        if (it == map.end()) continue;

        if (fd->is_repeated())
            parse_repeated_field(message, *ref, *fd, it->second.cast<FB::VariantList>());
        else
            parse_single_field(message, *ref, *fd, it->second);
    }
}
