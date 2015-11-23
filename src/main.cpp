#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "types.h"
#include "version.h"
#include "virtual_interface.h"
#include "tunnel.h"

#include "ip_packet.h"
#include "udp_packet.h"

#include "device_communicator/linux_device_communicator.h"

void callback(Overpass::SharedBuffer buffer)
{
	IpPacket packet(*buffer);

	std::string packetString;
	packet.toString(packetString);
	std::cout << packetString << std::endl;

	UdpPacket udpPacket(packet.payload());

	udpPacket.toString(packetString);
	std::cout << packetString << std::endl;
}

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
		std::cout << "Overpass v"
				  << LIBOVERPASS_VERSION_MAJOR << "."
				  << LIBOVERPASS_VERSION_MINOR << "."
				  << LIBOVERPASS_VERSION_PATCH << std::endl;
		return 0;
	}

	std::string interfaceName = "ovp%d";
	int interfaceFileDescriptor;
	if (!createVirtualInterface(interfaceName, interfaceFileDescriptor))
	{
		std::cerr << "Unable to create virtual interface." << std::endl;
		return 1;
	}

	if (!assignDeviceAddress(interfaceName, "11.11.11.1", "255.255.255.0"))
	{
		std::cerr << "Unable to assign address." << std::endl;
		return 1;
	}

	std::vector<std::thread> threadPool;
	std::shared_ptr<boost::asio::io_service> ioService(new boost::asio::io_service);

	std::shared_ptr<LinuxDeviceCommunicator> communicator(
				new LinuxDeviceCommunicator(interfaceFileDescriptor));

	Tunnel tunnel(ioService, callback, communicator);

	// Make sure we have at least two threads
	auto numberOfCores = std::max(static_cast<unsigned int>(2),
								  std::thread::hardware_concurrency());

	std::cout << "Firing up " << numberOfCores << " threads..." << std::endl;

	for (unsigned int i = 0; i < numberOfCores; ++i)
	{
		threadPool.push_back(std::thread([ioService](){ioService->run();}));
	}

	int val;
	std::cin >> val;

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
