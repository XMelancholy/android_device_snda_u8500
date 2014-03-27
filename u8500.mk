# Added platform feature permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
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
    $(LOCAL_PATH)/prebuilt/system/app/Superuser.apk:system/app/Superuser.apk \
    $(LOCAL_PATH)/prebuilt/system/xbin/su:system/xbin/su \
    $(LOCAL_PATH)/prebuilt/system/bin/busybox:system/bin/busybox \
    $(LOCAL_PATH)/prebuilt/system/bin/usbid_init.sh:system/bin/usbid_init.sh \
    $(LOCAL_PATH)/prebuilt/system/lib/egl/egl.cfg:system/lib/egl/egl.cfg \
    $(LOCAL_PATH)/prebuilt/system/etc/media_codecs.xml:system/etc/media_codecs.xml \
    $(LOCAL_PATH)/prebuilt/system/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(LOCAL_PATH)/prebuilt/system/etc/asound.conf:system/etc/asound.conf \
    $(LOCAL_PATH)/prebuilt/system/etc/cacert.txt:system/etc/cacert.txt \
    $(LOCAL_PATH)/prebuilt/system/etc/cspsa.conf:system/etc/cspsa.conf \
    $(LOCAL_PATH)/prebuilt/system/etc/dbus.conf:system/etc/dbus.conf \
    $(LOCAL_PATH)/prebuilt/system/etc/omxloaders:system/etc/omxloaders \
    $(LOCAL_PATH)/prebuilt/system/etc/ste_modem.sh:system/etc/ste_modem.sh \
    $(LOCAL_PATH)/prebuilt/system/etc/vold.fstab:system/etc/vold.fstab \
    $(LOCAL_PATH)/prebuilt/system/etc/wifi/p2p_supplicant.conf:system/etc/wifi/p2p_supplicant.conf \
    $(LOCAL_PATH)/prebuilt/system/etc/wifi/hostapd.conf:system/etc/wifi/hostapd.conf \
    $(LOCAL_PATH)/prebuilt/system/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

# ALSA Configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/hardware/libasound/src/conf/cards/aliases.conf:system/usr/share/alsa/cards/aliases.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/center_lfe.conf:system/usr/share/alsa/pcm/center_lfe.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/default.conf:system/usr/share/alsa/pcm/default.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/dmix.conf:system/usr/share/alsa/pcm/dmix.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/dpl.conf:system/usr/share/alsa/pcm/dpl.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/dsnoop.conf:system/usr/share/alsa/pcm/dsnoop.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/front.conf:system/usr/share/alsa/pcm/front.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/iec958.conf:system/usr/share/alsa/pcm/iec958.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/modem.conf:system/usr/share/alsa/pcm/modem.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/rear.conf:system/usr/share/alsa/pcm/rear.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/side.conf:system/usr/share/alsa/pcm/side.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/surround40.conf:system/usr/share/alsa/pcm/surround40.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/surround41.conf:system/usr/share/alsa/pcm/surround41.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/surround50.conf:system/usr/share/alsa/pcm/surround50.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/surround51.conf:system/usr/share/alsa/pcm/surround51.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/pcm/surround71.conf:system/usr/share/alsa/pcm/surround71.conf \
    $(LOCAL_PATH)/hardware/libasound/src/conf/alsa.conf:system/usr/share/alsa/alsa.conf

# BT Config
PRODUCT_COPY_FILES += \
    system/bluetooth/data/main.conf:system/etc/bluetooth/main.conf

# APN
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/prebuilt/system/etc/apns-conf.xml:system/etc/apns-conf.xml

# Hostapd
PRODUCT_PACKAGES += \
    hostapd_cli \
    hostapd

# Libasound
PRODUCT_PACKAGES += \
   libasound

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

# libblt_hw
PRODUCT_PACKAGES += \
    libblt_hw

# libu300-ril
PRODUCT_PACKAGES += \
    libu300-ril \
    libu300-parser \
    ril_config

# for build.prop
PRODUCT_PROPERTY_OVERRIDES += \
   media.aac_51_output_enabled=1 \
   persist.sys.usb.config=mtp,adb \
   ro.sf.lcd_density=240 \
   wifi.interface=wlan0

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
