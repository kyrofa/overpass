#ifndef SOCKET_COMMUNICATOR_H
#define SOCKET_COMMUNICATOR_H

#include <memory>

#include <boost/system/error_code.hpp>

#include "types.h"

namespace Overpass
{
	namespace internal
	{
		typedef std::function<void (const Overpass::SharedBuffer&)> ReadCallback;
	}

	template <typename T>
	class SocketCommunicator;

	template <typename T>
	std::shared_ptr<SocketCommunicator<T>> makeSocketCommunicator(
	      const SharedIoService &ioService,
	      internal::ReadCallback callback,
	      const std::shared_ptr<T> &socket,
	      std::size_t bufferSize = 1500);

	template <typename T>
	class SocketCommunicator :
	      public std::enable_shared_from_this<SocketCommunicator<T>>
	{
		public:
			SocketCommunicator(const SharedIoService &ioService,
			                   internal::ReadCallback callback,
			                   const std::shared_ptr<T> &socket,
			                   std::size_t bufferSize = 1500):
			   m_ioService(ioService),
			   m_callback(callback),
			   m_bufferSize(bufferSize),
			   m_socket(socket)
			{
			}

			void send(const Overpass::SharedBuffer &buffer) const
			{
				// Does nothing yet.
			}

			friend std::shared_ptr<SocketCommunicator> makeSocketCommunicator<T>(
			      const SharedIoService &ioService,
			      internal::ReadCallback callback,
			      const std::shared_ptr<T> &socket,
			      std::size_t bufferSize);

		private:

			void beginReading() const
			{
				Overpass::SharedBuffer buffer(new Overpass::Buffer(m_bufferSize));

				m_socket->async_read_some(boost::asio::buffer(*buffer),
				                          std::bind(&SocketCommunicator::handleRead,
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
					std::cerr << "Got error: " << error << std::endl;
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

		private:
			SharedIoService m_ioService;
			internal::ReadCallback m_callback;
			std::size_t m_bufferSize;
			std::shared_ptr<T> m_socket;
	};

	template <typename T>
	std::shared_ptr<SocketCommunicator<T>> makeSocketCommunicator(
	      const SharedIoService &ioService,
	      internal::ReadCallback callback,
	      const std::shared_ptr<T> &socket,
	      std::size_t bufferSize)
	{
		auto communicator = std::make_shared<SocketCommunicator<T> >(
		                       ioService, callback, socket, bufferSize);

		// Ideally the constructor would do this, but it can't as shared_from_this
		// can only be used once at least one shared pointer is pointing to the
		// object in question (in this case, the communicator).
		communicator->beginReading();
		return communicator;
	}
}

#endif // SOCKET_COMMUNICATOR_H
