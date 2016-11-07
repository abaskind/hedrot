#!/bin/sh
echo "Before running this script, be sure that: "
echo "   1/ all binaries have been compiled"
echo "   2/ a pdf version of the doc has been created"
echo "   3/ the Max standalone hedrotReceiver has been built."

today=$(date +"%y-%m-%d")

thisDirectory=`pwd`
rootDirectory=$thisDirectory/../..
destDirectory=$rootDirectory/build/forPackaging

hedrotVersion=`more $rootDirectory/scripts/VERSION`
hedrotVersionMessage="hedrot version: $hedrotVersion"
echo $hedrotVersionMessage

libhedrotVersion=`awk '/^#define LIBHEDROT_VERSION / { print $3 }' $rootDirectory/libhedrot/libhedrot.h`
libhedrotVersionMessage="libhedrot version: $libhedrotVersion"
echo $libhedrotVersionMessage

headtrackerFirmwareVersion=`awk '/^#define HEDROT_FIRMWARE_VERSION / { print $3 }' $rootDirectory/firmware/hedrot-firmware/hedrot_comm_protocol.h`
headtrackerFirmwareVersionMessage="Headtracker Firmware version: $headtrackerFirmwareVersion"
echo $headtrackerFirmwareVersionMessage

packageName="hedrot-$hedrotVersion-${today}-Mac"
packageFolder="$destDirectory/$packageName"


######### create destination folder #############################
rm -rf $destDirectory/*
mkdir $packageFolder

######### prepare and copy Max Standalone #############################

#copy the standalone and the default preset file
cp -R $rootDirectory/Max/hedrotReceiver.app $packageFolder
cp -R $rootDirectory/Max/hedrotReceiver.json $packageFolder

appcontentsdirectory=$packageFolder/hedrotReceiver.app/Contents

#update the version number in Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleGetInfoString '${hedrotVersion}'" $appcontentsdirectory/Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleLongVersionString '${hedrotVersion}'" $appcontentsdirectory/Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString '${hedrotVersion}'" $appcontentsdirectory/Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion '${hedrotVersion}'" $appcontentsdirectory/Info.plist



######### copy the command-line-demo #############################
mkdir $packageFolder/command-line-demo
cp "$rootDirectory/command-line-demo/xcode/hedrotReceiverDemo" "$packageFolder/command-line-demo/"

######### copy the firmware #############################
mkdir $packageFolder/firmware
cp "$rootDirectory/firmware/build/hedrot-firmware.ino.hex" "$packageFolder/firmware/hedrot-firmware_version_$headtrackerFirmwareVersion.hex"
cp "$rootDirectory/firmware/CHANGELOG.txt" "$packageFolder/firmware/"

######### copy the doc #############################
cp "$rootDirectory/doc/hedrot user manual.pdf" "$packageFolder/"

######### copy the license file #############################
cp "$rootDirectory/LICENSE" "$packageFolder/"

######### copy the CHANGELOG.txt file #############################
cp "$rootDirectory/CHANGELOG.txt" "$packageFolder/"

######### copy and update the main README.txt file #############################
cp "$rootDirectory/scripts/README-DISTRIBUTION.txt" "$packageFolder/README.txt"
perl -pi -w -e  "s/Hedrot Version: XXXX/Hedrot Version: $hedrotVersion/g;" $packageFolder/README.txt
perl -pi -w -e  "s/Firmware Version: YYYY/Firmware Version: $headtrackerFirmwareVersion/g;" $packageFolder/README.txt
perl -pi -w -e  "s/hedrot library Version: ZZZZ/hedrot library Version: $libhedrotVersion/g;" $packageFolder/README.txt



######### DMG #############################

#erase the DMG if it exists
rm "$rootDirectory/build/$packageName.dmg"

#create the DMG
touch $destDirectory/.Trash
hdiutil create "$rootDirectory/build/$packageName.dmg" -volname "$packageName" -fs HFS+ -srcfolder $destDirectory

#erase temp build directory
rm -rf $destDirectory/*