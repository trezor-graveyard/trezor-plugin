/**********************************************************\

  Auto-generated BitcoinTrezorPluginAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <vector>
#include <boost/weak_ptr.hpp>

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
        registerMethod("echo",      make_method(this, &BitcoinTrezorPluginAPI::echo));
        registerMethod("testEvent", make_method(this, &BitcoinTrezorPluginAPI::testEvent));
        
        // Read-write property
        registerProperty("testString",
                         make_property(this,
                                       &BitcoinTrezorPluginAPI::get_testString,
                                       &BitcoinTrezorPluginAPI::set_testString));
        
        // Read-only property
        registerProperty("version",
                         make_property(this,
                                       &BitcoinTrezorPluginAPI::get_version));
        
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

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();
    
    std::vector<FB::VariantMap> get_devices();

    // Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Event helpers
    FB_JSAPI_EVENT(test, 0, ());
    FB_JSAPI_EVENT(echo, 2, (const FB::variant&, const int));

    // Method test-event
    void testEvent();

private:
    BitcoinTrezorPluginWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;
};

#endif // H_BitcoinTrezorPluginAPI

