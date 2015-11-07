#ifndef VIRTUAL_INTERFACE_H
#define VIRTUAL_INTERFACE_H

#include <string>

std::string createVirtualInterface(const std::string &requestedDeviceName);
bool assignDeviceAddress(const std::string &deviceName,
						 const std::string &ipAddress,
						 const std::string &netmask);

#endif // VIRTUAL_INTERFACE_H
