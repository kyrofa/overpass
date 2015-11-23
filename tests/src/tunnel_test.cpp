#include <mutex>
#include <condition_variable>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "device_communicator/device_communicator_interface.h"
#include "tunnel.h"

namespace
{
	int fakeRead(uint8_t *buffer, std::size_t bufferSize,
				 ::testing::Unused, ::testing::Unused)
	{
		if (bufferSize > 0)
		{
			buffer[0] = 1;
		}

		return 1; // A single byte
	}

	class MockDeviceCommunicator : public DeviceCommunicatorInterface
	{
		public:
			MOCK_METHOD4(read, int(uint8_t *buffer, std::size_t bufferSize,
								   int timeoutSeconds, long timeoutMicroseconds));
			MOCK_METHOD2(write, int(uint8_t *buffer, std::size_t bufferSize));
	};

	class MockCallbackProvider
	{
		public:
			MOCK_METHOD1(callback, void(Overpass::SharedBuffer received));
	};
}

TEST(Tunnel, Read)
{
	std::shared_ptr<boost::asio::io_service> ioService(new boost::asio::io_service);

	std::shared_ptr<MockDeviceCommunicator> communicator(new MockDeviceCommunicator);

	MockCallbackProvider mockCallbackProvider;

	Tunnel tunnel(ioService, std::bind(&MockCallbackProvider::callback,
									   &mockCallbackProvider,
									   std::placeholders::_1),
				  communicator);

	std::mutex mutex;
	std::condition_variable condition;

	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ContainerEq;
	using ::testing::Pointee;

	EXPECT_CALL(*communicator, read(_, _, _, _))
			.Times(AtLeast(1))
			.WillRepeatedly(::testing::Invoke(fakeRead));

	EXPECT_CALL(mockCallbackProvider, callback(Pointee(ContainerEq(Overpass::Buffer{1}))))
			.Times(AtLeast(1))
			.WillRepeatedly(::testing::InvokeWithoutArgs([&]
	{
		std::lock_guard<std::mutex> lock(mutex);
		condition.notify_one();
	}));

	// Run tunnel in thread
	std::thread thread([ioService](){ioService->run();});

	// Wait for test to complete
	{
		std::unique_lock<std::mutex> lock(mutex);
		EXPECT_EQ(condition.wait_for(lock, std::chrono::seconds(1)),
				  std::cv_status::no_timeout);
	}

	ioService->stop();
	thread.join();
}
