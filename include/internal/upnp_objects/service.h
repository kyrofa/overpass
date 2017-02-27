#ifndef SERVICE_H
#define SERVICE_H

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

		class UpnpServiceEnabledException : public Exception
		{
			public:
				UpnpServiceEnabledException(const std::string &what,
				                            const UpnpService &service);
		};

		class UpnpService
		{
			public:
				UpnpService(IXML_Node *node, const std::string &baseUrl);

				const std::string& serviceType() const;
				const std::string& controlUrl() const;
				const std::string& eventSubUrl() const;

				bool enabled(int controlPoint) const;

			private:
				std::string m_serviceType;
				std::string m_controlUrl;
				std::string m_eventSubUrl;
		};
	}
}

#endif // SERVICE_H
