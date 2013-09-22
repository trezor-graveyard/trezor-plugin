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

bool BitcoinTrezorDeviceAPI::call_getEntropy(const int size,
					     const FB::JSObjectPtr &callback)
{
    boost::thread t(boost::bind(&BitcoinTrezorDeviceAPI::call_getEntropy_internal,
                                this, size, callback));
    return true;
}

bool BitcoinTrezorDeviceAPI::call_getAddress(const std::vector<int> &address_n,
					     const FB::JSObjectPtr &callback)
{
    boost::thread t(boost::bind(&BitcoinTrezorDeviceAPI::call_getAddress_internal,
                                this, address_n, callback));
    return true;
}

bool BitcoinTrezorDeviceAPI::call_getMasterPublicKey(const FB::JSObjectPtr &callback)
{
    boost::thread t(boost::bind(&BitcoinTrezorDeviceAPI::call_getMasterPublicKey_internal,
                                this, callback));
    return true;
}

bool BitcoinTrezorDeviceAPI::call_signTransaction(const FB::VariantMap &inputs,
                                                  const FB::VariantMap &outputs,
                                                  const FB::JSObjectPtr &callback)
{
    boost::thread t(boost::bind(&BitcoinTrezorDeviceAPI::call_signTransaction_internal,
                                this, inputs, outputs, callback));
    return true;
}


void BitcoinTrezorDeviceAPI::call_getEntropy_internal(const int size,
						      const FB::JSObjectPtr &callback)
{
    boost::mutex::scoped_lock lock(_mutex);
    std::string result;

    try {
        DeviceChannel device(_device);
        result = device.read_entropy(size);
        result = bytes_of_string_to_hex(result);
    } catch (const ActionCanceled &e) {
        fire_cancel();
    } catch (const ReadTimeout &e) {
        fire_timeout();
    } catch (const std::exception &e) {
        fire_failure(e.what());
    }

    callback->InvokeAsync("", FB::variant_list_of(result));
}

void BitcoinTrezorDeviceAPI::call_getAddress_internal(const std::vector<int> &address_n,
						      const FB::JSObjectPtr &callback)
{
    boost::mutex::scoped_lock lock(_mutex);
    std::string result;

    try {
        DeviceChannel device(_device);
        result = device.read_address(address_n, 0);
        result = bytes_of_string_to_hex(result);
    } catch (const ActionCanceled &e) {
        fire_cancel();
    } catch (const ReadTimeout &e) {
        fire_timeout();
    } catch (const std::exception &e) {
        fire_failure(e.what());
    }

    callback->InvokeAsync("", FB::variant_list_of(result));
}

void BitcoinTrezorDeviceAPI::call_getMasterPublicKey_internal(const FB::JSObjectPtr &callback)
{
    boost::mutex::scoped_lock lock(_mutex);
    std::string result;

    try {
        DeviceChannel device(_device);
        result = device.read_master_public_key();
        result = bytes_of_string_to_hex(result);
    } catch (const ActionCanceled &e) {
        fire_cancel();
    } catch (const ReadTimeout &e) {
        fire_timeout();
    } catch (const std::exception &e) {
        fire_failure(e.what());
    }

    callback->InvokeAsync("", FB::variant_list_of(result));
}

void BitcoinTrezorDeviceAPI::call_signTransaction_internal(const FB::VariantMap &inputs,
                                                           const FB::VariantMap &outputs,
                                                           const FB::JSObjectPtr &callback)
{
    boost::mutex::scoped_lock lock(_mutex);
    std::pair<std::vector<std::string>, std::string> result;

    try {
        std::vector<TxInput> msg_inputs;
        std::vector<TxOutput> msg_outputs;
        // TODO: fill msg_inputs/msg_outputs

        DeviceChannel device(_device);
        result = device.sign_tx(msg_inputs, msg_outputs);
    } catch (const ActionCanceled &e) {
        fire_cancel();
    } catch (const ReadTimeout &e) {
        fire_timeout();
    } catch (const std::exception &e) {
        fire_failure(e.what());
    }

    callback->InvokeAsync("", FB::variant_list_of(result.first)(result.second));
}
