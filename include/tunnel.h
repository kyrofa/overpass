#ifndef TUNNEL_H
#define TUNNEL_H

#include <vector>
#include <thread>
#include <functional>
#include <memory>

#include <boost/asio/io_service.hpp>

#include "types.h"

namespace Overpass
{
	class TunnelPrivate;

	class Tunnel: private boost::noncopyable
	{
		public:
			typedef std::function<void (const Overpass::SharedBuffer&)> ReadCallback;

			Tunnel(std::shared_ptr<boost::asio::io_service> ioService,
			      ReadCallback callback, int interfaceFileDescriptor);

			/*!
			 * \brief Tunnel destructor. Doesn't actually do anything-- exists so
			 *        TunnelPrivate can be incomplete.
			 */
			~Tunnel();

			void sendToTunnel(Overpass::SharedBuffer buffer);

		private:
			std::unique_ptr<TunnelPrivate> m_data;
	};
}

#endif // TUNNEL_H
