#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include <memory>
#include <iostream>

#include <boost/system/error_code.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>

#include "types.h"

namespace Overpass
{
	typedef std::function<void (const SharedBuffer&)> ReadCallback;

	template <typename T>
	class StreamServer;

	template <typename T>
	std::shared_ptr<StreamServer<T>> makeStreamServer(
	      const SharedIoService &ioService,
	      ReadCallback callback,
	      const std::shared_ptr<T> &socket,
	      std::size_t bufferSize = 1500);

	template <typename T>
	using SharedStreamServer = std::shared_ptr<StreamServer<T>>;

	template <typename T>
	class StreamServer :
	      public std::enable_shared_from_this<StreamServer<T>>
	{
		public:
			StreamServer(const SharedIoService &ioService,
			             ReadCallback callback,
			             const std::shared_ptr<T> &socket,
			             std::size_t bufferSize = 1500):
			   m_ioService(ioService),
			   m_callback(callback),
			   m_bufferSize(bufferSize),
			   m_socket(socket)
			{
			}

			void write(const Overpass::SharedBuffer &buffer) const
			{
				boost::asio::async_write(m_socket,
				                         boost::asio::buffer(*buffer),
				                         std::bind(&StreamServer::handleWrite,
				                                   this->shared_from_this(),
				                                   std::placeholders::_1,
				                                   std::placeholders::_2));
			}

			friend std::shared_ptr<StreamServer> makeStreamServer<T>(
			      const SharedIoService &ioService,
			      ReadCallback callback,
			      const std::shared_ptr<T> &socket,
			      std::size_t bufferSize);

		private:

			void beginReading() const
			{
				Overpass::SharedBuffer buffer(new Overpass::Buffer(m_bufferSize));

				m_socket->async_read_some(boost::asio::buffer(*buffer),
				                          std::bind(&StreamServer::handleRead,
				                                    this->shared_from_this(),
				                                    buffer,
				                                    std::placeholders::_1,
				                                    std::placeholders::_2));
			}

			void handleRead(const Overpass::SharedBuffer &buffer,
			                const boost::system::error_code &error,
			                std::size_t bytesRead) const
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
				m_ioService->post(std::bind(m_callback, buffer));

				// Read some more.
				beginReading();
			}

			void handleWrite(const boost::system::error_code &error,
			                 std::size_t bytesRead) const
			{
				if (error)
				{
					std::cerr << "Error writing: " << error << std::endl;
				}

				if (bytesRead == 0)
				{
					std::cerr << "Sent zero bytes?" << std::endl;
				}
			}

		private:
			SharedIoService m_ioService;
			ReadCallback m_callback;
			std::size_t m_bufferSize;
			std::shared_ptr<T> m_socket;
	};

	template <typename T>
	std::shared_ptr<StreamServer<T>> makeStreamServer(
	      const SharedIoService &ioService,
	      ReadCallback callback,
	      const std::shared_ptr<T> &socket,
	      std::size_t bufferSize)
	{
		auto communicator = std::make_shared<StreamServer<T> >(
		                       ioService, callback, socket, bufferSize);

		// Ideally the constructor would do this, but it can't as shared_from_this
		// can only be used once at least one shared pointer is pointing to the
		// object in question (in this case, the communicator).
		communicator->beginReading();
		return communicator;
	}
}

#endif // STREAM_SERVER_H
