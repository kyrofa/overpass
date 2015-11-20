#include <gtest/gtest.h>

#include <boost/asio/detail/socket_ops.hpp>

#include "include/udp_packet.h"

// Verify that parsing a UDP packet from a buffer works as expected.
TEST(UdpPacket, Parse)
{
	uint16_t sourcePort = 1; // Bits 0-15
	uint16_t destinationPort = 2; // Bits 16-31
	uint16_t packetLength = 8; // Bits 32-47
	uint16_t checksum = 3; // Bits 48-63

	using boost::asio::detail::socket_ops::host_to_network_short;
	auto networkSourcePort = host_to_network_short(sourcePort);
	auto networkdestinationPort = host_to_network_short(destinationPort);
	auto networkpacketLength = host_to_network_short(packetLength);
	auto networkSchecksum = host_to_network_short(checksum);

	std::vector<uint8_t> buffer = {
		static_cast<uint8_t>(networkSourcePort >> 8),
		static_cast<uint8_t>(networkSourcePort & 0x0f),

		static_cast<uint8_t>(networkdestinationPort >> 8),
		static_cast<uint8_t>(networkdestinationPort & 0x0f),

		static_cast<uint8_t>(networkpacketLength >> 8),
		static_cast<uint8_t>(networkpacketLength & 0x0f),

		static_cast<uint8_t>(networkSchecksum >> 8),
		static_cast<uint8_t>(networkSchecksum & 0x0f),
	};

	UdpPacket packet(buffer);

	ASSERT_EQ(sourcePort, packet.sourcePort());
	ASSERT_EQ(destinationPort, packet.destinationPort());
	ASSERT_EQ(packetLength, packet.packetLength());
	ASSERT_EQ(checksum, packet.checksum());
}

// Verify that an exception is thrown if the buffer is too short to hold the
// UDP header.
TEST(UdpPacket, ParseTooShortForHeader)
{
	std::vector<uint8_t> buffer = {1}; // Size of 1, should be 8 for the header

	ASSERT_THROW(UdpPacket packet(buffer), std::length_error);
}

// Verify that an exception is thrown if the packet size is shorter than the
// buffer.
TEST(UdpPacket, ParseStatedLengthTooShort)
{
	uint16_t sourcePort = 1; // Bits 0-15
	uint16_t destinationPort = 2; // Bits 16-31
	uint16_t packetLength = 7; // Bits 32-47
	uint16_t checksum = 3; // Bits 48-63

	using boost::asio::detail::socket_ops::host_to_network_short;
	auto networkSourcePort = host_to_network_short(sourcePort);
	auto networkdestinationPort = host_to_network_short(destinationPort);
	auto networkpacketLength = host_to_network_short(packetLength);
	auto networkSchecksum = host_to_network_short(checksum);

	std::vector<uint8_t> buffer = {
		static_cast<uint8_t>(networkSourcePort >> 8),
		static_cast<uint8_t>(networkSourcePort & 0x0f),

		static_cast<uint8_t>(networkdestinationPort >> 8),
		static_cast<uint8_t>(networkdestinationPort & 0x0f),

		static_cast<uint8_t>(networkpacketLength >> 8),
		static_cast<uint8_t>(networkpacketLength & 0x0f),

		static_cast<uint8_t>(networkSchecksum >> 8),
		static_cast<uint8_t>(networkSchecksum & 0x0f),
	};

	ASSERT_THROW(UdpPacket packet(buffer), std::length_error);
}

// Verify that an exception is thrown if the packet size is longer than the
// buffer.
TEST(UdpPacket, ParseStatedLengthTooLong)
{
	uint16_t sourcePort = 1; // Bits 0-15
	uint16_t destinationPort = 2; // Bits 16-31
	uint16_t packetLength = 9; // Bits 32-47
	uint16_t checksum = 3; // Bits 48-63

	using boost::asio::detail::socket_ops::host_to_network_short;
	auto networkSourcePort = host_to_network_short(sourcePort);
	auto networkdestinationPort = host_to_network_short(destinationPort);
	auto networkpacketLength = host_to_network_short(packetLength);
	auto networkSchecksum = host_to_network_short(checksum);

	std::vector<uint8_t> buffer = {
		static_cast<uint8_t>(networkSourcePort >> 8),
		static_cast<uint8_t>(networkSourcePort & 0x0f),

		static_cast<uint8_t>(networkdestinationPort >> 8),
		static_cast<uint8_t>(networkdestinationPort & 0x0f),

		static_cast<uint8_t>(networkpacketLength >> 8),
		static_cast<uint8_t>(networkpacketLength & 0x0f),

		static_cast<uint8_t>(networkSchecksum >> 8),
		static_cast<uint8_t>(networkSchecksum & 0x0f),
	};

	ASSERT_THROW(UdpPacket packet(buffer), std::length_error);
}
