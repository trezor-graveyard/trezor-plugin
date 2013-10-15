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
    DeviceChannel *_channel; // channel operated in open/close methods
    DeviceDescriptor _device; // descriptor for opening a dev channel
    boost::mutex _mutex; // synchronize access to same device

public:
    BitcoinTrezorDeviceAPI(const DeviceDescriptor &device)
        : _device(device)
    {
        // read-only attributes
        registerAttribute("vendorId", utils::hex_encode(_device.vendor_id()), true);
        registerAttribute("productId", utils::hex_encode(_device.product_id()), true);
        registerAttribute("serialNumber", _device.serial_number(), true);

        // methods
        registerMethod("open", make_method(this, &BitcoinTrezorDeviceAPI::open));
        registerMethod("close", make_method(this, &BitcoinTrezorDeviceAPI::close));
        registerMethod("call", make_method(this, &BitcoinTrezorDeviceAPI::call));
    }
    virtual ~BitcoinTrezorDeviceAPI() {};

public:
    void open();
    void close();
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
        // read-only attributes
        registerProperty("version", make_property(this, &BitcoinTrezorPluginAPI::get_version));
        registerProperty("devices", make_property(this, &BitcoinTrezorPluginAPI::get_devices));

        // methods
        registerMethod("configure", make_method(this, &BitcoinTrezorPluginAPI::configure));
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

    void configure(const std::string &config_str);
    std::string get_version();
    std::vector<FB::JSAPIPtr> get_devices();

private:
    BitcoinTrezorPluginWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
};

#endif // H_BitcoinTrezorPluginAPI
