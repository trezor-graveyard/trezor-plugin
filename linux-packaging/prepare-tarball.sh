#!/bin/sh
VERSION=0.1
tar cfj dist/browser-plugin-trezor-$VERSION.tar.bz2 --transform 's:^:browser-plugin-trezor/:' INSTALL npBitcoinTrezorPlugin.{32,64}bit.so trezor-udev.rules
