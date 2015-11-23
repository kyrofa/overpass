#ifndef DEVICE_COMMUNICATOR_DEVICE_COMMUNICATOR_INTERFACE_H
#define DEVICE_COMMUNICATOR_DEVICE_COMMUNICATOR_INTERFACE_H

#include <memory>

class DeviceCommunicatorInterface
{
	public:
		virtual int read(uint8_t *buffer, std::size_t bufferSize,
						 int timeoutSeconds, long timeoutMicroseconds) = 0;
		virtual int write(uint8_t *buffer, std::size_t bufferSize) = 0;
};

typedef std::shared_ptr<DeviceCommunicatorInterface> DeviceCommunicatorInterfacePtr;

#endif // DEVICE_COMMUNICATOR_DEVICE_COMMUNICATOR_INTERFACE_H
