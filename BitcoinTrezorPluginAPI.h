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
        // Read-only properties
        registerProperty("version", make_property(this, &BitcoinTrezorPluginAPI::get_version));
        registerProperty("devices", make_property(this, &BitcoinTrezorPluginAPI::get_devices));
        
        registerMethod("getEntropy", make_method(this, &BitcoinTrezorPluginAPI::getEntropy));
        registerMethod("getAddress", make_method(this, &BitcoinTrezorPluginAPI::getAddress));
        registerMethod("getMasterPublicKey", make_method(this, &BitcoinTrezorPluginAPI::getMasterPublicKey));
        //registerMethod("getUUID", make_method(this, &BitcoinTrezorPluginAPI::getUUID));
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

    // Read-only property ${PROPERTY.ident}
    std::string get_version();
    
    std::vector<FB::VariantMap> get_devices();
    
    bool getEntropy(const std::map<std::string, FB::variant> &device, 
        const int size, const FB::JSObjectPtr &callback);
    std::string getUUID(const std::map<std::string, FB::variant> &device);
    bool getAddress(const std::map<std::string, FB::variant> &device, 
        const std::vector<int> &address_n, const FB::JSObjectPtr &callback);
    bool getMasterPublicKey(const std::map<std::string, FB::variant> &device, 
        const FB::JSObjectPtr &callback);
    
    // Event helpers
    FB_JSAPI_EVENT(actionCanceled, 0, ());
    FB_JSAPI_EVENT(readTimeout, 0, ());
    FB_JSAPI_EVENT(generalFailure, 1, (std::string));

private:
    BitcoinTrezorPluginWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;
    
    void getEntropy_internal(const std::map<std::string, 
        FB::variant> &device, const int size, const FB::JSObjectPtr &callback);
    void getAddress_internal(const std::map<std::string, 
        FB::variant> &device, const std::vector<int> &address_n, const FB::JSObjectPtr &callback);
    void getMasterPublicKey_internal(const std::map<std::string, FB::variant> &device, 
        const FB::JSObjectPtr &callback);
};

#endif // H_BitcoinTrezorPluginAPI

