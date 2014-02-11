#include "JSObject.h"

#include "apis.h"
#include "exceptions.h"
#include "config.pb.h"

DeviceAPI::DeviceAPI(const DeviceDescriptor &device) :
    device(device)
{
    registerAttribute("id", device.serial_number(), true);
    registerAttribute("vendorId", utils::hex_encode(device.vendor_id()), true);
    registerAttribute("productId", utils::hex_encode(device.product_id()), true);
}

PluginAPI::PluginAPI(const BitcoinTrezorPluginPtr &plugin,
                     const FB::BrowserHostPtr &host) :
    _plugin(plugin),
    _host(host)
{
    registerAttribute("version", FBSTRING_PLUGIN_VERSION, true);
    
    registerMethod("call", make_method(this, &PluginAPI::call));
    registerMethod("close", make_method(this, &PluginAPI::close));
    registerMethod("devices", make_method(this, &PluginAPI::devices));
    registerMethod("configure", make_method(this, &PluginAPI::configure));
    registerMethod("deriveChildNode", make_method(this, &PluginAPI::derive_child_node));
}

/// Gets a reference to the plugin that was passed in when the object
/// was created.  If the plugin has already been released then this
/// will throw a FB::script_error that will be translated into a
/// javascript exception in the page.
BitcoinTrezorPluginPtr PluginAPI::acquire_plugin()
{
    BitcoinTrezorPluginPtr plugin(_plugin.lock());
    if (!plugin)
        throw FB::script_error("The plugin is invalid");
    return plugin;
}

/// Loads configuration from a serialized and signed Protobuf
/// Configuration message.
/// Unauthenticated.
void PluginAPI::configure(const std::string &config_str_hex)
{
    try {
        const std::string config_str = utils::hex_decode(config_str_hex);
        const uint8_t *config_buf = (const uint8_t *)config_str.c_str();
        const size_t config_len = config_str.size();
        bool verified = false;

        if (config_len >= utils::SIGNATURE_LENGTH)
            verified = utils::signature_verify(config_buf,
                                               config_buf + utils::SIGNATURE_LENGTH,
                                               config_len - utils::SIGNATURE_LENGTH);
        if (!verified) {
            FBLOG_ERROR("configure()", "Signature verification failed");
            throw ConfigurationError("Signature verification failed");
        }

        Configuration config;
        config.ParseFromArray(config_buf + utils::SIGNATURE_LENGTH,
                              config_len - utils::SIGNATURE_LENGTH);
        acquire_plugin()->configure(config);

    } catch (const std::exception &e) {
        FBLOG_ERROR("configure()", "Exception caught");
        FBLOG_ERROR("configure()", e.what());
        throw FB::script_error(e.what());
    }
}

/// Lists allowed device objects.
/// Authentication required.
/// Returns array of DeviceAPI.
std::vector<FB::JSAPIPtr> PluginAPI::devices()
{
    if (!acquire_plugin()->authenticate()) {
        FBLOG_ERROR("get_devices()", "URL not allowed");
        throw ConfigurationError("URL not allowed");
    }

    try {
        std::vector<DeviceDescriptor> devices = acquire_plugin()->enumerate();
        std::vector<FB::JSAPIPtr> result;

        for (size_t i = 0; i < devices.size(); i++)
            result.push_back(boost::make_shared<DeviceAPI>(devices[i]));

        return result;
        
    } catch (const std::exception &e) {
        FBLOG_ERROR("get_devices()", "Exception caught");
        FBLOG_ERROR("get_devices()", e.what());
        throw FB::script_error(e.what());
    }
}

/// BIP32 PubKey derivation
FB::VariantMap PluginAPI::derive_child_node(const FB::VariantMap &node_map,
                                            unsigned int index)
{
    try {
        std::auto_ptr<PB::Message> node_msg;
        HDNode node;

        node_msg = create_message("HDNodeType");
        message_from_map(*node_msg, node_map);
        message_to_hdnode(*node_msg, node);

        hdnode_public_ckd(&node, index);

        message_from_hdnode(*node_msg, node);
        return message_to_map(*node_msg);
        
    } catch (const std::exception &e) {
        FBLOG_ERROR("hdnode_descent()", "Exception caught");
        FBLOG_ERROR("hdnode_descent()", e.what());
        throw FB::script_error(e.what());
    }
}

/// Calls the device.
void PluginAPI::call(const FB::JSAPIPtr &device,
                     bool use_timeout,
                     const std::string &type_name,
                     const FB::VariantMap &message_map,
                     const FB::JSObjectPtr &callbacks)
{
    try {
        FBLOG_INFO("call()", "Calling device");
        
        acquire_plugin()
            ->communicator(boost::static_pointer_cast<DeviceAPI>(device)->device)
            ->call(use_timeout, type_name, message_map, callbacks);
        
    } catch (const std::exception &e) {
        FBLOG_ERROR("call()", "Exception caught");
        FBLOG_ERROR("call()", e.what());
        callbacks->InvokeAsync("error", FB::variant_list_of(e.what()));
    }
}

/// Closes the device.
void PluginAPI::close(const FB::JSAPIPtr &device,
                      const FB::JSObjectPtr &callbacks)
{
    try {
        FBLOG_INFO("close()", "Closing device");
        
        acquire_plugin()
            ->communicator(boost::static_pointer_cast<DeviceAPI>(device)->device)
            ->close(callbacks);
        
    } catch (const std::exception &e) {
        FBLOG_ERROR("close()", "Exception caught");
        FBLOG_ERROR("close()", e.what());
        callbacks->InvokeAsync("error", FB::variant_list_of(e.what()));
    }
}

