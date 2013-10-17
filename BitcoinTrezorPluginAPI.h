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

struct DeviceCallJob
{
    std::string type_name;
    FB::VariantMap message_map;
    FB::JSObjectPtr callback;
};

template <typename T>
class JobQueue
{
private:
    std::queue<T> _queue;
    boost::condition_variable _cond;
    boost::mutex _mutex;
    bool _closed;

public:
    JobQueue() : _closed(false) {}

public:
    void close()
    {
        boost::mutex::scoped_lock lock(_mutex);
        _closed = true;
        _cond.notify_all();
    }
    
    void open()
    {
        boost::mutex::scoped_lock lock(_mutex);
        _closed = false;
        _cond.notify_all();
    }
    
    void put(const T &item)
    {
        boost::mutex::scoped_lock lock(_mutex);
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

class BitcoinTrezorDeviceAPI : public FB::JSAPIAuto
{
private:
    DeviceDescriptor _device;
    JobQueue<DeviceCallJob> _call_queue;
    boost::thread _call_thread;

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
    virtual ~BitcoinTrezorDeviceAPI() { close(); };

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
