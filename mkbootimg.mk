LOCAL_PATH := $(call my-dir)

INSTALLED_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img
$(INSTALLED_BOOTIMAGE_TARGET): $(PRODUCT_OUT)/kernel $(recovery_ramdisk) $(INSTALLED_RAMDISK_TARGET) $(MKBOOTIMG) $(MINIGZIP) $(INTERNAL_BOOTIMAGE_FILES)
	$(call pretty,"Boot image: $@")
	$(hide) mkdir -p $(PRODUCT_OUT)/mkbootimg/
	$(hide) cp -R $(PRODUCT_OUT)/root/* $(PRODUCT_OUT)/mkbootimg/
	$(hide) cp -R $(PRODUCT_OUT)/../../../../device/snda/u8500/prebuilt/root/init $(PRODUCT_OUT)/mkbootimg/
	$(hide) $(MKBOOTFS) $(PRODUCT_OUT)/mkbootimg > $(PRODUCT_OUT)/mkbootimg.cpio
	$(hide) cat $(PRODUCT_OUT)/mkbootimg.cpio | gzip > $(PRODUCT_OUT)/mkbootimg.fs
	$(hide) rm -rf $(PRODUCT_OUT)/system/bin/recovery
	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/mkbootimg.fs --cmdline "$(BOARD_KERNEL_CMDLINE)" --base $(BOARD_KERNEL_BASE) --pagesize $(BOARD_KERNEL_PAGESIZE) --ramdisk_offset  $(BOARD_MKBOOTIMG_ARGS) -o $@

INSTALLED_RECOVERYIMAGE_TARGET := $(PRODUCT_OUT)/recovery.img
$(INSTALLED_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) \
	$(recovery_ramdisk) \
	$(recovery_kernel)
	@echo ----- Making recovery image ------
	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/ramdisk-recovery.img --cmdline "$(BOARD_KERNEL_CMDLINE)" --base $(BOARD_KERNEL_BASE) --pagesize $(BOARD_KERNEL_PAGESIZE) --ramdisk_offset  $(BOARD_MKBOOTIMG_ARGS)  -o $@
	@echo ----- Made recovery image -------- $@
