#include "internal/upnp_port_mapper_private.h"
#include "upnp_port_mapper.h"

using namespace Overpass;

UpnpPortMapper::UpnpPortMapper(const SharedIoService &ioService, uint16_t port) :
   m_data(new internal::UpnpPortMapperPrivate(ioService, port))
{
	m_data->start();
}
