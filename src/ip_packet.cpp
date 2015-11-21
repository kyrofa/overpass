#include <stdexcept>
#include <sstream>

#include <boost/asio/detail/socket_ops.hpp>

#include "include/ip_packet.h"

namespace
{
	const int IP_HEADER_SIZE = 20;
}

IpPacket::IpPacket(uint16_t sourcePort, uint16_t destinationPort,
				   const std::vector<uint8_t> &payload)
{

}

IpPacket::IpPacket(const std::vector<uint8_t> &buffer)
{
	auto bufferSize = buffer.size();
	if (bufferSize < IP_HEADER_SIZE)
	{
		std::stringstream stream;
		stream << "IP header is " << IP_HEADER_SIZE << " bytes, but buffer "
			   << "is only "  << bufferSize << " bytes";
		throw std::length_error(stream.str());
	}

	// Parse the buffer into an IP packet.

	using boost::asio::detail::socket_ops::network_to_host_short;
	using boost::asio::detail::socket_ops::network_to_host_long;

	// Parse version, bits 0-3
	m_version = buffer[0] >> 4;

	// Parse internet header length, bits 4-7
	m_internetHeaderLength = buffer[0] & 0x0f;

	// Parse differentiated services code point, bits 8-13
	m_differentiatedServicesCodePoint = network_to_host_short(buffer[1] >> 2);

	// Parse explicit congestion notification, bits 14-15
	m_explicitCongestionNotification = buffer[1] & 0x03;

	// Parse packet length, bits 16-31
	m_packetLength = network_to_host_short((buffer[2] << 8) | buffer[3]);
	if (bufferSize != m_packetLength)
	{
		std::stringstream stream;
		stream << "Buffer is " << bufferSize << " bytes, expected "
			   << m_packetLength;
		throw std::length_error(stream.str());
	}

	// Parse identification, bits 32-47
	m_identification = network_to_host_short((buffer[4] << 8) | buffer[5]);

	// Parse flags, bits 48-50
	m_flags = buffer[6] >> 13;

	// Parse fragment offset, bits 51-63
	m_fragmentOffset = network_to_host_short(((buffer[6] & 0x1f) << 8) | buffer[7]);

	// Parse time to live, bits 64-71
	m_timeToLive = buffer[8];

	// Parse protocol, bits 72-79
	m_protocol = buffer[9];

	// Parse header checksum, bits 80-95
	m_headerChecksum = network_to_host_short((buffer[10] << 8) | buffer[11]);

	// Parse source IP address, bits 96-127
	m_sourceIpAddress = network_to_host_long((buffer[12] << 24) |
											 (buffer[13] << 16) |
											 (buffer[14] << 8) |
											 (buffer[15] & 0x000000ff));

	// Parse destination IP address, bits 128-159
	m_destinationIpAddress = network_to_host_long((buffer[16] << 24) |
							 (buffer[17] << 16) |
							 (buffer[18] << 8) |
							 (buffer[19] & 0x000000ff));

	// Parse any options, starting at byte 160. Calculate the number of bytes in
	// the options. This is calculated using the internet header length, which
	// is given in terms of 32-bit words. If it's greater than 5 (the minimum
	// size in words of the header with no options), then we have options we
	// need to parse.
	int optionsSize = (m_internetHeaderLength-5)*32/8;
	for (int i = 0; i < optionsSize; i+=4)
	{
		m_options.push_back((buffer[20+i] << 24) |
							(buffer[20+i+1] << 16) |
							(buffer[20+i+2] << 8) |
							(buffer[20+i+3] & 0x000000ff));
	}

	// Now calculate the byte offset of the payload so we can extract it.
	int payloadOffset = m_internetHeaderLength*32/8;

	// Parse payload, the rest of the packet.
	m_payload.assign(buffer.cbegin()+payloadOffset, buffer.cend());
}

void IpPacket::toBuffer(std::vector<uint8_t> &buffer) const
{

}

void IpPacket::toString(std::string &packetString) const
{
	std::stringstream stream;
	stream << "[IP PACKET]" << std::endl;
	stream << "Version:\t" << version() << std::endl;
	stream << "IHL:\t" << internetHeaderLength() << std::endl;
	stream << "DSCP:\t" << differentiatedServicesCodePoint() << std::endl;
	stream << "ECN:\t" << explicitCongestionNotification() << std::endl;
	stream << "Length:\t" << packetLength() << std::endl;
	stream << "ID:\t" << identification() << std::endl;
	stream << "Flags:\t" << flags() << std::endl;
	stream << "Fragment offset:\t" << fragmentOffset() << std::endl;
	stream << "TTL:\t" << timeToLive() << std::endl;
	stream << "Protocol:\t" << protocol() << std::endl;
	stream << "Checksum:\t" << headerChecksum() << std::endl;
	stream << "Source IP:\t" << sourceIpAddress() << std::endl;
	stream << "Destination IP:\t" << destinationIpAddress() << std::endl;

	packetString = stream.str();
}

uint8_t IpPacket::version() const
{
	return m_version;
}

uint8_t IpPacket::internetHeaderLength() const
{
	return m_internetHeaderLength;
}

uint8_t IpPacket::differentiatedServicesCodePoint() const
{
	return m_differentiatedServicesCodePoint;
}

uint8_t IpPacket::explicitCongestionNotification() const
{
	return m_explicitCongestionNotification;
}

uint16_t IpPacket::packetLength() const
{
	return m_packetLength;
}

uint16_t IpPacket::identification() const
{
	return m_identification;
}

uint8_t IpPacket::flags() const
{
	return m_flags;
}

uint16_t IpPacket::fragmentOffset() const
{
	return m_fragmentOffset;
}

uint8_t IpPacket::timeToLive() const
{
	return m_timeToLive;
}

uint8_t IpPacket::protocol() const
{
	return m_protocol;
}

uint16_t IpPacket::headerChecksum() const
{
	return m_headerChecksum;
}

uint32_t IpPacket::sourceIpAddress() const
{
	return m_sourceIpAddress;
}

uint32_t IpPacket::destinationIpAddress() const
{
	return m_destinationIpAddress;
}

const std::vector<uint32_t>& IpPacket::options() const
{
	return m_options;
}

const std::vector<uint8_t>& IpPacket::payload() const
{
	return m_payload;
}

