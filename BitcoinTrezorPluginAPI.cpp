/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "BitcoinTrezorPluginAPI.h"
#include "exceptions.h"

std::string bytes_of_string_to_hex(const std::string &str)
{
    std::stringstream stream;
    for (size_t i = 0; i < str.length(); i++)
        stream << std::hex << (((unsigned short) str[i]) & 0xFF);
    return stream.str();
}

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

void BitcoinTrezorDeviceAPI::call(const unsigned short type,
                                  const FB::VariantMap &message,
                                  const FB::JSObjectPtr &callback)
{
    boost::thread t(boost::bind(&BitcoinTrezorDeviceAPI::call_internal,
                                this, type, message, callback));
}

void BitcoinTrezorDeviceAPI::call_internal(const unsigned short type,
                                           const FB::VariantMap &message,
                                           const FB::JSObjectPtr &callback)
{
    boost::mutex::scoped_lock lock(_mutex);
    FB::VariantMap result_message;
    unsigned short result_type;

    try {
        DeviceChannel device(_device);
        boost::shared_ptr<PB::Message> message_ = message_of_type_and_map(type, message);
        std::pair<boost::shared_ptr<PB::Message>, unsigned short > result =
            device.call(*message_, type);
        result_message = message_serialize_as_map(*result.first);
        result_type = result.second;
    } catch (const ActionCanceled &e) {
        fire_cancel();
    } catch (const ReadTimeout &e) {
        fire_timeout();
    } catch (const std::exception &e) {
        fire_failure(e.what());
    }

    callback->InvokeAsync("", FB::variant_list_of(result_type)(result_message));
}
