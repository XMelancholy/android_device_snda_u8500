# Inherit device configuration
$(call inherit-product, device/snda/u8500/full_u8500.mk)

# Inherit CM common GSM stuff.
$(call inherit-product, vendor/cm/config/gsm.mk)

# Inherit CM common Phone stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

TARGET_BOOTANIMATION_NAME := 540
## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := u8500
PRODUCT_NAME := cm_u8500

# Torch
PRODUCT_PACKAGES += \
   Torch

# Debug mode
ADDITIONAL_DEFAULT_PROPERTIES += ro.debuggable=1
ADDITIONAL_DEFAULT_PROPERTIES += ro.secure=0
ADDITIONAL_DEFAULT_PROPERTIES += ro.adb.secure=0
ADDITIONAL_DEFAULT_PROPERTIES += persist.service.adb.enable=1
