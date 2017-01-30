#include <iostream>

#include <boost/asio/ip/udp.hpp>

#include "overpass_server.h"

using namespace Overpass;

namespace Overpass
{
	class OverpassServerPrivate :
	      public std::enable_shared_from_this<OverpassServerPrivate>
	{
		public:
			OverpassServerPrivate(const SharedIoService &ioService,
			                      const std::string &ipAddress,int port,
			                      OverpassServer::ReadCallback callback,
			                      std::size_t bufferSize) :
			   m_ioService(ioService),
			   m_callback(callback),
			   m_socket(*m_ioService, boost::asio::ip::udp::endpoint(
			               boost::asio::ip::address::from_string(ipAddress),
			               port)),
			   m_bufferSize(bufferSize)
			{
			}

			void beginReading()
			{
				using boost::asio::ip::udp;

				SharedBuffer buffer(new Overpass::Buffer(m_bufferSize));

				// Allocate a new endpoint to hold the sender's information.
				std::shared_ptr<udp::endpoint> endpoint(new udp::endpoint);

				m_socket.async_receive_from(
				         boost::asio::buffer(*buffer), *endpoint,
				         std::bind(&OverpassServerPrivate::handleRead,
				                   shared_from_this(), endpoint, buffer,
				                   std::placeholders::_1, std::placeholders::_2));
			}

			void sendTo(const boost::asio::ip::udp::endpoint &destination,
			            const SharedBuffer &buffer)
			{
				m_socket.send_to(boost::asio::buffer(*buffer), destination);
			}

		private:
			void handleRead(
			      const std::shared_ptr<boost::asio::ip::udp::endpoint> &sender,
			      const SharedBuffer &buffer,
			      const boost::system::error_code &error,
			      std::size_t bytesRead)
			{
				if (error)
				{
					std::cerr << "Error reading: " << error << std::endl;
					return;
				}

				if (bytesRead == 0)
				{
					std::cerr << "Received zero bytes?" << std::endl;
					return;
				}

				// We got something: dispatch callback with buffer.
				m_ioService->post(std::bind(m_callback, *sender, buffer));

				// Read some more.
				beginReading();
			}

		private:
			SharedIoService m_ioService;
			OverpassServer::ReadCallback m_callback;
			boost::asio::ip::udp::socket m_socket;
			std::size_t m_bufferSize;
	};
}

OverpassServer::OverpassServer(const SharedIoService &ioService,
                               const std::string &ipAddress, int port,
                               ReadCallback callback,
                               std::size_t bufferSize) :
   m_data(new OverpassServerPrivate(
             ioService, ipAddress, port, callback, bufferSize))
{
	m_data->beginReading();
}

OverpassServer::~OverpassServer()
{
}

void OverpassServer::sendTo(const boost::asio::ip::udp::endpoint &destination,
                            const SharedBuffer &buffer) const
{
	m_data->sendTo(destination, buffer);
}
