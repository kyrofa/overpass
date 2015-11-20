#ifndef TUNNEL_H
#define TUNNEL_H

#include <vector>
#include <thread>
#include <functional>

#include <boost/asio/io_service.hpp>

class Tunnel
{
	public:
		explicit Tunnel(int interfaceFileDescriptor);

		void start();
		void stop();

		//void send(const std::vector<std::uint8_t> &buffer);

	private:
		void readFromTunnel();

	private:
		/*!
		 * \brief The file descriptor of the virtual network interface.
		 */
		int m_fileDescriptor;

		/*!
		 * \brief The IO service to run asynchronous tasks.
		 */
		std::shared_ptr<boost::asio::io_service> m_ioService;

		/*!
		 * \brief Thread pool for reading from/writing to virtual network
		 *        interface.
		 */
		std::vector<std::thread> m_threadPool;
};

#endif // TUNNEL_H
