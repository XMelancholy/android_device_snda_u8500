# Android_device_snda_u8500

Getting Started :

		curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
		chmod 755 ~/bin/repo
		mkdir cm11
		cd cm11
		repo init -u git://github.com/CyanogenMod/android.git -b cm11.0
		repo sync

		mkdir -p device/snda/u8500
		git clone https://github.com/XMelancholy/android_device_snda_u8500.git -b cm11.0 device/snda/u8500

Now connect your phone which have runing CM11 :

		cd device/snda/u8500
		./extract-files.sh

		cd ../../..
		mkdir -p kernel/snda
		git clone https://github.com/XMelancholy/android_kernel_snda_u8500 kernel/snda/u8500


Patch android source code :

		patch -p1 < device/snda/u8500/patches/bionic.patch
		patch -p1 < device/snda/u8500/patches/external_bluetooth_bluedroid.patch
		patch -p1 < device/snda/u8500/patches/frameworks_av.patch
		patch -p1 < device/snda/u8500/patches/frameworks_base.patch
		patch -p1 < device/snda/u8500/patches/frameworks_native.patch
		patch -p1 < device/snda/u8500/patches/hardware_libhardware.patch
		patch -p1 < device/snda/u8500/patches/system_core.patch



Our step is optional!!! Use only if you going to sync CM 11 source code daily, than simple revert each patch before you sync CM 10 source code :

		patch -p1 -R < device/snda/u8500/patches/bionic.patch
		patch -p1 -R < device/snda/u8500/patches/external_bluetooth_bluedroid.patch
		patch -p1 -R < device/snda/u8500/patches/frameworks_av.patch
		patch -p1 -R < device/snda/u8500/patches/frameworks_base.patch
		patch -p1 -R < device/snda/u8500/patches/frameworks_native.patch
		patch -p1 -R < device/snda/u8500/patches/hardware_libhardware.patch
		patch -p1 -R < device/snda/u8500/patches/system_core.patch
		repo forall -p -c 'git checkout -f'
		repo sync
		patch -p1 < device/snda/u8500/patches/bionic.patch
		patch -p1 < device/snda/u8500/patches/external_bluetooth_bluedroid.patch
		patch -p1 < device/snda/u8500/patches/frameworks_av.patch
		patch -p1 < device/snda/u8500/patches/frameworks_base.patch
		patch -p1 < device/snda/u8500/patches/frameworks_native.patch
		patch -p1 < device/snda/u8500/patches/hardware_libhardware.patch
		patch -p1 < device/snda/u8500/patches/system_core.patch


Download CM prebuilts : 

		./vendor/cm/get-prebuilts

You are ready to build :

		source build/envsetup.sh
		lunch cm_u8500-userdebug
		make otapackage

ENJOY!
