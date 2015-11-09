#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <functional>

class UdpServer
{
	public:
		UdpServer(std::shared_ptr<boost::asio::io_service> &ioService, const std::string &ipAddress, int port,
				  std::function<void(std::shared_ptr<std::vector<boost::uint8_t> >)> callback);

	private:
		void beginReading();
		void handleReceive(const std::shared_ptr<boost::asio::ip::udp::endpoint> &endpoint,
						   const std::shared_ptr<std::vector<boost::uint8_t>> &buffer,
						   const boost::system::error_code& errorCode,
						   std::size_t bytesReceived);

	private:
		std::shared_ptr<boost::asio::io_service> m_ioService;
		boost::asio::ip::udp::socket m_socket;
		std::function<void(std::shared_ptr<std::vector<boost::uint8_t> >)> m_callback;
};

#endif // UDP_SERVER_H
