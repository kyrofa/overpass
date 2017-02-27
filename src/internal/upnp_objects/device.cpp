#include "internal/upnp_objects/xml_parsing.h"
#include "internal/upnp_objects/service.h"

#include "internal/upnp_objects/device.h"

using namespace Overpass::internal;

InvalidUpnpDeviceException::InvalidUpnpDeviceException(const std::string &what) :
   Exception("invalid UPnP device: " + what)
{
}

UpnpDevice::UpnpDevice(IXML_Node *node, const std::string &baseUrl,
                       int expiration) :
   m_friendlyName(getNodeText(getChildWithName(node, "friendlyname"))),
   m_deviceType(getNodeText(getChildWithName(node, "devicetype"))),
   m_udn(getNodeText(getChildWithName(node, "udn"))),
   m_expiration(expiration)
{
	// Check for validity
	if (m_udn.empty())
	{
		throw InvalidUpnpDeviceException("missing UDN");
	}

	IXML_Node *deviceListNode = getChildWithName(node, "devicelist");
	if (deviceListNode)
	{
		// Find all devices nested within this device
		std::vector<IXML_Node*> deviceNodes;
		getChildrenWithName(deviceListNode, "device", deviceNodes);
		for (const auto &deviceNode : deviceNodes)
		{
			m_deviceList.push_back(std::make_shared<UpnpDevice>(deviceNode,
			                                                    baseUrl,
			                                                    expiration));
		}
	}

	IXML_Node *serviceListNode = getChildWithName(node, "servicelist");
	if (serviceListNode)
	{
		// Find all services hosted by this device
		std::vector<IXML_Node*> serviceNodes;
		getChildrenWithName(serviceListNode, "service", serviceNodes);
		for (const auto &serviceNode : serviceNodes)
		{
			m_serviceList.push_back(std::make_shared<UpnpService>(serviceNode,
			                                                      baseUrl));
		}
	}
}

const std::string& UpnpDevice::friendlyName() const
{
	return m_friendlyName;
}

const std::string& UpnpDevice::deviceType() const
{
	return m_deviceType;
}

const std::string& UpnpDevice::udn() const
{
	return m_udn;
}

int UpnpDevice::expiration() const
{
	return m_expiration;
}

void UpnpDevice::setExpiration(int newExpiration)
{
	m_expiration = newExpiration;

	// Propogate that expiration time to embedded devices as well
	for (auto &device : m_deviceList)
	{
		device->setExpiration(newExpiration);
	}
}

void UpnpDevice::getServicesByType(
      const std::string &type,
      std::vector<std::shared_ptr<UpnpService>> &services) const
{
	for (const auto &service : m_serviceList)
	{
		if (service->serviceType() == type)
		{
			services.push_back(service);
		}
	}

	// Propogate this call to all embedded devices
	for (const auto &device : m_deviceList)
	{
		device->getServicesByType(type, services);
	}
}
