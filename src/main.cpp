#include <iostream>

#include <boost/program_options.hpp>
#include <boost/asio/signal_set.hpp>

#include <tins/ip.h>
#include <tins/udp.h>

#include "tunnel.h"
#include "version.h"
#include "virtual_interface.h"

void parseParameters(
      int argc, char *argv[],
      boost::program_options::options_description &availableParameters,
      boost::program_options::variables_map &parameters)
{
	availableParameters.add_options()
	      ("help,h", "Print help message")
	      ("version,v", "Print version number");

	boost::program_options::store(
	         boost::program_options::parse_command_line(argc, argv,
	                                                    availableParameters),
	         parameters);

	boost::program_options::notify(parameters);
}

void callback(const Overpass::SharedBuffer &buffer)
{
	Tins::IP ipPacket(buffer->data(), buffer->size());
	const Tins::UDP &udpPacket = ipPacket.rfind_pdu<Tins::UDP>();
	std::cout << "[UDP PACKET]" << std::endl;
	std::cout << "Source port:\t" << udpPacket.sport() << std::endl;
	std::cout << "Destination port:\t" << udpPacket.dport() << std::endl;
	std::cout << "Packet length:\t" << udpPacket.length() << std::endl;
	std::cout << "Checksum:\t" << udpPacket.checksum() << std::endl;
}

int main(int argc, char *argv[])
{
	boost::program_options::options_description availableParameters("Parameters");
	boost::program_options::variables_map parameters;

	try
	{
		parseParameters(argc, argv, availableParameters, parameters);
	}
	catch(const boost::program_options::error &exception)
	{
		std::cerr << "Unable to parse parameters: " << exception.what() << std::endl;
		return 1;
	}

	if (parameters.count("help"))
	{
		std::cout << availableParameters << std::endl;
		return 0;
	}

	if (parameters.count("version"))
	{
		std::cout << "Overpass v" << Overpass::version() << std::endl;
		return 0;
	}

	std::string interfaceName = "ovp%d";
	int interfaceFileDescriptor;
	if (!Overpass::createVirtualInterface(interfaceName, interfaceFileDescriptor))
	{
		std::cerr << "Unable to create virtual interface." << std::endl;
		return 1;
	}

	if (!Overpass::assignDeviceAddress(interfaceName, "11.11.11.1", "255.255.255.0"))
	{
		std::cerr << "Unable to assign address." << std::endl;
		return 1;
	}

	std::shared_ptr<boost::asio::io_service> ioService(
	         new boost::asio::io_service);
	Overpass::Tunnel tunnel(ioService, callback, interfaceFileDescriptor);

	// Construct a signal set registered for process termination.
	boost::asio::signal_set signal_set(*ioService, SIGINT, SIGTERM);

	// Start an asynchronous wait for one of the signals to occur.
	using boost::system::error_code;
	signal_set.async_wait(
	         [&ioService](const error_code& error, int signalNumber)
	{
		if (error)
		{
			std::cerr << "Got error: " << error << std::endl;
		}

		std::cout << "Caught signal: requesting stop" << std::endl;

		ioService->stop();
	});

	// Make sure we have at least two threads
	auto numberOfCores = std::max(static_cast<unsigned int>(2),
	                              std::thread::hardware_concurrency());

	std::cout << "Firing up " << numberOfCores << " threads..." << std::endl;

	std::vector<std::thread> threadPool;
	for (unsigned int i = 0; i < numberOfCores; ++i)
	{
		threadPool.push_back(std::thread([ioService](){ioService->run();}));
	}

	ioService->run();

	std::cout << "Stopping..." << std::endl;

	ioService->stop();

	std::for_each(threadPool.begin(), threadPool.end(),
	              [](std::thread &thread)
	{
		thread.join();
	});

	close(interfaceFileDescriptor);

	return 0;
}
