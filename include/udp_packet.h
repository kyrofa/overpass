#ifndef UDP_PACKET_H
#define UDP_PACKET_H

#include <cstdint>
#include <vector>

class UdpPacket
{
	public:
		UdpPacket(const std::vector<uint8_t> &buffer);

		uint16_t sourcePort() const;
		uint16_t destinationPort() const;
		uint16_t packetLength() const;
		uint16_t checksum() const;
		const std::vector<uint8_t>& payload() const;

	private:
		uint16_t m_sourcePort; // Bits 0-15
		uint16_t m_destinationPort; // Bits 16-31
		uint16_t m_packetLength; // Bits 32-47
		uint16_t m_checksum; // Bits 48-63
		std::vector<uint8_t> m_payload; // Bits 64 on
};

#endif // UDP_PACKET_H
