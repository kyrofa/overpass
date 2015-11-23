#include <algorithm>
#include <iostream>
#include <bitset>

#include "tunnel.h"

namespace
{
	const int BUFFER_SIZE = 1500; // 1500 bytes, standard MTU
}

class TunnelPrivate
{
	public:
		TunnelPrivate(const std::shared_ptr<boost::asio::io_service> &ioService,
					  Tunnel::ReadCallback callback, const DeviceCommunicatorInterfacePtr &communicator);

		void sendAsynchronously(Overpass::SharedBuffer buffer);

	private:
		void readFromTunnel();
		void sendToTunnel(Overpass::SharedBuffer buffer);

	private:
		std::shared_ptr<boost::asio::io_service> m_ioService;
		Tunnel::ReadCallback m_callback;
		DeviceCommunicatorInterfacePtr m_communicator;
};

TunnelPrivate::TunnelPrivate(
		const std::shared_ptr<boost::asio::io_service> &ioService,
		Tunnel::ReadCallback callback, const DeviceCommunicatorInterfacePtr &communicator) :
	m_ioService(ioService),
	m_callback(callback),
	m_communicator(communicator)
{
	// Start reading from file descriptor as soon as the IO service is running.
	m_ioService->post(std::bind(&TunnelPrivate::readFromTunnel, this));
}

void TunnelPrivate::sendAsynchronously(Overpass::SharedBuffer buffer)
{
	m_ioService->post(std::bind(&TunnelPrivate::sendToTunnel, this, buffer));
}

void TunnelPrivate::readFromTunnel()
{
	uint8_t buffer[BUFFER_SIZE];

	// Read with a one-second timeout
	auto bytesRead = m_communicator->read(buffer, BUFFER_SIZE, 1, 0);

	if (bytesRead > 0)
	{
		std::shared_ptr<std::vector<uint8_t> > sharedBuffer(new std::vector<uint8_t>(buffer, buffer+bytesRead));

		m_ioService->post(std::bind(m_callback, sharedBuffer));
	}
	else if (bytesRead < 0)
	{
		perror("Unable to read from tunnel");
	}

	//	for (int i = 0; i < bytesRead; i+=5)
	//	{
	//		for (int j = 0; j < 5; ++j)
	//		{
	//			std::cout << static_cast<std::bitset<8> >(buffer[i+j]) << "(" << std::hex << (int) buffer[i+j] << ") ";
	//		}
	//		std::cout << std::endl;
	//	}

	m_ioService->post(std::bind(&TunnelPrivate::readFromTunnel, this));
}

void TunnelPrivate::sendToTunnel(Overpass::SharedBuffer buffer)
{
	if (m_communicator->write(buffer->data(), buffer->size()) < 0)
	{
		perror("Unable to send to tunnel");
	}
}

Tunnel::Tunnel(std::shared_ptr<boost::asio::io_service> ioService,
			   ReadCallback callback, const DeviceCommunicatorInterfacePtr &communicator) :
	m_data(new TunnelPrivate(ioService, callback, communicator))
{
}

Tunnel::~Tunnel()
{
}

void Tunnel::sendToTunnel(Overpass::SharedBuffer buffer)
{
	m_data->sendAsynchronously(buffer);
}
