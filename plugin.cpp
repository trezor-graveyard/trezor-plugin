#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>

#include "DOM/Window.h"

#include "plugin.h"
#include "exceptions.h"
#include "devices.h"
#include "utils.h"
#include "apis.h"

/// Called from PluginFactory::globalPluginInitialize()
void BitcoinTrezorPlugin::StaticInitialize()
{
    hid_init();
}

/// Called from PluginFactory::globalPluginDeinitialize()
void BitcoinTrezorPlugin::StaticDeinitialize()
{
    hid_exit();
}

/// BitcoinTrezorPlugin constructor.  Note that your API is not available
/// at this point, nor the window.  For best results wait to use
/// the JSAPI object until the onPluginReady method is called.
BitcoinTrezorPlugin::BitcoinTrezorPlugin()
{
}

/// BitcoinTrezorPlugin destructor.
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
    for (DeviceCommunicators::iterator it = _communicators.begin();
         it != _communicators.end();
         it++) { delete *it; }

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
    _configuration = config;
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
    struct hid_device_info *current;

    for (current = devices; current; current = current->next) {
        // skip interfaces known to be foreign
        if (current->interface_number > 0)
            continue;

        // convert to device desc
        DeviceDescriptor desc;
        desc.set_path(current->path);
        desc.set_vendor_id(current->vendor_id);
        desc.set_product_id(current->product_id);
        if (current->serial_number)
            desc.set_serial_number(utils::utf8_encode(current->serial_number));

        // match against known devices
        for (size_t i = 0; i < config.known_devices_size(); i++) {
            const DeviceDescriptor *dd = &config.known_devices(i);
            const bool matches_vendor =
                (!(dd->has_vendor_id()) || dd->vendor_id() == desc.vendor_id());
            const bool matches_product =
                (!(dd->has_product_id()) || dd->product_id() == desc.product_id());
            const bool matches_serial_number =
                (!(dd->has_serial_number()) || dd->serial_number() == desc.serial_number());

            if (matches_serial_number && matches_vendor && matches_product)
            {
                // disallow duplicit serial numbers, debug interface should be skipped already
                for (size_t j = 0; j < result.size(); j++)
                    if (result[j].serial_number() == desc.serial_number())
                        throw std::runtime_error("Duplicit devices detected");
                result.push_back(desc);
                break;
            }
        }
    }

    hid_free_enumeration(devices);

    return result;
}

DeviceCommunicator *BitcoinTrezorPlugin::communicator(const DeviceDescriptor &desc)
{
    for (DeviceCommunicators::iterator it = _communicators.begin();
         it != _communicators.end();
         it++)
    {
        if ((*it)->device().path() == desc.path())
            return *it;
    }

    DeviceCommunicator *ret = new DeviceCommunicator(desc);
    _communicators.push_back(ret);

    return ret;
}

/// Creates an instance of the JSAPI object that provides your main
/// Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<BitcoinTrezorPlugin> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
FB::JSAPIPtr BitcoinTrezorPlugin::createJSAPI()
{
    // m_host is the BrowserHost
    return boost::make_shared<PluginAPI>(
        FB::ptr_cast<BitcoinTrezorPlugin>(shared_from_this()), m_host);
}

///
DeviceCommunicator::DeviceCommunicator(const DeviceDescriptor &device) :
    _device(device)
{
    _thread = boost::thread(boost::bind(&DeviceCommunicator::consumer, this));
}

///
DeviceCommunicator::~DeviceCommunicator()
{
    _thread.interrupt();
    _thread.join();
}

/// Enqueues a dev close job. Used to close the device handle.
void DeviceCommunicator::close(const FB::JSObjectPtr &callbacks)
{
    FBLOG_INFO("close()", "Enqueueing DeviceCloseJob");
    _queue.put(DeviceCloseJob(callbacks));
}

/// Enequeues a dev open job. Even though regular call commands auto-open, this
/// can be used to explicitly open the device handle and wait for the result.
void DeviceCommunicator::open(const FB::JSObjectPtr &callbacks)
{
    FBLOG_INFO("open()", "Enqueueing DeviceOpenJob");
    _queue.put(DeviceOpenJob(callbacks));
}

