#include <unistd.h>
#include "device_communicator/linux_device_communicator.h"

LinuxDeviceCommunicator::LinuxDeviceCommunicator(int deviceFileDescriptor) :
	m_fileDescriptor(deviceFileDescriptor)
{
}

int LinuxDeviceCommunicator::read(uint8_t *buffer, std::size_t bufferSize,
									 int timeoutSeconds, long timeoutMicroseconds)
{
	m_timeout.tv_sec = timeoutSeconds;
	m_timeout.tv_usec = timeoutMicroseconds;

	FD_ZERO(&m_fileDescriptorSet);
	FD_SET(m_fileDescriptor, &m_fileDescriptorSet);

	if (select(m_fileDescriptor+1, &m_fileDescriptorSet, NULL, NULL,
			   &m_timeout) < 0)
	{
		return -1;
	}

	if (FD_ISSET(m_fileDescriptor, &m_fileDescriptorSet))
	{
		return ::read(m_fileDescriptor, buffer, bufferSize);
	}

	return 0; // Timeout-- nothing read.
}

int LinuxDeviceCommunicator::write(uint8_t *buffer, std::size_t bufferSize)
{
	return ::write(m_fileDescriptor, buffer, bufferSize);
}
