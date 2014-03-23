# Inherit device configuration
$(call inherit-product, device/snda/u8500/full_u8500.mk)

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := u8500
PRODUCT_NAME := aosp_u8500

# Debug mode
ADDITIONAL_DEFAULT_PROPERTIES += ro.debuggable=1
ADDITIONAL_DEFAULT_PROPERTIES += ro.secure=0
ADDITIONAL_DEFAULT_PROPERTIES += ro.adb.secure=0
ADDITIONAL_DEFAULT_PROPERTIES += persist.service.adb.enable=1
