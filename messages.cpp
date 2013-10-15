#include "exceptions.h"
#include "messages.h"
#include "utils.h"

#include <google/protobuf/dynamic_message.h>

#include "logging.h"

static const std::string MESSAGE_TYPE_ENUM_NAME = "MessageType";
static const std::string MESSAGE_TYPE_PREFIX = MESSAGE_TYPE_ENUM_NAME + "_";

static PB::DescriptorPool descriptor_pool(PB::DescriptorPool::generated_pool());
static PB::DynamicMessageFactory message_factory(&descriptor_pool);

void
load_protobuf(const PB::FileDescriptorSet &fdset)
{
    for (int i = 0; i < fdset.file_size(); i++)
        descriptor_pool.BuildFile(fdset.file(i));
}

static PB::MessageFactory *
pb_message_factory()
{
    return &message_factory;
}

static const PB::DescriptorPool *
pb_descriptor_pool()
{
    return &descriptor_pool;
}

static const PB::Descriptor *
message_descriptor(const std::string &name)
{
    const PB::DescriptorPool *dp = pb_descriptor_pool();

    const PB::Descriptor *md = dp->FindMessageTypeByName(name);
    if (!md) throw MessageTypeUnknown();

    return md;
}

std::string
message_name(uint16_t type)
{
    const PB::DescriptorPool *dp = pb_descriptor_pool();

    const PB::EnumDescriptor *ed = dp->FindEnumTypeByName(MESSAGE_TYPE_ENUM_NAME);
    if (!ed) throw MessageTypeUnknown();

    const PB::EnumValueDescriptor *evd = ed->FindValueByNumber(type);
    if (!evd) throw MessageTypeUnknown();

    const std::string ename = evd->name();
    return ename.substr(MESSAGE_TYPE_PREFIX.length());
}

std::string
message_name(const PB::Message &message)
{
    return message.GetTypeName();
}

uint16_t
message_type(const std::string &name)
{
    const PB::DescriptorPool *dp = pb_descriptor_pool();

    const PB::EnumDescriptor *ed = dp->FindEnumTypeByName(MESSAGE_TYPE_ENUM_NAME);
    if (!ed) throw MessageTypeUnknown();

    const std::string ename = MESSAGE_TYPE_PREFIX + name;
    const PB::EnumValueDescriptor *evd = ed->FindValueByName(ename);
    if (!evd) throw MessageTypeUnknown();

    return evd->number();
}

uint16_t
message_type(const PB::Message &message)
{
    return message_type(message_name(message));
}

std::auto_ptr<PB::Message>
create_message(const std::string &name)
{
    PB::MessageFactory *factory = pb_message_factory();

    const PB::Descriptor *descriptor = message_descriptor(name);
    const PB::Message *prototype = factory->GetPrototype(descriptor);

    return std::auto_ptr<PB::Message>(prototype->New());
}

//
// Firebreath API functions
//

static bool
is_field_binary(const PB::FieldDescriptor &fd)
{
    const PB::FieldOptions *opt = &fd.options();
    const PB::Reflection *ref = opt->GetReflection();
    const PB::FieldDescriptor *efd = ref->FindKnownExtensionByName("binary");
    if (!efd)
        throw std::runtime_error("Extention 'binary' not found");
    return ref->GetBool(*opt, efd);
}

static FB::variant
serialize_single_field(const PB::Message &message,
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
        return is_field_binary(fd) ? utils::hex_encode(str) : str;
    }

    case PB::FieldDescriptor::CPPTYPE_ENUM:
        return ref.GetEnum(message, &fd)->name();

    case PB::FieldDescriptor::CPPTYPE_MESSAGE:
        return message_to_map(ref.GetMessage(message, &fd));

    default:
        throw std::invalid_argument("Protobuf field of unknown type");
    }
}

static FB::variant
serialize_repeated_field(const PB::Message &message,
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
            result.push_back(is_field_binary(fd) ? utils::hex_encode(str) : str);
        }
        break;

    case PB::FieldDescriptor::CPPTYPE_ENUM:
        for (int i = 0; i < size; i++)
            result.push_back(ref.GetRepeatedEnum(message, &fd, i)->name());
        break;

    case PB::FieldDescriptor::CPPTYPE_MESSAGE:
        for (int i = 0; i < size; i++)
            result.push_back(message_to_map(ref.GetRepeatedMessage(message, &fd, i)));
        break;

    default:
        throw std::invalid_argument("Protobuf field of unknown/invalid type");
    }

    return result;
}

