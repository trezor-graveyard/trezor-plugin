/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.cpp

\**********************************************************/

#include <fstream>

#include "JSObject.h"
#include "global/config.h"

#include "BitcoinTrezorPluginAPI.h"
#include "exceptions.h"
#include "config.pb.h"

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

void BitcoinTrezorPluginAPI::configure(const std::string &config_strxx)
{
    std::ifstream ifs("/Users/jpochyla/Projects/trezor-plugin/signer/config_signed.bin");
    std::string config_str((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());

    if (config_str.size() < utils::SIGNATURE_LENGTH) {
        FBLOG_ERROR("configure()", "Invalid data");
        throw ConfigurationError("Invalid data");
    }

    // split signature and config data
    const uint8_t *sig = (uint8_t*)config_str.c_str();
    const uint8_t *data = sig + utils::SIGNATURE_LENGTH;
    const size_t datalen = config_str.size() - utils::SIGNATURE_LENGTH;

    // verify signature
    bool verified = utils::signature_verify(sig, data, datalen);
    if (!verified) {
        FBLOG_ERROR("configure()", "Signature verification failed");
        throw ConfigurationError("Signature verification failed");
    }

    // load config
    Configuration config;
    config.ParseFromArray(data, datalen);
    getPlugin()->configure(config);
}

std::string BitcoinTrezorPluginAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}

std::vector<FB::JSAPIPtr> BitcoinTrezorPluginAPI::get_devices()
{
    std::vector<DeviceDescriptor> devices = getPlugin()->enumerate();
    std::vector<FB::JSAPIPtr> result;

    for (size_t i = 0; i < devices.size(); i++)
        result.push_back(boost::make_shared<BitcoinTrezorDeviceAPI>(devices[i]));

    return result;
}

void BitcoinTrezorDeviceAPI::open()
{
    if (!_channel)
        _channel = new DeviceChannel(_device);
}

void BitcoinTrezorDeviceAPI::close()
{
    delete _channel;
    _channel = 0;
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
        std::auto_ptr<PB::Message> output_message;
        std::auto_ptr<PB::Message> input_message = create_message(type_name);
        message_from_map(*input_message, message_map);

        {
            boost::mutex::scoped_lock lock(_mutex);
            _channel->write(*input_message);
            output_message = _channel->read();
        }

        callback->InvokeAsync("", FB::variant_list_of
                              (false)
                              (message_name(*output_message))
                              (message_to_map(*output_message)));

    } catch (const std::exception &e) {
        FBLOG_FATAL("call_internal()", "Exception occurred");
        FBLOG_FATAL("call_internal()", e.what());

        callback->InvokeAsync("", FB::variant_list_of(e.what()));
    }
}
