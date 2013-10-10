#ifndef TREZOR_MESSAGES_H
#define	TREZOR_MESSAGES_H

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include "APITypes.h"

namespace PB = google::protobuf;

std::string
message_name(uint16_t type);

uint16_t
message_type(const std::string &name);

boost::shared_ptr<PB::Message>
create_message(const std::string &name);

//
// Firebreath API functions
//

FB::VariantMap
message_to_map(const PB::Message &message);

void
message_from_map(PB::Message &message, const FB::VariantMap &map);

#endif // TREZOR_MESSAGES_H
