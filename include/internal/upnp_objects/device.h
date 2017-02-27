#ifndef DEVICE_H
#define DEVICE_H

#include <memory>
#include <string>
#include <vector>

#include <upnp/ixml.h>

#include "types.h"

namespace Overpass
{
	namespace internal
	{
		class UpnpService;

		class InvalidUpnpDeviceException : public Exception
		{
			public:
				InvalidUpnpDeviceException(const std::string &what);
		};

		class UpnpDevice
		{
			public:
				UpnpDevice(IXML_Node *node, const std::string &baseUrl,
				           int expiration);

				const std::string& friendlyName() const;
				const std::string& deviceType() const;
				const std::string& udn() const;

				int expiration() const;
				void setExpiration(int newExpiration);

				void getServicesByType(
				      const std::string &type,
				      std::vector<std::shared_ptr<UpnpService>> &services) const;

			private:
				std::string m_friendlyName;
				std::string m_deviceType;
				std::string m_udn;
				int m_expiration;

				std::vector<std::shared_ptr<UpnpDevice>> m_deviceList;
				std::vector<std::shared_ptr<UpnpService>> m_serviceList;
		};
	}
}

#endif // DEVICE_H
