$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Dalvik vm for system.prop
$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)

# Inherit from u8500 device
$(call inherit-product, device/snda/u8500/u8500.mk)

PRODUCT_NAME := aosp_u8500
PRODUCT_DEVICE := u8500
PRODUCT_BRAND := Bambookphone
PRODUCT_MANUFACTURER := snda.com
PRODUCT_MODEL := Bambook S1
