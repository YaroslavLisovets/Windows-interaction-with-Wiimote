#include "WiimoteHid.hpp"


void wiimote_hid::write() {
    DWORD bytes;
    WriteFile(this->device_handle, this->buffer, 22, &bytes,
              &this->hid_overlap);
}

unsigned int wiimote_hid::read() {
    DWORD b = 0, r;
    while (not b) {
        if (!ReadFile(this->device_handle, this->buffer, 22, &b, &this->hid_overlap)) {
            r = WaitForSingleObject(this->hid_overlap.hEvent, 10);
            if (r == WAIT_TIMEOUT) {
                CancelIo(this->device_handle);
                ResetEvent(this->hid_overlap.hEvent);
                b = 0;
                continue;
            } else if (r == WAIT_FAILED) {
                b = 0;
                continue;
            }
            if (!GetOverlappedResult(this->device_handle, &this->hid_overlap, &b, 0)) {
                b = 0;
                continue;
            }
            ResetEvent(this->hid_overlap.hEvent);
            continue;
        }
    }
    return b;
}

struct wiimote_hid GetWiimoteHid() {
    GUID device_id;
    HANDLE dev;
    HDEVINFO device_info;
    int index;
    DWORD len;
    SP_DEVICE_INTERFACE_DATA device_data;
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = nullptr;
    HIDD_ATTRIBUTES attr;
    device_data.cbSize = sizeof(device_data);
    index = 0;
    HidD_GetHidGuid(&device_id);
    device_info = SetupDiGetClassDevs(&device_id, nullptr, nullptr, (DIGCF_DEVICEINTERFACE | DIGCF_PRESENT));
    struct wiimote_hid wiimote{};
    for (;; index++) {
        if (detail_data)
            free(detail_data);
        if (!SetupDiEnumDeviceInterfaces(device_info, nullptr, &device_id,index, &device_data))
            break;
        SetupDiGetDeviceInterfaceDetail(device_info, &device_data,
                    nullptr, 0, &len, nullptr);
        detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA_A *) malloc(len);
        detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_data,
                         detail_data, len, nullptr, nullptr))
            continue;
        dev = CreateFile(detail_data->DevicePath, (GENERIC_READ | GENERIC_WRITE),
                         (FILE_SHARE_READ | FILE_SHARE_WRITE), nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED,
                         nullptr);
        if (dev == INVALID_HANDLE_VALUE)
            continue;
        attr.Size = sizeof(attr);
        HidD_GetAttributes(dev, &attr);
        if ((attr.VendorID == WM_VENDOR_ID)
            && ((attr.ProductID == WM_PRODUCT_ID))) {
            wiimote.device_handle = dev;
            wiimote.hid_overlap.hEvent = CreateEvent(nullptr, 1, 1, "");
            wiimote.hid_overlap.Offset = 0;
            wiimote.hid_overlap.OffsetHigh = 0;
            break;
        } else
            CloseHandle(dev);
    }
    return wiimote;
}

template<typename T>
void ProcessWiimotes(bool new_scan, const T &callback) {
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
    searchParams.dwSize = sizeof(searchParams);
    searchParams.fReturnAuthenticated = true;
    searchParams.fReturnRemembered = true;
    searchParams.fReturnConnected = true;
    searchParams.fReturnUnknown = true;
    searchParams.fIssueInquiry = new_scan;
    searchParams.cTimeoutMultiplier = 1;

    BLUETOOTH_FIND_RADIO_PARAMS radioParam;
    radioParam.dwSize = sizeof(radioParam);
    bool found = false;
    HANDLE hRadio;

    HBLUETOOTH_RADIO_FIND hFindRadio = BluetoothFindFirstRadio(&radioParam, &hRadio);
    while (hFindRadio) {
        BLUETOOTH_RADIO_INFO radioInfo;
        radioInfo.dwSize = sizeof(radioInfo);

        auto const radioInfoResults = BluetoothGetRadioInfo(hRadio, &radioInfo);
        if (ERROR_SUCCESS == radioInfoResults) {
            searchParams.hRadio = hRadio;

            BLUETOOTH_DEVICE_INFO deviceInfo;
            deviceInfo.dwSize = sizeof(deviceInfo);
            auto wiimote_name = L"Nintendo RVL-CNT-01";
            // Enumerate BT devices
            do {
                HBLUETOOTH_DEVICE_FIND hFindDevice = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
                do {
                    if (std::wcscmp(wiimote_name, deviceInfo.szName) == 0) {
                        if (deviceInfo.fConnected)
                            return;
                        callback(hRadio, radioInfo, deviceInfo);
                        found = true;
                    }
                } while (BluetoothFindNextDevice(hFindDevice, &deviceInfo));
            } while (not found);
        }

        if (false == BluetoothFindNextRadio(hFindRadio, &hRadio)) {
            CloseHandle(hRadio);
            BluetoothFindRadioClose(hFindRadio);
            hFindRadio = nullptr;
        }
    }
}

bool ForgetWiimote(BLUETOOTH_DEVICE_INFO_STRUCT &deviceInfo) {
    if (!deviceInfo.fConnected && deviceInfo.fRemembered) {
        BluetoothRemoveDevice(&deviceInfo.Address);
        deviceInfo.fRemembered = 0;
        return true;
    }
    return false;
}

bool AttachWiimote(HANDLE hRadio, const BLUETOOTH_RADIO_INFO &radio_info, BLUETOOTH_DEVICE_INFO_STRUCT &deviceInfo) {
    if (!deviceInfo.fConnected && !deviceInfo.fRemembered) {
        const DWORD hr = BluetoothSetServiceState(
                hRadio, &deviceInfo, &HumanInterfaceDeviceServiceClass_UUID, BLUETOOTH_SERVICE_ENABLE);
        if (FAILED(hr))
            return false;
        else
            return true;
    }
    return false;
}

void wiimoteDisconnect(bluetooth_device *bluetoothDevice) {
    BluetoothRemoveDevice(&bluetoothDevice->deviceInfo.Address);
}

bluetooth_device *FindConnectWiimoteBLE() {
    bluetooth_device *device;
    ProcessWiimotes(true, [&device](HANDLE hRadio, const BLUETOOTH_RADIO_INFO &radioInfo,
                                    BLUETOOTH_DEVICE_INFO_STRUCT &deviceInfo) {
        ForgetWiimote(deviceInfo);
        AttachWiimote(hRadio, radioInfo, deviceInfo);
        device = new bluetooth_device{hRadio, deviceInfo};
    });
    return device;
}

