#include <stdexcept>
#include <sstream>

#include <boost/asio/detail/socket_ops.hpp>

#include "include/udp_packet.h"

UdpPacket::UdpPacket(const std::vector<uint8_t> &buffer)
{
	auto bufferSize = buffer.size();
	if (bufferSize < 8)
	{
		std::stringstream stream;
		stream << "UDP header is 8 bytes, but buffer is only "
			   << bufferSize << " bytes";
		throw std::length_error(stream.str());
	}

	/* Parse the buffer into a UDP packet, which is formatted like this:
	 * Bits  0-15: Source port
	 * Bits 15-31: Destination port
	 * Bits 32-47: Packet length
	 * Bits 48-63: Checksum
	 * Bits 64 on: Payload
	 */

	using boost::asio::detail::socket_ops::network_to_host_short;

	// Parse source port, bytes 0-1.
	m_sourcePort = network_to_host_short((buffer[0] << 8) | buffer[1]);

	// Parse destination port, bytes 2-3.
	m_destinationPort = network_to_host_short((buffer[2] << 8) | buffer[3]);

	// Parse packet length, bytes 4-5.
	m_packetLength = network_to_host_short((buffer[4] << 8) | buffer[5]);
	if (bufferSize != m_packetLength)
	{
		std::stringstream stream;
		stream << "Buffer is " << bufferSize << " bytes, expected "
			   << m_packetLength;
		throw std::length_error(stream.str());
	}

	// Parse checksum, bytes 6-7.
	m_checksum = network_to_host_short((buffer[6] << 8) | buffer[7]);

	// Parse payload, bytes 8 and on.
	m_payload.assign(buffer.cbegin()+8, buffer.cend());
}

uint16_t UdpPacket::sourcePort() const
{
	return m_sourcePort;
}

uint16_t UdpPacket::destinationPort() const
{
	return m_destinationPort;
}

uint16_t UdpPacket::packetLength() const
{
	return m_packetLength;
}

uint16_t UdpPacket::checksum() const
{
	return m_checksum;
}

const std::vector<uint8_t>& UdpPacket::payload() const
{
	return m_payload;
}
