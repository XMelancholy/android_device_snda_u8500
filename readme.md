# Android_device_snda_u8500

	mkdir aospjb
	cd aospjb
	repo init -u https://android.googlesource.com/platform/manifest -b android-4.1.2_r2.1

	mkdir -p device/snda/u8500
	git clone https://github.com/XMelancholy/android_device_snda_u8500.git -b android-4.1.2_r2.1 device/snda/u8500

	mkdir -p vendor/snda/u8500
	git clone https://github.com/XMelancholy/android_vendor_snda_u8500 vendor/snda/u8500

	mkdir -p kernel/snda
	git clone https://github.com/XMelancholy/android_kernel_snda_u8500 kernel/snda/u8500

# patch

	patch -p1 < device/snda/u8500/patches/build.patch
	patch -p1 < device/snda/u8500/patches/hardware_libhardware_legacy.patch
	patch -p1 < device/snda/u8500/patches/system_core.patch
	patch -p1 < device/snda/u8500/patches/system_vold.patch
	
    还原
	patch -p1 -R < device/snda/u8500/patches/build.patch
	patch -p1 -R < device/snda/u8500/patches/hardware_libhardware_legacy.patch
	patch -p1 -R < device/snda/u8500/patches/system_core.patch
	patch -p1 -R < device/snda/u8500/patches/system_vold.patch
		
# build

	source build/envsetup.sh
	lunch aosp_u8500-userdebug
	make otapackage
