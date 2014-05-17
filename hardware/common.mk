
# modem_lib
PRODUCT_PACKAGES += \
   libisimessage \
   libmalat \
   libmalcs \
   libmalftd \
   libmalgpds \
   libmalgss \
   libmalmce \
   libmalmis \
   libmalmon \
   libmalmte \
   libmalnet \
   libmalnvd \
   libmalpipe \
   libmalrf \
   libmalsms \
   libmaluicc \
   libmalutil \
   libshmnetlnk \
   libphonet

# cspsa
PRODUCT_PACKAGES += \
   cspsa-server \
   libcspsa \
   cspsa.conf \
   cspsa-cmd \
   libcspsa-core \
   libcspsa-ll-file

# cg2900
#PRODUCT_PACKAGES += \
#   ste-cg29xx_ctrl

FMRADIO_CG2900_SET_TX_ONLY := false
FMRADIO_CG2900_SET_RX_ONLY := false

# fm_cg2900
PRODUCT_PACKAGES += \
   libfmradio.cg2900 \
   libfmradio.cg2900_tx \
   libfmradio.cg2900_rx \

PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/FM/config/com.stericsson.hardware.fm.receiver.xml:system/etc/permissions/com.stericsson.hardware.fm.receiver.xml \
   $(LOCAL_PATH)/FM/config/com.stericsson.hardware.fm.transmitter.xml:system/etc/permissions/com.stericsson.hardware.fm.transmitter.xml

# Dbus
PRODUCT_PACKAGES += \
   dbus-daemon \
   libdbus

# Bluez 5 libs
PRODUCT_PACKAGES += \
    libbtio \
    libbluetooth

# setup functions to initialize the encoder and decoder for A2DP streams
PRODUCT_PACKAGES += \
    libsbc

# Bluez daemon, android hal and haltest
PRODUCT_PACKAGES += \
    bluetoothd \
    bluetoothd-snoop \
    bluetooth.default \
    audio.a2dp.default \
    haltest

# Bluez btmon
PRODUCT_PACKAGES += \
    btmon

# Bluez tools
PRODUCT_PACKAGES += \
    hciattach \
    hciconfig \
    hcitool \
    hcidump \
    rfcomm \
    rctest \
    l2test \
    l2ping \
    sdptool \
    ciptool \
    bccmd \
    btproxy

PRODUCT_PACKAGES += \
   chargemode \
   watchdog-kicker

# bluez Configs
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/external/bluetooth/data/audio.conf:system/etc/bluetooth/audio.conf \
   $(LOCAL_PATH)/external/bluetooth/data/blacklist.conf:system/etc/bluetooth/blacklist.conf \
   $(LOCAL_PATH)/external/bluetooth/data/input.conf:system/etc/bluetooth/input.conf \
   $(LOCAL_PATH)/external/bluetooth/data/main.conf:system/etc/bluetooth/main.conf \
   $(LOCAL_PATH)/external/bluetooth/data/network.conf:system/etc/bluetooth/network.conf \
   $(LOCAL_PATH)/external/bluetooth/data/stack.conf:system/etc/bluetooth/stack.conf
