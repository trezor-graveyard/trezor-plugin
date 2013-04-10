/**********************************************************\

  Auto-generated BitcoinTrezorPlugin.h

  This file contains the auto-generated main plugin object
  implementation for the Bitcoin Trezor Plugin project

\**********************************************************/
#ifndef H_BitcoinTrezorPluginPLUGIN
#define H_BitcoinTrezorPluginPLUGIN

#include "PluginWindow.h"
#include "PluginEvents/MouseEvents.h"
#include "PluginEvents/AttachedEvent.h"

#include "PluginCore.h"

#include "hidapi.h"
#include "devices.h"

#include <time.h>
#include <stdlib.h>

FB_FORWARD_PTR(BitcoinTrezorPlugin)
class BitcoinTrezorPlugin : public FB::PluginCore
{
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
    
    TrezorDevices getAvailableDevices() { 
        if (available.empty()) {
            processDevices();
        }
        return available; 
    }
    TrezorDevice *getDevice(const std::map<std::string, FB::variant> device) {
        TrezorDevice *trezor = new TrezorDevice(device);
        trezor->open();
        return trezor;
    }

    BEGIN_PLUGIN_EVENT_MAP()
        EVENTTYPE_CASE(FB::MouseDownEvent, onMouseDown, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseUpEvent, onMouseUp, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
        EVENTTYPE_CASE(FB::AttachedEvent, onWindowAttached, FB::PluginWindow)
        EVENTTYPE_CASE(FB::DetachedEvent, onWindowDetached, FB::PluginWindow)
    END_PLUGIN_EVENT_MAP()

    /** BEGIN EVENTDEF -- DON'T CHANGE THIS LINE **/
    virtual bool onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *);
    virtual bool onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *);
    virtual bool onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *);
    virtual bool onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *);
    virtual bool onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *);
    /** END EVENTDEF -- DON'T CHANGE THIS LINE **/
    
protected:
    TrezorDevices known, available;
    
    void processDevices();
};


#endif

