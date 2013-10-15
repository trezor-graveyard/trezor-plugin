#ifndef TREZOR_MESSAGES_H
#define	TREZOR_MESSAGES_H

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.pb.h>

#include "APITypes.h"

namespace PB = google::protobuf;

void
load_protobuf(const PB::FileDescriptorSet &fdset);

std::string
message_name(uint16_t type);

std::string
message_name(const PB::Message &message);

uint16_t
message_type(const std::string &name);

uint16_t
message_type(const PB::Message &message);

std::auto_ptr<PB::Message>
create_message(const std::string &name);

//
// Firebreath API functions
//

FB::VariantMap
message_to_map(const PB::Message &message);

void
message_from_map(PB::Message &message, const FB::VariantMap &map);

#endif // TREZOR_MESSAGES_H
