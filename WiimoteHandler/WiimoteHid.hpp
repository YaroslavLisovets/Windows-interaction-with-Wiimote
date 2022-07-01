#ifndef WIIMOTEHANDLER_WIIMOTEHID_HPP
#define WIIMOTEHANDLER_WIIMOTEHID_HPP

#include <WinSock2.h>
#include <bluetoothapis.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#include <cwchar>

#define WM_VENDOR_ID              0x057E
#define WM_PRODUCT_ID             0x0306

struct wiimote_hid {
    HANDLE device_handle;
    OVERLAPPED hid_overlap;
    byte buffer[22];
    void write();
    unsigned int read();
};

wiimote_hid GetWiimoteHid();

struct bluetooth_device {
    HANDLE hRadio;
    BLUETOOTH_DEVICE_INFO_STRUCT deviceInfo;
};

template<typename T>
void ProcessWiimotes(bool new_scan, const T &callback = nullptr);

bool ForgetWiimote(BLUETOOTH_DEVICE_INFO_STRUCT &deviceInfo);

bool AttachWiimote(HANDLE hRadio, const BLUETOOTH_RADIO_INFO &radio_info, BLUETOOTH_DEVICE_INFO_STRUCT &deviceInfo);

void wiimoteDisconnect(bluetooth_device *bluetoothDevice);

bluetooth_device *FindConnectWiimoteBLE();

#endif
