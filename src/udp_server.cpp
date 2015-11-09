#include <iostream>

#include "include/udp_server.h"

UdpServer::UdpServer(std::shared_ptr<boost::asio::io_service> &ioService,
					 const std::string &ipAddress, int port,
					 std::function<void(std::shared_ptr<std::vector<boost::uint8_t> >)> callback) :
	m_ioService(ioService),
	m_socket(*ioService, boost::asio::ip::udp::endpoint(
				 boost::asio::ip::address::from_string(ipAddress), port)),
	m_callback(callback)
{
	beginReading();
}

void UdpServer::beginReading()
{
	using boost::asio::ip::udp;

	// Allocate a new vector here to contain data
	std::shared_ptr<std::vector<boost::uint8_t> > buffer(
				new std::vector<boost::uint8_t>(30));
	std::shared_ptr<udp::endpoint> endpoint(new udp::endpoint);

	m_socket.async_receive_from(boost::asio::buffer(*buffer), *endpoint,
								boost::bind(&UdpServer::handleReceive, this,
											endpoint, buffer,
											boost::asio::placeholders::error,
											boost::asio::placeholders::bytes_transferred));
}

void UdpServer::handleReceive(const std::shared_ptr<boost::asio::ip::udp::endpoint> &endpoint,
				   const std::shared_ptr<std::vector<boost::uint8_t> > &buffer,
				   const boost::system::error_code& errorCode,
				   std::size_t bytesReceived)
{
	if (!errorCode)
	{
		std::cout << bytesReceived << " bytes received..." << std::endl;
		m_ioService->post(std::bind(m_callback, buffer));
	}
	else
	{
		std::cerr << "Got an error: " << errorCode << std::endl;
	}
	beginReading();
}
