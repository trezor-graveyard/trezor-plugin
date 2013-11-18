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

#include "BitcoinTrezorPlugin.h"
#include "utils.h"

struct DeviceCallJob
{
public:
    std::string type_name;
    FB::VariantMap message_map;
    FB::JSObjectPtr callback;

public:
    DeviceCallJob() {};
    DeviceCallJob(const std::string &type_name_,
                  const FB::VariantMap &message_map_,
                  const FB::JSObjectPtr &callback_)
        : type_name(type_name_),
          message_map(message_map_),
		  callback(callback_) {}
};

template <typename T>
class JobQueue
{
private:
    std::queue<T> _queue;
    boost::mutex _mutex;
    boost::condition_variable _cond;
    std::auto_ptr<std::exception> _exception;
    bool _closed;

public:
    JobQueue(bool closed = false)
        : _closed(closed),
          _exception(0) {}

public:
    void close()
    {
        close(std::auto_ptr<std::exception>());
    }

    void close(std::auto_ptr<std::exception> e)
    {
        boost::mutex::scoped_lock lock(_mutex);
        _closed = true;
        _exception = e;
        _cond.notify_all();
    }
    
    void open()
    {
        boost::mutex::scoped_lock lock(_mutex);
        _closed = false;
        _exception.reset();
        _cond.notify_all();
    }
    
    void put(const T &item)
    {
        boost::mutex::scoped_lock lock(_mutex);
        std::exception *e = _exception.get();
        if (e)
            throw *e;
        if (_closed)
            throw std::logic_error("Cannot put into closed queue");
        _queue.push(item);
        _cond.notify_one();
    }

    bool get(T &item)
    {
        boost::mutex::scoped_lock lock(_mutex);
        while (!_closed && _queue.empty())
            _cond.wait(lock);
        if (_closed)
            return false;
        item = _queue.front();
        _queue.pop();
        return true;
    }
};

class DeviceAPI : public FB::JSAPIAuto
{
private:
    DeviceDescriptor _device;
    JobQueue<DeviceCallJob> _call_queue;
    boost::thread _call_thread;

public:
    DeviceAPI(const DeviceDescriptor &device)
        : _device(device), _call_queue(true)
    {
        // read-only attributes
        registerAttribute("vendorId", utils::hex_encode(_device.vendor_id()), true);
        registerAttribute("productId", utils::hex_encode(_device.product_id()), true);
        registerAttribute("serialNumber", _device.serial_number(), true);

        // methods
        registerMethod("open", make_method(this, &DeviceAPI::open));
        registerMethod("close", make_method(this, &DeviceAPI::close));
        registerMethod("call", make_method(this, &DeviceAPI::call));
    }
    virtual ~DeviceAPI() { close(); };

public:
    void open();
    void close();
    
    void call(const std::string &type_name,
              const FB::VariantMap &message_map,
              const FB::JSObjectPtr &callback);

private:
    void consume_calls();
    void process_call(DeviceChannel &channel,
                      const std::string &type_name,
                      const FB::VariantMap &message_map,
                      const FB::JSObjectPtr &callback);
};

class PluginAPI : public FB::JSAPIAuto
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// @fn PluginAPI::PluginAPI(const BitcoinTrezorPluginPtr& plugin, const FB::BrowserHostPtr host)
    ///
    /// @brief  Constructor for your JSAPI object.
    ///         You should register your methods, properties, and events
    ///         that should be accessible to Javascript from here.
    ///
    /// @see FB::JSAPIAuto::registerMethod
    /// @see FB::JSAPIAuto::registerProperty
    /// @see FB::JSAPIAuto::registerEvent
    ////////////////////////////////////////////////////////////////////////////
    DeviceAPI(const BitcoinTrezorPluginPtr& plugin, const FB::BrowserHostPtr& host) :
        m_plugin(plugin), m_host(host)
    {
        // read-only attributes
        registerProperty("version", make_property(this, &PluginAPI::get_version));
        registerProperty("devices", make_property(this, &PluginAPI::get_devices));

        // methods
        registerMethod("configure", make_method(this, &PluginAPI::configure));
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// @fn PluginAPI::~PluginAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~PluginAPI() {};

    BitcoinTrezorPluginPtr getPlugin();

    void configure(const std::string &config_str);
    std::string get_version();
    std::vector<FB::JSAPIPtr> get_devices();

private:
    BitcoinTrezorPluginWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
};
