#ifndef UPNP_PORT_MAPPER_PRIVATE_H
#define UPNP_PORT_MAPPER_PRIVATE_H

#include <map>
#include <memory>
#include <mutex>

#include <boost/asio/deadline_timer.hpp>

#include <upnp/upnp.h>

#include "types.h"

namespace Overpass
{
	class PortMappingException : public Exception
	{
		public:
			PortMappingException(const std::string &what);
	};

	namespace internal
	{
		class UpnpDevice;

		class UpnpPortMapperPrivate :
		         public std::enable_shared_from_this<UpnpPortMapperPrivate>
		{
			public:
				UpnpPortMapperPrivate(const SharedIoService &ioService,
				                      std::uint16_t port);
				~UpnpPortMapperPrivate();

				void start();

				int upnpClientCallback(Upnp_EventType eventType, void *event);

			private:
				void upnpSearchAsync(const std::string &target, int timeout);
				void handleNewUpnpDevice(const Upnp_Discovery &discovery);
				void checkExpirationTimes(
				      const boost::system::error_code &errorCode);

				void updatePortMappings();

			private:
				std::uint16_t m_port;
				UpnpClient_Handle m_upnpClient;
				boost::asio::deadline_timer m_expirationTimer;

				std::mutex m_devicesMutex;
				std::map<std::string, std::shared_ptr<UpnpDevice>> m_devices;
		};
	}
}

#endif // UPNP_PORT_MAPPER_PRIVATE_H
