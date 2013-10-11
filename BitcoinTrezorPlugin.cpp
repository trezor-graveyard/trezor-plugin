/**********************************************************\

  Auto-generated BitcoinTrezorPlugin.cpp

  This file contains the auto-generated main plugin object
  implementation for the Bitcoin Trezor Plugin project

\**********************************************************/

#include "devices.h"

#include "BitcoinTrezorPluginAPI.h"

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
BitcoinTrezorPlugin::BitcoinTrezorPlugin() :
    _known_devices()
{
    _known_devices.push_back(DeviceDescriptor(0x1cbe, 0xcaf3)); // Trezor
    _known_devices.push_back(DeviceDescriptor(0x10c4, 0xea80)); // Trezor Pi
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

std::vector<DeviceDescriptor> BitcoinTrezorPlugin::list_available_devices()
{
    std::vector<DeviceDescriptor> result;
    struct hid_device_info *devices = hid_enumerate(0x0, 0x0);
    struct hid_device_info *current = devices;

    while (current) {
        DeviceDescriptor curr_desc(*current);
        for (std::vector<DeviceDescriptor>::iterator it = _known_devices.begin();
             it != _known_devices.end();
             ++it)
        {
            if (it->is_of_same_product(curr_desc)) {
                result.push_back(curr_desc);
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
    return boost::make_shared<BitcoinTrezorPluginAPI>(FB::ptr_cast<BitcoinTrezorPlugin>(shared_from_this()), m_host);
}
