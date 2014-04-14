# Added platform feature permissions
PRODUCT_COPY_FILES += \
   frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
   frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
   frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
   frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
   frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
   frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
   frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
   frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
   frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
   frameworks/native/data/etc/android.hardware.sensor.barometer.xml:system/etc/permissions/android.hardware.sensor.barometer.xml \
   frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
   frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
   frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
   frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
   frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
   packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml
    
# GSM
PRODUCT_COPY_FILES += \
   frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml

# Key layouts and touchscreen
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/prebuilt/system/usr/keylayout/AB8500_Hs_Button.kl:system/usr/keylayout/AB8500_Hs_Button.kl \
   $(LOCAL_PATH)/prebuilt/system/usr/keylayout/STMPE-keypad.kl:system/usr/keylayout/STMPE-keypad.kl \
   $(LOCAL_PATH)/prebuilt/system/usr/keylayout/tc3589x-keypad.kl:system/usr/keylayout/tc3589x-keypad.kl \
   $(LOCAL_PATH)/prebuilt/system/usr/keylayout/ux500-ske-keypad.kl:system/usr/keylayout/ux500-ske-keypad.kl \
   $(LOCAL_PATH)/prebuilt/system/usr/keylayout/ux500-ske-keypad-qwertz.kl:system/usr/keylayout/ux500-ske-keypad-qwertz.kl \
   $(LOCAL_PATH)/prebuilt/system/usr/idc/cyttsp-spi.idc:system/usr/idc/cyttsp-spi.idc \
   $(LOCAL_PATH)/prebuilt/system/usr/idc/synaptics_rmi4_i2c.idc:system/usr/idc/synaptics_rmi4_i2c.idc \
   $(LOCAL_PATH)/prebuilt/system/usr/idc/bu21013_ts.idc:system/usr/idc/bu21013_ts.idc \
   $(LOCAL_PATH)/prebuilt/system/usr/idc/tp_ft5306.idc:system/usr/idc/tp_ft5306.idc

# Custom init / uevent
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/prebuilt/root/init.rc:root/init.rc \
   $(LOCAL_PATH)/prebuilt/root/fstab.st-ericsson:root/fstab.st-ericsson \
   $(LOCAL_PATH)/prebuilt/root/init.st-ericsson.rc:root/init.st-ericsson.rc \
   $(LOCAL_PATH)/prebuilt/root/ueventd.st-ericsson.rc:root/ueventd.st-ericsson.rc

# USB function switching
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/prebuilt/root/init.st-ericsson.usb.rc:root/init.st-ericsson.usb.rc

# Configs
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/prebuilt/system/bin/usbid_init.sh:system/bin/usbid_init.sh \
   $(LOCAL_PATH)/prebuilt/system/lib/egl/egl.cfg:system/lib/egl/egl.cfg \
   $(LOCAL_PATH)/prebuilt/system/etc/media_codecs.xml:system/etc/media_codecs.xml \
   $(LOCAL_PATH)/prebuilt/system/etc/media_profiles.xml:system/etc/media_profiles.xml \
   $(LOCAL_PATH)/prebuilt/system/etc/asound.conf:system/etc/asound.conf \
   $(LOCAL_PATH)/prebuilt/system/etc/cacert.txt:system/etc/cacert.txt \
   $(LOCAL_PATH)/prebuilt/system/etc/cspsa.conf:system/etc/cspsa.conf \
   $(LOCAL_PATH)/prebuilt/system/etc/dbus.conf:system/etc/dbus.conf \
   $(LOCAL_PATH)/prebuilt/system/etc/omxloaders:system/etc/omxloaders \
   $(LOCAL_PATH)/prebuilt/system/etc/ril_config:system/etc/ril_config \
   $(LOCAL_PATH)/prebuilt/system/etc/ste_modem.sh:system/etc/ste_modem.sh \
   $(LOCAL_PATH)/prebuilt/system/etc/init.d/01stesetup:system/etc/init.d/01stesetup \
   $(LOCAL_PATH)/prebuilt/system/etc/init.d/10wireless:system/etc/init.d/10wireless \
   $(LOCAL_PATH)/prebuilt/system/etc/wifi/p2p_supplicant.conf:system/etc/wifi/p2p_supplicant.conf \
   $(LOCAL_PATH)/prebuilt/system/etc/wifi/hostapd.conf:system/etc/wifi/hostapd.conf \
   $(LOCAL_PATH)/prebuilt/system/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

