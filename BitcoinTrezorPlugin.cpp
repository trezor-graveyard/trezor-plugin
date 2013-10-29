/**********************************************************\

  Auto-generated BitcoinTrezorPlugin.cpp

  This file contains the auto-generated main plugin object
  implementation for the Bitcoin Trezor Plugin project

\**********************************************************/

#include <boost/regex.hpp>

#include "DOM/Window.h"

#include "BitcoinTrezorPluginAPI.h"
#include "exceptions.h"
#include "devices.h"

#include "BitcoinTrezorPlugin.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn BitcoinTrezorPlugin::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginInitialize()
///
/// @see FB::FactoryBase::globalPluginInitialize
///////////////////////////////////////////////////////////////////////////////
void BitcoinTrezorPlugin::StaticInitialize()
{
    hid_init();
}

///////////////////////////////////////////////////////////////////////////////
/// @fn BitcoinTrezorPlugin::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginDeinitialize()
///
/// @see FB::FactoryBase::globalPluginDeinitialize
///////////////////////////////////////////////////////////////////////////////
void BitcoinTrezorPlugin::StaticDeinitialize()
{
    hid_exit();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  BitcoinTrezorPlugin constructor.  Note that your API is not available
///         at this point, nor the window.  For best results wait to use
///         the JSAPI object until the onPluginReady method is called
///////////////////////////////////////////////////////////////////////////////
BitcoinTrezorPlugin::BitcoinTrezorPlugin()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  BitcoinTrezorPlugin destructor.
///////////////////////////////////////////////////////////////////////////////
BitcoinTrezorPlugin::~BitcoinTrezorPlugin()
{
    // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
    // root object) and tell the host to free the retained JSAPI objects then
    // unless you are holding another shared_ptr reference to your JSAPI object
    // they will be released here.
    releaseRootJSAPI();
    m_host->freeRetainedObjects();
}

void BitcoinTrezorPlugin::onPluginReady() {}

void BitcoinTrezorPlugin::shutdown()
{
    // This will be called when it is time for the plugin to shut down;
    // any threads or anything else that may hold a shared_ptr to this
    // object should be released here so that this object can be safely
    // destroyed. This is the last point that shared_from_this and weak_ptr
    // references to this object will be valid
}

void BitcoinTrezorPlugin::configure(const Configuration &config)
{
    // check expiration
    if (config.has_valid_until() && config.valid_until() < time(0)) {
        FBLOG_ERROR("configure()", "Configuration has expired");
        throw ConfigurationError("Configuration has expired");
    }

    // check allowed sites
    if (!authenticate(config)) {
        FBLOG_ERROR("configure()", "URL is not allowed");
        throw ConfigurationError("URL is not allowed");
    }

    // load wire protocol
    load_protobuf(config.wire_protocol());

    // store the config
    _stored_config = config;
}

bool BitcoinTrezorPlugin::authenticate(const Configuration &config)
{
    const std::string location = m_host->getDOMWindow()->getLocation();

    // blacklist
    for (size_t i = 0; i < config.blacklist_urls_size(); i++) {
        if (boost::regex_match(location, boost::regex(config.blacklist_urls(i))))
            return false;
    }

    // whitelist
    for (size_t i = 0; i < config.whitelist_urls_size(); i++) {
        if (boost::regex_match(location, boost::regex(config.whitelist_urls(i))))
            return true;
    }

    return false;
}

std::vector<DeviceDescriptor> BitcoinTrezorPlugin::enumerate(const Configuration &config)
{
    std::vector<DeviceDescriptor> result;
    struct hid_device_info *devices = hid_enumerate(0x0, 0x0);
    struct hid_device_info *current = devices;

    while (current) {
        DeviceDescriptor desc;
        desc.set_vendor_id(current->vendor_id);
        desc.set_product_id(current->product_id);
        if (current->serial_number)
            desc.set_serial_number(utils::utf8_encode(current->serial_number));

        for (size_t i = 0; i < config.known_devices_size(); i++) {
            const DeviceDescriptor *dd = &config.known_devices(i);
            if ((!(dd->has_vendor_id()) || dd->vendor_id() == desc.vendor_id()) &&
                (!(dd->has_product_id()) || dd->product_id() == desc.product_id()) &&
                (!(dd->has_serial_number()) || dd->serial_number() == desc.serial_number()))
            {
                result.push_back(desc);
                break;
            }
        }

        current = current->next;
    }

    hid_free_enumeration(devices);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an instance of the JSAPI object that provides your main
///         Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<BitcoinTrezorPlugin> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
///////////////////////////////////////////////////////////////////////////////
FB::JSAPIPtr BitcoinTrezorPlugin::createJSAPI()
{
    // m_host is the BrowserHost
    return boost::make_shared<BitcoinTrezorPluginAPI>(
        FB::ptr_cast<BitcoinTrezorPlugin>(shared_from_this()), m_host);
}
