#!/bin/bash

set -e

source="../firebreath-dev/build/projects/BitcoinTrezorPlugin/Release/Bitcoin Trezor Plugin.plugin"
title="Bitcoin Trezor Plugin"
size="50m"

plugin=`basename "${source}"`
tmpimage="${title}.temp.dmg"
finalimage="${title}.dmg"

set -x

# create a big enough r/w dmg
hdiutil create -srcfolder "${source}" -volname "${title}" -fs HFS+ \
    -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${size} "${tmpimage}"

# mount it, store the name
device=$(hdiutil attach -readwrite -noverify -noautoopen "${tmpimage}" | \
    egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 2

# set the dmg properties
echo '
   tell application "Finder"
     tell disk "'${title}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 885, 430}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 72
           make new alias file at container window to POSIX file "/Library/Internet Plug-Ins" with properties {name:"Internet Plug-Ins"}
           set position of item "'${plugin}'" of container window to {100, 100}
           set position of item "Internet Plug-Ins" of container window to {375, 100}
           update without registering applications
           delay 5
           close
     end tell
   end tell
' | osascript

# set permissions, compress
chmod -Rf go-w "/Volumes/${title}"
sync
sync
hdiutil detach "${device}"
hdiutil convert "${tmpimage}" -format UDZO -imagekey zlib-level=9 -o "${finalimage}"
rm -f "${tmpimage}"
