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
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

// Read-only property version
std::string BitcoinTrezorPluginAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}

std::vector<FB::VariantMap> BitcoinTrezorPluginAPI::get_devices() {
    TrezorDevices available = getPlugin()->getAvailableDevices();
    std::vector<FB::VariantMap> result;
        
    for (TrezorDevices::iterator it = available.begin(); it != available.end(); ++it) {
        result.push_back(it->asMap());
    }
    
    return result;
}

bool BitcoinTrezorPluginAPI::getEntropy(const std::map<std::string, FB::variant> &device, 
    const int size, const FB::JSObjectPtr &callback) {
    
    boost::thread t(boost::bind(&BitcoinTrezorPluginAPI::getEntropy_internal,
         this, device, size, callback));
    
    return true;
}

std::string BitcoinTrezorPluginAPI::getUUID(const std::map<std::string, FB::variant> &device) {
    TrezorDevice *trezor = getPlugin()->getDevice(device);
    std::string uuid(trezor->getUUID());
    delete trezor;
    return uuid;
}

bool BitcoinTrezorPluginAPI::getAddress(const std::map<std::string, FB::variant> &device, 
    const std::vector<int> &address_n, const FB::JSObjectPtr &callback) {
    
    boost::thread t(boost::bind(&BitcoinTrezorPluginAPI::getAddress_internal,
         this, device, address_n, callback));
    
    return true;
}

bool BitcoinTrezorPluginAPI::getMasterPublicKey(const std::map<std::string, FB::variant> &device, 
    const FB::JSObjectPtr &callback) {
    
    boost::thread t(boost::bind(&BitcoinTrezorPluginAPI::getMasterPublicKey_internal,
         this, device, callback));
    
    return true;
}

void BitcoinTrezorPluginAPI::getEntropy_internal(const std::map<std::string, FB::variant> &device, 
    const int size, const FB::JSObjectPtr &callback) {
    TrezorDevice *trezor = getPlugin()->getDevice(device);
    std::string result;
    
    try {
        result = trezor->get_entropy(size);
    } catch (ActionCanceled &e) {
        fire_actionCanceled();
    } catch (ReadTimeout &e) {
        fire_readTimeout();
    } catch (FB::script_error &e) {
        fire_generalFailure(e.m_error);
    }
    
    delete trezor;
    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(result));
}

void BitcoinTrezorPluginAPI::getAddress_internal(const std::map<std::string, FB::variant> &device, 
    const std::vector<int> &address_n, const FB::JSObjectPtr &callback) {
    TrezorDevice *trezor = getPlugin()->getDevice(device);
    std::string result;
    
    try {
        result = trezor->get_address(address_n);
    } catch (ActionCanceled &e) {
        fire_actionCanceled();
    } catch (ReadTimeout &e) {
        fire_readTimeout();
    } catch (FB::script_error &e) {
        fire_generalFailure(e.m_error);
    }
    
    delete trezor;
    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(result));
}

void BitcoinTrezorPluginAPI::getMasterPublicKey_internal(
    const std::map<std::string, FB::variant> &device, 
    const FB::JSObjectPtr &callback) {
    
    TrezorDevice *trezor = getPlugin()->getDevice(device);
    std::string result;
    
    try {
        result = trezor->get_master_public_key();
    } catch (ActionCanceled &e) {
        fire_actionCanceled();
    } catch (ReadTimeout &e) {
        fire_readTimeout();
    } catch (FB::script_error &e) {
        fire_generalFailure(e.m_error);
    }
    
    delete trezor;
    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(result));
}