static void
parse_single_field(PB::Message &message,
                   const PB::Reflection &ref,
                   const PB::FieldDescriptor &fd,
                   const FB::variant &val)
{
    switch (fd.cpp_type()) {

    case PB::FieldDescriptor::CPPTYPE_DOUBLE:
        ref.SetDouble(&message, &fd, val.convert_cast<double>());
        break;

    case PB::FieldDescriptor::CPPTYPE_FLOAT:
        ref.SetFloat(&message, &fd, val.convert_cast<float>());
        break;

    case PB::FieldDescriptor::CPPTYPE_INT32:
        ref.SetInt32(&message, &fd, val.convert_cast<int>());
        break;

    case PB::FieldDescriptor::CPPTYPE_UINT32:
        ref.SetUInt32(&message, &fd, val.convert_cast<unsigned int>());
        break;

    case PB::FieldDescriptor::CPPTYPE_BOOL:
        ref.SetBool(&message, &fd, val.convert_cast<bool>());
        break;

    case PB::FieldDescriptor::CPPTYPE_STRING: {
        const std::string str = val.convert_cast<std::string>();
        ref.SetString(&message, &fd, is_field_binary(fd) ? utils::hex_decode(str) : str);
        break;
    }

    case PB::FieldDescriptor::CPPTYPE_ENUM: {
        const PB::EnumDescriptor *ed = fd.enum_type();
        const PB::EnumValueDescriptor *evd =
            ed->FindValueByName(val.convert_cast<std::string>());
        if (!evd) throw std::invalid_argument("Unknown enum value");
        ref.SetEnum(&message, &fd, evd);
        break;
    }

    case PB::FieldDescriptor::CPPTYPE_MESSAGE: {
        PB::Message *fm = ref.MutableMessage(&message, &fd);
        message_from_map(*fm, val.cast<FB::VariantMap>());
        break;
    }

    default:
        throw std::invalid_argument("Protobuf field of unknown/invalid type");
    }
}

static void
parse_repeated_field(PB::Message &message,
                     const PB::Reflection &ref,
                     const PB::FieldDescriptor &fd,
                     const FB::VariantList &val)
{
    switch (fd.cpp_type()) {

    case PB::FieldDescriptor::CPPTYPE_DOUBLE:
        for (int i = 0; i < val.size(); i++)
            ref.AddDouble(&message, &fd, val[i].convert_cast<double>());
        break;

    case PB::FieldDescriptor::CPPTYPE_FLOAT:
        for (int i = 0; i < val.size(); i++)
            ref.AddFloat(&message, &fd, val[i].convert_cast<float>());
        break;

    case PB::FieldDescriptor::CPPTYPE_INT32:
        for (int i = 0; i < val.size(); i++)
            ref.AddInt32(&message, &fd, val[i].convert_cast<int>());
        break;

    case PB::FieldDescriptor::CPPTYPE_UINT32:
        for (int i = 0; i < val.size(); i++)
            ref.AddUInt32(&message, &fd, val[i].convert_cast<unsigned int>());
        break;

    case PB::FieldDescriptor::CPPTYPE_BOOL:
        for (int i = 0; i < val.size(); i++)
            ref.AddBool(&message, &fd, val[i].convert_cast<bool>());
        break;

    case PB::FieldDescriptor::CPPTYPE_STRING:
        for (int i = 0; i < val.size(); i++) {
            const std::string str = val[i].convert_cast<std::string>();
            ref.AddString(&message, &fd, is_field_binary(fd) ? utils::hex_decode(str) : str);
        }
        break;

    case PB::FieldDescriptor::CPPTYPE_ENUM: {
        const PB::EnumDescriptor *ed = fd.enum_type();
        for (int i = 0; i < val.size(); i++) {
            const PB::EnumValueDescriptor *evd =
                ed->FindValueByName(val[i].convert_cast<std::string>());
            if (!evd) throw std::invalid_argument("Unknown enum value");
            ref.AddEnum(&message, &fd, evd);
        }
        break;
    }

    case PB::FieldDescriptor::CPPTYPE_MESSAGE:
        for (int i = 0; i < val.size(); i++) {
            PB::Message *fm = ref.MutableRepeatedMessage(&message, &fd, i);
            message_from_map(*fm, val[i].convert_cast<FB::VariantMap>());
        }
        break;

    default:
        throw std::invalid_argument("Protobuf field of unknown/invalid type");
    }
}

FB::VariantMap
message_to_map(const PB::Message &message)
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

void
message_from_map(PB::Message &message, const FB::VariantMap &map)
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
