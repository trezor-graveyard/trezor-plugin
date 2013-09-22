/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/exception/all.hpp>

#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "BitcoinTrezorPlugin.h"

#ifndef H_BitcoinTrezorPluginAPI
#define H_BitcoinTrezorPluginAPI

class BitcoinTrezorDeviceAPI : public FB::JSAPIAuto
{
private:
    DeviceDescriptor _device; // descriptor for opening a dev channel in threads
    boost::mutex _mutex; // synchronize access to same device

public:
    BitcoinTrezorDeviceAPI(const DeviceDescriptor &device)
        : _device(device)
    {
        // read-only attributes
        registerAttribute("vendorId", _device.vendor_id, true);
        registerAttribute("productId", _device.product_id, true);
        registerAttribute("serialNumber", _device.serial_number, true);

        // methods
        registerMethod("getMasterPublicKey", make_method(this, &BitcoinTrezorDeviceAPI::call_getMasterPublicKey));
        registerMethod("getEntropy", make_method(this, &BitcoinTrezorDeviceAPI::call_getEntropy));
        registerMethod("getAddress", make_method(this, &BitcoinTrezorDeviceAPI::call_getAddress));
        registerMethod("signTransaction", make_method(this, &BitcoinTrezorDeviceAPI::call_signTransaction));
    }
    virtual ~BitcoinTrezorDeviceAPI() {};

public:
    bool call_getMasterPublicKey(const FB::JSObjectPtr &callback);
    bool call_getEntropy(const int size,
                         const FB::JSObjectPtr &callback);
    bool call_getAddress(const std::vector<int> &address_n,
                         const FB::JSObjectPtr &callback);
    bool call_signTransaction(const FB::VariantMap &inputs,
                              const FB::VariantMap &outputs,
                              const FB::JSObjectPtr &callback);

private:
    void call_getMasterPublicKey_internal(const FB::JSObjectPtr &callback);
    void call_getEntropy_internal(const int size,
				  const FB::JSObjectPtr &callback);
    void call_getAddress_internal(const std::vector<int> &address_n,
				  const FB::JSObjectPtr &callback);
    void call_signTransaction_internal(const FB::VariantMap &inputs,
                                       const FB::VariantMap &outputs,
                                       const FB::JSObjectPtr &callback);

    // event helpers
    FB_JSAPI_EVENT(cancel, 0, ());
    FB_JSAPI_EVENT(timeout, 0, ());
    FB_JSAPI_EVENT(failure, 1, (const std::string &));
};

class BitcoinTrezorPluginAPI : public FB::JSAPIAuto
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// @fn BitcoinTrezorPluginAPI::BitcoinTrezorPluginAPI(const BitcoinTrezorPluginPtr& plugin, const FB::BrowserHostPtr host)
    ///
    /// @brief  Constructor for your JSAPI object.
    ///         You should register your methods, properties, and events
    ///         that should be accessible to Javascript from here.
    ///
    /// @see FB::JSAPIAuto::registerMethod
    /// @see FB::JSAPIAuto::registerProperty
    /// @see FB::JSAPIAuto::registerEvent
    ////////////////////////////////////////////////////////////////////////////
    BitcoinTrezorPluginAPI(const BitcoinTrezorPluginPtr& plugin, const FB::BrowserHostPtr& host) :
        m_plugin(plugin), m_host(host)
    {
        registerProperty("version", make_property(this, &BitcoinTrezorPluginAPI::get_version));
        registerProperty("devices", make_property(this, &BitcoinTrezorPluginAPI::get_devices));
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// @fn BitcoinTrezorPluginAPI::~BitcoinTrezorPluginAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~BitcoinTrezorPluginAPI() {};

    BitcoinTrezorPluginPtr getPlugin();

    std::string get_version();
    std::vector<FB::JSAPIPtr> get_devices();

private:
    BitcoinTrezorPluginWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
};

#endif // H_BitcoinTrezorPluginAPI
