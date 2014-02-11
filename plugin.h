#pragma once

#include <boost/variant.hpp>

#include "PluginCore.h"
#include "JSAPIAuto.h"

#include "hidapi.h"
#include "devices.h"
#include "config.pb.h"

/// Generic producer-consumer work queue.
template <typename T>
class JobQueue
{
private:
    std::queue<T> _queue;
    boost::mutex _mutex;
    boost::condition_variable _cond;

public:
    /// Enqueues an item.
    void put(const T &item)
    {
        boost::mutex::scoped_lock lock(_mutex);
        _queue.push(item);
        _cond.notify_one();
    }
    
    /// Given a pointer, waits and pops from the queue into it.
    /// Acts as a termination point.
    void take(T *item)
    {
        boost::mutex::scoped_lock lock(_mutex);
        while (_queue.empty())
            _cond.wait(lock);
        *item = _queue.front();
        _queue.pop();
    }
};

/// Enqueued device close command.
struct DeviceCloseJob {
public:
    FB::JSObjectPtr callbacks;
    
public:
    DeviceCloseJob() {}
    DeviceCloseJob(const FB::JSObjectPtr &callbacks_) :
        callbacks(callbacks_) {}
};

/// Enqueued device open command.
struct DeviceOpenJob {
public:
    FB::JSObjectPtr callbacks;
    
public:
    DeviceOpenJob() {}
    DeviceOpenJob(const FB::JSObjectPtr &callbacks_) :
        callbacks(callbacks_) {}
};

/// Enqueued device call command.
struct DeviceCallJob
{
public:
    bool use_timeout;
    std::string type_name;
    FB::VariantMap message_map;
    FB::JSObjectPtr callbacks;
    
public:
    DeviceCallJob() {}
    DeviceCallJob(bool use_timeout_,
                  const std::string &type_name_,
                  const FB::VariantMap &message_map_,
                  const FB::JSObjectPtr &callbacks_) :
        use_timeout(use_timeout_),
        type_name(type_name_),
        message_map(message_map_),
        callbacks(callbacks_) {}
};

/// Union type over possible device jobs.
typedef boost::variant<
    DeviceCloseJob,
    DeviceOpenJob,
    DeviceCallJob
> DeviceJob;

/// Receives dispatched device jobs and executes them.
/// Owns the device channel and the communication buffer.
class DeviceJobExecutor : public boost::static_visitor<void>
{
private:
    DeviceDescriptor _device;
    std::auto_ptr<HIDBuffer> _buffer;
    std::auto_ptr<DeviceChannel> _channel;
    
public:
    DeviceJobExecutor(const DeviceDescriptor &device);

public:
    void operator()(const DeviceCloseJob &job);
    void operator()(const DeviceOpenJob &job);
    void operator()(const DeviceCallJob &job);

private:
    void call(bool use_timeout,
              const std::string &type_name,
              const FB::VariantMap &message_map,
              const FB::JSObjectPtr &callbacks);
};

/// Contains the job queue and the consuming thread.
/// Waits until the thread joins before destruction.
class DeviceCommunicator
{
private:
    typedef JobQueue<DeviceJob> DeviceJobQueue;
    DeviceDescriptor _device;
    DeviceJobQueue _queue;
    boost::thread _thread;

public:
    DeviceCommunicator(const DeviceDescriptor &device);
    ~DeviceCommunicator();

public:
    const DeviceDescriptor &device() const { return _device; }

    void close(const FB::JSObjectPtr &callbacks);
    void open(const FB::JSObjectPtr &callbacks);
    void call(bool use_timeout,
              const std::string &type_name,
              const FB::VariantMap &message_map,
              const FB::JSObjectPtr &callbacks);

private:
    void consumer();
};

FB_FORWARD_PTR(BitcoinTrezorPlugin)
class BitcoinTrezorPlugin : public FB::PluginCore
{
private:
    typedef std::vector<DeviceCommunicator *> DeviceCommunicators;
    DeviceCommunicators _communicators;
    Configuration _configuration;

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

    bool authenticate() { return authenticate(_configuration); }
    bool authenticate(const Configuration &config);

    std::vector<DeviceDescriptor> enumerate() { return enumerate(_configuration); }
    std::vector<DeviceDescriptor> enumerate(const Configuration &config);

    DeviceCommunicator *communicator(const DeviceDescriptor &device);

    BEGIN_PLUGIN_EVENT_MAP()
    END_PLUGIN_EVENT_MAP()
};
