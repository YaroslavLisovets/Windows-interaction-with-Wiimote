
#include "WiimoteHid.hpp"

int main() {
    auto wiimoteBLE = FindConnectWiimoteBLE();
    wiimote_hid wiimote = GetWiimoteHid();
    wiimote.buffer[0] = 0x11;
    wiimote.buffer[1] = 0x10|0x20;
    wiimote.write();
    wiimote.read();
    system("pause");
    CloseHandle(wiimote.device_handle);
    wiimoteDisconnect(wiimoteBLE);
    return 0;
}



