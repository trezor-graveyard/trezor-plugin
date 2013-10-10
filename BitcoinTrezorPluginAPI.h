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
#include "utils.h"

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
        registerAttribute("vendorId", hex_encode(_device.vendor_id), true);
        registerAttribute("productId", hex_encode(_device.product_id), true);
        registerAttribute("serialNumber", _device.serial_number, true);

        // methods
        registerMethod("call", make_method(this, &BitcoinTrezorDeviceAPI::call));
    }
    virtual ~BitcoinTrezorDeviceAPI() {};

public:
    void call(const std::string &type_name,
              const FB::VariantMap &message_map,
              const FB::JSObjectPtr &callback);

private:
    void call_internal(const std::string &type_name,
                       const FB::VariantMap &message_map,
                       const FB::JSObjectPtr &callback);
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
