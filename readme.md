# Android_device_snda_u8500

Getting Started :

		curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
		chmod 755 ~/bin/repo
		mkdir cm10
		cd cm10
		repo init -u git://github.com/CyanogenMod/android.git -b jellybean
		repo sync

		mkdir -p device/snda/u8500
		git clone https://github.com/XMelancholy/android_device_snda_u8500.git -b cm10 device/snda/u8500


Now connect your phone which have runing CM10 :

		cd device/snda/u8500
		./extract-files.sh

		mkdir -p kernel/snda
		git clone https://github.com/XMelancholy/android_kernel_snda_u8500 kernel/snda/u8500

Download CM prebuilts : 

		./vendor/cm/get-prebuilts cd

You are ready to build :

		source build/envsetup.sh
		lunch cm_u8500-userdebug
		make otapackage

ENJOY!
