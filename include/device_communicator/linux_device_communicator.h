#ifndef DEVICE_COMMUNICATIONS_LINUX_H
#define DEVICE_COMMUNICATIONS_LINUX_H

#include <cstdint>
#include <ctime>

#include <sys/select.h>

#include "device_communicator/device_communicator_interface.h"

class LinuxDeviceCommunicator : public DeviceCommunicatorInterface
{
	public:
		explicit LinuxDeviceCommunicator(int deviceFileDescriptor);

		int read(uint8_t *buffer, std::size_t bufferSize,
						 int timeoutSeconds, long timeoutMicroseconds) override;
		int write(uint8_t *buffer, std::size_t bufferSize) override;

	private:
		int m_fileDescriptor;
		timeval m_timeout;
		fd_set m_fileDescriptorSet;
};

#endif // DEVICE_COMMUNICATIONS_LINUX_H
