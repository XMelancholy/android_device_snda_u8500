/* needed for bluetoothd (Bluez 5) */
#define CONTROL_PERMS_APPEND \
    { "bluetoothd", AID_BLUETOOTH, AID_BLUETOOTH }, \
    { "bluetoothd-snoop", AID_NOBODY, AID_NOBODY },
