#ifndef OVERPASS_SERVER_H
#define OVERPASS_SERVER_H

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/udp.hpp>

#include "types.h"

namespace Overpass
{
	class OverpassServerPrivate;

	class OverpassServer: private boost::noncopyable
	{
		public:
			typedef std::function<void (
			      const boost::asio::ip::udp::endpoint&,
			      const SharedBuffer&)> ReadCallback;

			OverpassServer(const SharedIoService &ioService,
			               const std::string &ipAddress, int port,
			               ReadCallback callback, std::size_t bufferSize = 1500);

			// OverpassServer destructor. Does nothing other than ensure
			// OverpassServerPrivate can be used incomplete.
			~OverpassServer();

			void sendTo(const boost::asio::ip::udp::endpoint &destination,
			            const SharedBuffer &buffer) const;

		private:
			// Using a shared_ptr instead of unique_ptr because of
			// enable_shared_from_this.
			std::shared_ptr<OverpassServerPrivate> m_data;
	};
}

#endif // OVERPASS_SERVER_H
