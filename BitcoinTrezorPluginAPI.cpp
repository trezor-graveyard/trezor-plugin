/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "BitcoinTrezorPluginAPI.h"
#include "exceptions.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn BitcoinTrezorPluginPtr BitcoinTrezorPluginAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
BitcoinTrezorPluginPtr BitcoinTrezorPluginAPI::getPlugin()
{
    BitcoinTrezorPluginPtr plugin(m_plugin.lock());
    if (!plugin)
        throw FB::script_error("The plugin is invalid");
    return plugin;
}

std::string BitcoinTrezorPluginAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}

std::vector<FB::JSAPIPtr> BitcoinTrezorPluginAPI::get_devices()
{
    std::vector<DeviceDescriptor> available = getPlugin()->list_available_devices();
    std::vector<FB::JSAPIPtr> result;

    for (std::vector<DeviceDescriptor>::iterator it = available.begin();
         it != available.end();
         it++)
    {
        result.push_back(boost::make_shared<BitcoinTrezorDeviceAPI>(*it));
    }

    return result;
}

void BitcoinTrezorDeviceAPI::call(const std::string &type_name,
                                  const FB::VariantMap &message_map,
                                  const FB::JSObjectPtr &callback)
{
    boost::thread t(boost::bind(&BitcoinTrezorDeviceAPI::call_internal,
                                this, type_name, message_map, callback));
}

void BitcoinTrezorDeviceAPI::call_internal(const std::string &type_name,
                                           const FB::VariantMap &message_map,
                                           const FB::JSObjectPtr &callback)
{
    try {
        boost::shared_ptr<PB::Message> message = create_message(type_name);
        uint16_t type = message_type(type_name);
        message_from_map(*message, message_map);

        std::pair<uint16_t, boost::shared_ptr<PB::Message> > result;

        {
            boost::mutex::scoped_lock lock(_mutex);
            DeviceChannel device(_device);
            device.write(*message, type);
            result = device.read();
        }

        callback->InvokeAsync("", FB::variant_list_of
                              (false)
                              (message_name(result.first))
                              (message_to_map(*result.second)));

    } catch (const std::exception &e) {
        FBLOG_FATAL("call_internal()", "Exception occurred");
        FBLOG_FATAL("call_internal()", e.what());

        callback->InvokeAsync("", FB::variant_list_of(e.what()));
    }
}
