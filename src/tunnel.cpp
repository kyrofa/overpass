#include <algorithm>
#include <iostream>

#include "include/tunnel.h"

namespace
{
	const int BUFFER_SIZE = 1500; // 1500 bytes, standard MTU
}

Tunnel::Tunnel(int interfaceFileDescriptor) :
	m_fileDescriptor(interfaceFileDescriptor),
	m_ioService(new boost::asio::io_service)
{
}

void Tunnel::start()
{
	stop();

	m_ioService.reset(new boost::asio::io_service);

	// Ask IO service to begin reading (won't actually start until the thread
	// pool is fired up, though)
	m_ioService->post(std::bind(&Tunnel::readFromTunnel, this));

	// Make sure we have at least two threads
	auto numberOfCores = std::max(static_cast<unsigned int>(2),
								  std::thread::hardware_concurrency());

	std::cout << "Firing up " << numberOfCores << " threads..." << std::endl;

	for (unsigned int i = 0; i < numberOfCores; ++i)
	{
		m_threadPool.push_back(std::thread([this](){this->m_ioService->run();}));
	}
}

void Tunnel::stop()
{
	m_ioService->stop();

	std::for_each(m_threadPool.begin(), m_threadPool.end(),
				  [](std::thread &thread)
	{
		thread.join();
	});

	m_threadPool.clear();
}

void Tunnel::readFromTunnel()
{
	char buffer[BUFFER_SIZE];

	std::cout << "Reading..." << std::endl;

	std::size_t bytesRead = read(m_fileDescriptor, buffer, BUFFER_SIZE);

	std::cout << "Just read " << bytesRead << " bytes..." << std::endl;

	m_ioService->post(std::bind(&Tunnel::readFromTunnel, this));
}
