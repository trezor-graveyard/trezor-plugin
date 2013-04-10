/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"

#include "BitcoinTrezorPluginAPI.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn FB::variant BitcoinTrezorPluginAPI::echo(const FB::variant& msg)
///
/// @brief  Echos whatever is passed from Javascript.
///         Go ahead and change it. See what happens!
///////////////////////////////////////////////////////////////////////////////
FB::variant BitcoinTrezorPluginAPI::echo(const FB::variant& msg)
{
    static int n(0);
    //fire_echo("So far, you clicked this many times: ", n++);

    // return "foobar";
    return msg;
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
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

// Read/Write property testString
std::string BitcoinTrezorPluginAPI::get_testString()
{
    return m_testString;
}

void BitcoinTrezorPluginAPI::set_testString(const std::string& val)
{
    m_testString = val;
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
    return true; // the thread is started
}

void BitcoinTrezorPluginAPI::getEntropy_internal(const std::map<std::string, FB::variant> &device, 
    const int size, const FB::JSObjectPtr &callback) {
    
    TrezorDevice *trezor = getPlugin()->getDevice(device);
    
    std::string result(trezor->get_entropy(size));
    trezor->close();
    delete trezor;
    
    FBLOG_INFO("getEntropy()", "Sending back");
    FBLOG_INFO("getEntropy()", result.c_str());
    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(result));
}

void BitcoinTrezorPluginAPI::testEvent()
{
    //fire_test();
}
