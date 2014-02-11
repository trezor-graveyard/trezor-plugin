#pragma once

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
#include "global/config.h"

#include "plugin.h"
#include "utils.h"

/// Structure wrapping a device descriptor for the JS API.
struct DeviceAPI : public FB::JSAPIAuto
{
public:
    DeviceDescriptor device;

public:
    DeviceAPI(const DeviceDescriptor &device);
    virtual ~DeviceAPI() {}
};

/// Root plugin API object.
class PluginAPI : public FB::JSAPIAuto
{
private:
    BitcoinTrezorPluginWeakPtr _plugin;
    FB::BrowserHostPtr _host;

public:
    PluginAPI(const BitcoinTrezorPluginPtr &plugin,
              const FB::BrowserHostPtr &host);
    virtual ~PluginAPI() {}

public:
    std::vector<FB::JSAPIPtr> devices();
    void configure(const std::string &config_str_hex);
    void close(const FB::JSAPIPtr &device,
               const FB::JSObjectPtr &callbacks);
    void call(const FB::JSAPIPtr &device,
              bool use_timeout,
              const std::string &type_name,
              const FB::VariantMap &message_map,
              const FB::JSObjectPtr &callbacks);
    FB::VariantMap derive_child_node(const FB::VariantMap &node_map,
                                     unsigned int index);

private:
    BitcoinTrezorPluginPtr acquire_plugin();
};