/// Enqueues a device call.
void DeviceCommunicator::call(bool use_timeout,
                              const std::string &type_name,
                              const FB::VariantMap &message_map,
                              const FB::JSObjectPtr &callbacks)
{
    FBLOG_INFO("call()", "Enqueing DeviceCallJob");
    _queue.put(DeviceCallJob(use_timeout, type_name, message_map, callbacks));
}

/// Device job processor. Consumes the job queue. Runs in a special thread.
void DeviceCommunicator::consumer()
{
    FBLOG_INFO("executor()", "Device job executor started");
    DeviceJobExecutor exec(_device);
    DeviceJob job;

    for (;;) {
        try {
            _queue.take(&job);
            boost::apply_visitor(exec, job);
        } catch (const boost::thread_interrupted &e) {
            throw;
        } catch (const std::exception &e) {
            FBLOG_ERROR("executor()", "Exception caught");
            FBLOG_ERROR("executor()", e.what());
        }
    }
    FBLOG_INFO("executor()", "Device job executor finished");
}

DeviceJobExecutor::DeviceJobExecutor(const DeviceDescriptor &device) :
    _device(device) {}

void DeviceJobExecutor::operator()(const DeviceCloseJob &job)
{
    try {
        if (_channel.get())
            _channel.reset();
        if (_buffer.get())
            _buffer.reset();
        job.callbacks->InvokeAsync("success", FB::variant_list_of());
    } catch (const boost::thread_interrupted &e) {
        throw;
    } catch (const std::exception &e) {
        FBLOG_ERROR("operator()(const DeviceCloseJob &)", "Error occurred");
        FBLOG_ERROR("operator()(const DeviceCloseJob &)", e.what());
        job.callbacks->InvokeAsync("error", FB::variant_list_of(e.what()));
    }
}

void DeviceJobExecutor::operator()(const DeviceOpenJob &job)
{
    try {
        if (!_buffer.get())
            _buffer.reset(new HIDBuffer());
        if (!_channel.get())
            _channel.reset(new DeviceChannel(_device.path(), _buffer.get()));
        job.callbacks->InvokeAsync("success", FB::variant_list_of());
    } catch (const boost::thread_interrupted &e) {
        throw;
    } catch (const std::exception &e) {
        FBLOG_ERROR("operator()(const DeviceOpenJob &)", "Error occurred");
        FBLOG_ERROR("operator()(const DeviceOpenJob &)", e.what());
        job.callbacks->InvokeAsync("error", FB::variant_list_of(e.what()));
    }
}

void DeviceJobExecutor::operator()(const DeviceCallJob &job)
{
    try {
        if (!_buffer.get())
            _buffer.reset(new HIDBuffer());
        if (!_channel.get())
            _channel.reset(new DeviceChannel(_device.path(), _buffer.get()));
        call(job.use_timeout, job.type_name, job.message_map, job.callbacks);
    } catch (const boost::thread_interrupted &e) {
        throw;
    } catch (const std::exception &e) {
        FBLOG_ERROR("operator()(const DeviceCallJob &)", "Error occurred");
        FBLOG_ERROR("operator()(const DeviceCallJob &)", e.what());
        job.callbacks->InvokeAsync("error", FB::variant_list_of(e.what()));
    }
}

/// Executes an individual device call.
void DeviceJobExecutor::call(bool use_timeout,
                             const std::string &type_name,
                             const FB::VariantMap &message_map,
                             const FB::JSObjectPtr &callbacks)
{
    FBLOG_INFO("exec_call()", "Call starting");

    std::auto_ptr<PB::Message> outmsg;
    std::auto_ptr<PB::Message> inmsg = create_message(type_name);
    message_from_map(*inmsg, message_map);

    _channel->write(*inmsg);
    outmsg = _channel->read(use_timeout);

    callbacks->InvokeAsync("success", FB::variant_list_of
                           (message_name(*outmsg))
                           (message_to_map(*outmsg)));

    FBLOG_INFO("exec_call()", "Call finished");
}
