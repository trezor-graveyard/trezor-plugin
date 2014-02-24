#!/bin/sh
VERSION=$(grep FBSTRING_PLUGIN_VERSION ../PluginConfig.cmake | cut -f 2 -d '"')

strip npBitcoinTrezorPlugin.{32,64}bit.so

tar cfj browser-plugin-trezor/browser-plugin-trezor-${VERSION}.tar.bz2 --transform 's:^:browser-plugin-trezor/:' INSTALL npBitcoinTrezorPlugin.{32,64}bit.so trezor-udev.rules

sed -i "s/^Version:.*$/Version: $VERSION/" browser-plugin-trezor/browser-plugin-trezor.dsc

sed -i "s/^Version:.*$/Version:        $VERSION/" browser-plugin-trezor/browser-plugin-trezor.spec
