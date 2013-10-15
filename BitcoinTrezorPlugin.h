/**********************************************************\

  Auto-generated BitcoinTrezorPlugin.h

  This file contains the auto-generated main plugin object
  implementation for the Bitcoin Trezor Plugin project

\**********************************************************/
#ifndef H_BitcoinTrezorPluginPLUGIN
#define H_BitcoinTrezorPluginPLUGIN

#include "PluginCore.h"

#include "hidapi.h"
#include "devices.h"
#include "config.pb.h"

FB_FORWARD_PTR(BitcoinTrezorPlugin)
class BitcoinTrezorPlugin : public FB::PluginCore
{
private:
    Configuration _stored_config;

public:
    static void StaticInitialize();
    static void StaticDeinitialize();

public:
    BitcoinTrezorPlugin();
    virtual ~BitcoinTrezorPlugin();

public:
    void onPluginReady();
    void shutdown();
    virtual FB::JSAPIPtr createJSAPI();
    // If you want your plugin to always be windowless, set this to true
    // If you want your plugin to be optionally windowless based on the
    // value of the "windowless" param tag, remove this method or return
    // FB::PluginCore::isWindowless()
    virtual bool isWindowless() { return true; }

    void configure(const Configuration &config);

    bool authenticate() { return authenticate(_stored_config); }
    bool authenticate(const Configuration &config);

    std::vector<DeviceDescriptor> enumerate() { return enumerate(_stored_config); }
    std::vector<DeviceDescriptor> enumerate(const Configuration &config);

    BEGIN_PLUGIN_EVENT_MAP()
    END_PLUGIN_EVENT_MAP()
};

#endif
