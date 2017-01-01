#!/bin/sh
echo "Before running this script, be sure that: "
echo "   1/ all binaries have been compiled"
echo "   2/ a pdf version of the doc has been created"
echo "   3/ the Max standalone hedrotReceiver has been built."

today=$(date +"%y-%m-%d")

thisDirectory=`pwd`
rootDirectory=$thisDirectory/../..
destDirectory=$rootDirectory/build/forPackaging

hedrotVersion=`awk '/^#define HEDROT_VERSION / { print $3 }' $rootDirectory/libhedrot/libhedrot.h`
hedrotVersionMessage="hedrot version: $hedrotVersion"
echo $hedrotVersionMessage

hedrotFirmwareVersion=`awk '/^#define HEDROT_FIRMWARE_VERSION / { print $3 }' $rootDirectory/firmware/hedrot-firmware/hedrot_comm_protocol.h`
hedrotFirmwareVersionMessage="Headtracker Firmware version: $hedrotFirmwareVersion"
echo $hedrotFirmwareVersionMessage

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
cp "$rootDirectory/firmware/build/hedrot-firmware.ino.hex" "$packageFolder/firmware/hedrot-firmware_version_$hedrotFirmwareVersion.hex"
cp "$rootDirectory/firmware/CHANGELOG.txt" "$packageFolder/firmware/"

######### copy the max source code and external #############################
mkdir $packageFolder/Max
cp $rootDirectory/Max/hedrotReceiver.json $packageFolder/Max/
cp $rootDirectory/Max/patches/*.maxpat $packageFolder/Max/
cp -R $rootDirectory/Max/hedrot_receiver.mxo $packageFolder/Max/

######### copy the doc #############################
cp "$rootDirectory/doc/hedrot user manual.pdf" "$packageFolder/"

######### copy the license file #############################
cp "$rootDirectory/LICENSE" "$packageFolder/"

######### copy the CHANGELOG.txt file #############################
cp "$rootDirectory/CHANGELOG.txt" "$packageFolder/"

######### copy and update the main README.txt file #############################
cp "$rootDirectory/scripts/README-DISTRIBUTION.txt" "$packageFolder/README.txt"
perl -pi -w -e  "s/Hedrot Version: XXXX/Hedrot Version: $hedrotVersion/g;" $packageFolder/README.txt
perl -pi -w -e  "s/Firmware Version: YYYY/Firmware Version: $hedrotFirmwareVersion/g;" $packageFolder/README.txt



######### DMG #############################

#erase the DMG if it exists
rm "$rootDirectory/build/$packageName.dmg"

#create the DMG
# the following line (creating a .Trash) is a hocus-pocus work-around that some times works when hdiutils does not succeed and returns an error 5341 (bug)
#touch $destDirectory/.Trash 
hdiutil create "$rootDirectory/build/$packageName.dmg" -volname "$packageName" -fs HFS+ -srcfolder $destDirectory

#erase temp build directory
rm -rf $destDirectory/*