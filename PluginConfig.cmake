#/**********************************************************\
#
# Auto-Generated Plugin Configuration file
# for Bitcoin Trezor Plugin
#
#\**********************************************************/

set(PLUGIN_NAME "BitcoinTrezorPlugin")
set(PLUGIN_PREFIX "BTP")
set(COMPANY_NAME "bitcointrezorcom")

# ActiveX constants:
set(FBTYPELIB_NAME BitcoinTrezorPluginLib)
set(FBTYPELIB_DESC "BitcoinTrezorPlugin 1.0 Type Library")
set(IFBControl_DESC "BitcoinTrezorPlugin Control Interface")
set(FBControl_DESC "BitcoinTrezorPlugin Control Class")
set(IFBComJavascriptObject_DESC "BitcoinTrezorPlugin IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "BitcoinTrezorPlugin ComJavascriptObject Class")
set(IFBComEventSource_DESC "BitcoinTrezorPlugin IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 8d838ea1-1caa-52d1-9e15-2b685431b990)
set(IFBControl_GUID 3e03fcda-b45e-5cc1-ba47-14feefe5aa35)
set(FBControl_GUID 56a7926b-b5ff-5824-87d1-aaaf0c9870ef)
set(IFBComJavascriptObject_GUID 6033c94b-ab22-5732-a258-0bbf76783989)
set(FBComJavascriptObject_GUID 94f956e8-8918-5fb6-bef4-1ff98a9daf54)
set(IFBComEventSource_GUID 92f09d9f-7bbe-519c-87e7-14956e95baf7)
if ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID b84694c1-5c92-5c10-9b72-7ef16f7a9903)
else ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID c208b825-61ea-598c-8145-4cd95f10fb21)
endif ( FB_PLATFORM_ARCH_32 )

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "bitcointrezorcom.BitcoinTrezorPlugin")
set(MOZILLA_PLUGINID "bitcointrezor.com/BitcoinTrezorPlugin")

# strings
set(FBSTRING_CompanyName "bitcointrezor.com")
set(FBSTRING_PluginDescription "Plugin for embedding Bitcoin Trezor into websites")
set(FBSTRING_PLUGIN_VERSION "1.0.1")
set(FBSTRING_LegalCopyright "Copyright 2013 bitcointrezor.com")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "Bitcoin Trezor Plugin")
set(FBSTRING_FileExtents "")
if ( FB_PLATFORM_ARCH_32 )
    set(FBSTRING_PluginName "Bitcoin Trezor Plugin")  # No 32bit postfix to maintain backward compatability.
else ( FB_PLATFORM_ARCH_32 )
    set(FBSTRING_PluginName "Bitcoin Trezor Plugin_${FB_PLATFORM_ARCH_NAME}")
endif ( FB_PLATFORM_ARCH_32 )
set(FBSTRING_MIMEType "application/x-bitcointrezorplugin")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 0)
set(FBMAC_USE_COCOA 0)
set(FBMAC_USE_COREGRAPHICS 0)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)

# add_firebreath_library(log4cplus)
add_firebreath_library(openssl)
add_boost_library(regex)
add_boost_library(system)
add_boost_library(filesystem)
add_boost_library(thread)
