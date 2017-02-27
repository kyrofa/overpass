#ifndef UPNP_PORT_MAPPER_H
#define UPNP_PORT_MAPPER_H

#include <memory>

#include "types.h"

namespace Overpass
{
	namespace internal
	{
		class UpnpPortMapperPrivate;
	}

	class UpnpPortMapper
	{
		public:
			UpnpPortMapper(const SharedIoService &ioService, std::uint16_t port);

		private:
			// Using a shared_ptr instead of unique_ptr because of
			// enable_shared_from_this.
			std::shared_ptr<internal::UpnpPortMapperPrivate> m_data;
	};
}

#endif // UPNP_PORT_MAPPER_H
