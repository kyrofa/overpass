#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <memory>
#include <cstdint>

namespace Overpass
{
	typedef std::vector<uint8_t> Buffer;
	typedef std::shared_ptr<Buffer> SharedBuffer;
}

#endif // TYPES_H