# New cn_binary needed for mobile network for cm11
# cn_binary source build for kk ,jb cn_binary it's not work
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/prebuilt/system/bin/cn_server:system/bin/cn_server

# Agps config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/prebuilt/system/etc/LbsConfig.cfg:system/etc/LbsConfig.cfg \
    $(LOCAL_PATH)/prebuilt/system/etc/LbsLogConfig.cfg:system/etc/LbsLogConfig.cfg \
    $(LOCAL_PATH)/prebuilt/system/etc/LbsPgpsConfig.cfg:system/etc/LbsPgpsConfig.cfg \
    $(LOCAL_PATH)/prebuilt/system/etc/LbsPltConfig.cfg:system/etc/LbsPltConfig.cfg

# Filesystem management tools
PRODUCT_PACKAGES += \
   make_ext4fs \
   setup_fs \
   e2fsck

# Hostapd
PRODUCT_PACKAGES += \
   hostapd_cli \
   hostapd

# STE Audio
PRODUCT_PACKAGES += \
   libasound \
   libalsactrl \
   libmmprobe

# Audio HAL
PRODUCT_PACKAGES += \
   audio_policy.montblanc \
   audio.primary.montblanc

# A2DP and USB
PRODUCT_PACKAGES += \
   audio.a2dp.default \
   audio.usb.default

# Lights
PRODUCT_PACKAGES += \
   lights.montblanc
   
# libgralloc
PRODUCT_PACKAGES += \
   gralloc.montblanc

# libhwcomposer
PRODUCT_PACKAGES += \
   hwcomposer.montblanc

# libcopybit
PRODUCT_PACKAGES += \
   copybit.montblanc

# libpower
PRODUCT_PACKAGES += \
   power.montblanc

# libsensors
PRODUCT_PACKAGES += \
   sensors.montblanc

# libmemtrack
PRODUCT_PACKAGES += \
   memtrack.montblanc

# libstagefrighthw
PRODUCT_PACKAGES += \
   libstagefrighthw

# libblt_hw
PRODUCT_PACKAGES += \
   libblt_hw

# libomxil-bellagio
PRODUCT_PACKAGES += \
   libomxil-bellagio

# bcmdhd_net_iface
PRODUCT_PACKAGES += \
   libnetcmdiface

# Netlink
PRODUCT_PACKAGES += \
   libnl_2

# Libaudioparameter
PRODUCT_PACKAGES += \
   libaudioparameter

# libdbus
PRODUCT_PACKAGES += \
   libdbus \
   dbus-daemon

# FileExplorer
#PRODUCT_PACKAGES += \
#   FileExplorer

# for build.prop
PRODUCT_PROPERTY_OVERRIDES += \
   media.aac_51_output_enabled=1 \
   persist.sys.usb.config=mtp,adb \
   ro.sf.lcd_density=240 \
   wifi.interface=wlan0

# kitkat bootloop fix. More info http://source.android.com/devices/tuning.html
PRODUCT_PROPERTY_OVERRIDES += \
    ro.zygote.disable_gl_preload=1

# This device is xhdpi.  However the platform doesn't
# currently contain all of the bitmaps at xhdpi density so
# we do this little trick to fall back to the hdpi version
# if the xhdpi doesn't exist.
PRODUCT_AAPT_CONFIG := normal mdpi hdpi
PRODUCT_AAPT_PREF_CONFIG := hdpi

# We have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_eu_supl.mk)

DEVICE_PACKAGE_OVERLAYS += device/snda/u8500/overlay

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0

$(call inherit-product-if-exists, vendor/snda/u8500/u8500-vendor.mk)
