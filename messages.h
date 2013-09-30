#ifndef MESSAGES_H
#define	MESSAGES_H

#include <map>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include "APITypes.h"

namespace PB = google::protobuf;

extern const std::map<unsigned short, const PB::Descriptor *> message_type_to_descriptor;

FB::VariantMap message_serialize_as_map(const PB::Message &message);
void message_parse_from_map(PB::Message &message, const FB::VariantMap &map);

boost::shared_ptr<PB::Message> message_of_type(const unsigned short type);
boost::shared_ptr<PB::Message> message_of_type_and_map(const unsigned short type,
                                                       const FB::VariantMap &map);

#endif	/* MESSAGES_H */
