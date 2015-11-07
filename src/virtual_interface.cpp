#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <cstring>
#include <string>

namespace
{
	const char *CLONE_DEVICE = "/dev/net/tun";
}

std::string createVirtualInterface(const std::string &requestedDeviceName)
{
	int fd = open(CLONE_DEVICE, O_RDWR);
	if (fd < 0)
	{
		perror("Failed to open clone device");
		return "";
	}

	ifreq request;
	memset(&request, 0, sizeof(request));

	request.ifr_flags = IFF_TAP;
	requestedDeviceName.copy(request.ifr_name, IFNAMSIZ);

	if (ioctl(fd, TUNSETIFF, &request) < 0)
	{
		perror("Failed to clone the device");
		return "";
	}

	return std::string(request.ifr_name);
}

bool assignDeviceAddress(const std::string &deviceName,
						 const std::string &ipAddress,
						 const std::string &netmask)
{
	ifreq request;
	memset(&request, 0, sizeof(request));

	deviceName.copy(request.ifr_name, IFNAMSIZ);
	request.ifr_addr.sa_family = AF_INET;

	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	sockaddr_in* socketAddress = reinterpret_cast<sockaddr_in*>(&request.ifr_addr);
	inet_pton(AF_INET, ipAddress.c_str(), &socketAddress->sin_addr);
	if (ioctl(sockfd, SIOCSIFADDR, &request) < 0)
	{
		perror("Unable to set IP address");
		return false;
	}

	inet_pton(AF_INET, netmask.c_str(), &socketAddress->sin_addr);
	if (ioctl(sockfd, SIOCSIFNETMASK, &request) < 0)
	{
		perror("Unable to set netmask");
		return false;
	}

	if (ioctl(sockfd, SIOCGIFFLAGS, &request) < 0)
	{
		perror("Unable to obtain interface flags");
		return false;
	}

	request.ifr_flags |= (IFF_UP | IFF_RUNNING);

	if (ioctl(sockfd, SIOCSIFFLAGS, &request) < 0)
	{
		perror("Unable to set interface up");
		return false;
	}

	return true;
}
