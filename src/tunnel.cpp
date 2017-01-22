#include <algorithm>
#include <iostream>
#include <bitset>
#include <functional>

#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/placeholders.hpp>

#include "tunnel.h"

namespace
{
	const int BUFFER_SIZE = 1500; // 1500 bytes, standard MTU
}

namespace Overpass
{
	class TunnelPrivate
	{
		public:
			TunnelPrivate(
			      const std::shared_ptr<boost::asio::io_service> &ioService,
			      Tunnel::ReadCallback callback, int interfaceFileDescriptor);

			void sendAsynchronously(Overpass::SharedBuffer buffer);

		private:
			void handleReadFromTunnel(const boost::system::error_code& error,
			                          std::size_t bytesRead);
			void sendToTunnel(Overpass::SharedBuffer buffer);

		private:
			std::shared_ptr<boost::asio::io_service> m_ioService;
			Tunnel::ReadCallback m_callback;
			boost::asio::posix::stream_descriptor m_streamDescriptor;
			uint8_t m_buffer[BUFFER_SIZE];
	};

	TunnelPrivate::TunnelPrivate(
	      const std::shared_ptr<boost::asio::io_service> &ioService,
	      Tunnel::ReadCallback callback, int interfaceFileDescriptor) :
	   m_ioService(ioService),
	   m_callback(callback),
	   m_streamDescriptor(*ioService)
	{
		m_streamDescriptor.assign(interfaceFileDescriptor);

		// Start reading from file descriptor as soon as the IO service is up.
		m_streamDescriptor.async_read_some(
		         boost::asio::buffer(m_buffer),
		         std::bind(&TunnelPrivate::handleReadFromTunnel, this,
		                   std::placeholders::_1,
		                   std::placeholders::_2));
	}

	void TunnelPrivate::sendAsynchronously(Overpass::SharedBuffer buffer)
	{
		m_ioService->post(std::bind(&TunnelPrivate::sendToTunnel, this, buffer));
	}

	void TunnelPrivate::handleReadFromTunnel(
	      const boost::system::error_code& error, std::size_t bytesRead)
	{
		if (error)
		{
			std::cerr << "Got error: " << error << std::endl;
		}
		else
		{
			if (bytesRead > 0)
			{
				Overpass::SharedBuffer sharedBuffer(new Overpass::Buffer(
				                                       m_buffer, m_buffer+bytesRead));

				m_ioService->post(std::bind(m_callback, sharedBuffer));
			}
			else if (bytesRead == 0)
			{
				perror("Unable to read from tunnel");
			}
		}

		m_streamDescriptor.async_read_some(
		         boost::asio::buffer(m_buffer),
		         std::bind(&TunnelPrivate::handleReadFromTunnel, this,
		                   std::placeholders::_1,
		                   std::placeholders::_2));
	}

	void TunnelPrivate::sendToTunnel(Overpass::SharedBuffer buffer)
	{
//		if (m_communicator->write(buffer->data(), buffer->size()) < 0)
//		{
//			perror("Unable to send to tunnel");
//		}
	}

	Tunnel::Tunnel(std::shared_ptr<boost::asio::io_service> ioService,
	               ReadCallback callback, int interfaceFileDescriptor) :
	   m_data(new TunnelPrivate(ioService, callback, interfaceFileDescriptor))
	{
	}

	Tunnel::~Tunnel()
	{
	}

	void Tunnel::sendToTunnel(Overpass::SharedBuffer buffer)
	{
		m_data->sendAsynchronously(buffer);
	}
}
