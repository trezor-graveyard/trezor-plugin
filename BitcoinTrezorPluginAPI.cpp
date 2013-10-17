/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.cpp

\**********************************************************/

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

void BitcoinTrezorPluginAPI::configure(const std::string &config_str_hex)
{
    std::string config_str = utils::hex_decode(config_str_hex);

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
    FBLOG_INFO("open()", "Starting call consumer");
    try {
        if (_call_thread.joinable())
            throw std::logic_error("Already open");
        _call_queue.open();
        _call_thread = boost::thread(boost::bind(
            &BitcoinTrezorDeviceAPI::consume_calls, this));
    } catch (const std::exception &e) {
        throw FB::script_error(e.what());
    }
}

void BitcoinTrezorDeviceAPI::close()
{
    try {
        FBLOG_INFO("close()", "Closing call queue, waiting for consumer to join");
        _call_queue.close();
        _call_thread.join(); // TODO: rewrite to async
    } catch (const std::exception &e) {
        throw FB::script_error(e.what());
    }
}

void BitcoinTrezorDeviceAPI::call(const std::string &type_name,
                                  const FB::VariantMap &message_map,
                                  const FB::JSObjectPtr &callback)
{
    try {
        FBLOG_INFO("call()", "Enqueing call job");
       _call_queue.put((DeviceCallJob){type_name, message_map, callback});
    } catch (const std::exception &e) {
        throw FB::script_error(e.what());
    }
}

void BitcoinTrezorDeviceAPI::consume_calls()
{
    FBLOG_INFO("consume_calls()", "Call job consumer started");

    HIDBuffer buffer;
    DeviceChannel channel(_device, &buffer);
    DeviceCallJob job;

    while (_call_queue.get(job))
        process_call(channel, job.type_name, job.message_map, job.callback);
    
    FBLOG_INFO("consume_calls()", "Call job consumer finished");
}

void BitcoinTrezorDeviceAPI::process_call(DeviceChannel &channel,
                                          const std::string &type_name,
                                          const FB::VariantMap &message_map,
                                          const FB::JSObjectPtr &callback)
{
    try {
        FBLOG_INFO("process_call()", "Call starting");
        
        std::auto_ptr<PB::Message> outmsg;
        std::auto_ptr<PB::Message> inmsg = create_message(type_name);
        message_from_map(*inmsg, message_map);

        channel.write(*inmsg);
        outmsg = channel.read();

        FBLOG_INFO("process_call()", "Call finished");

        callback->InvokeAsync("", FB::variant_list_of
                              (false)
                              (message_name(*outmsg))
                              (message_to_map(*outmsg)));

    } catch (const std::exception &e) {
        FBLOG_FATAL("process_call()", "Exception occurred");
        FBLOG_FATAL("process_call()", e.what());

        callback->InvokeAsync("", FB::variant_list_of(e.what()));
    }
}
