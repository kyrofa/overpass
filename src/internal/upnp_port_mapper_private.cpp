#include <iostream>
#include <regex>

#include <boost/algorithm/string.hpp>

#include <upnp/upnptools.h>

#include "internal/upnp_objects/xml_parsing.h"
#include "internal/upnp_objects/device.h"
#include "internal/upnp_objects/service.h"

#include "internal/upnp_port_mapper_private.h"

using namespace Overpass::internal;

namespace
{
	const std::string INTERNET_GATEWAY_DEVICE_TYPE =
	      "urn:schemas-upnp-org:device:InternetGatewayDevice:1";
	const std::string WAN_DEVICE_TYPE =
	      "urn:schemas-upnp-org:device:WANDevice:1";
	const std::string WANIP_SERVICE_TYPE =
	      "urn:schemas-upnp-org:service:WANIPConnection:1";
	const std::string WANPPP_SERVICE_TYPE =
	      "urn:schemas-upnp-org:service:WANPPPConnection:1";

	const int EXPIRATION_TIMER_PERIOD_SECONDS = 10;

	extern "C" int upnpCallbackDispatcher(Upnp_EventType eventType, void *event,
	                                      void *cookie)
	{
		UpnpPortMapperPrivate *portMapper = static_cast<UpnpPortMapperPrivate*>(
		                                       cookie);
		if (!portMapper)
		{
			throw Overpass::PortMappingException(
			         "port mapper instance not found in callback");
		}

		return portMapper->upnpClientCallback(eventType, event);
	}
}

Overpass::PortMappingException::PortMappingException(const std::string &what) :
   Exception("unable to map ports using UPnP: " + what)
{
}

UpnpPortMapperPrivate::UpnpPortMapperPrivate(const SharedIoService &ioService,
                                             uint16_t port) :
   m_port(port),
   m_expirationTimer(*ioService)
{
}

UpnpPortMapperPrivate::~UpnpPortMapperPrivate()
{
	UpnpFinish();
}

void UpnpPortMapperPrivate::start()
{
	UpnpInit(NULL, 0);

	if (UpnpRegisterClient(
	       upnpCallbackDispatcher, this, &m_upnpClient) != UPNP_E_SUCCESS)
	{
		throw PortMappingException("failed to register client");
	}

	upnpSearchAsync("upnp:rootdevice", 5);

	m_expirationTimer.expires_from_now(boost::posix_time::seconds(
	                                      EXPIRATION_TIMER_PERIOD_SECONDS));
	m_expirationTimer.async_wait(std::bind(
	                                &UpnpPortMapperPrivate::checkExpirationTimes,
	                                shared_from_this(), std::placeholders::_1));
}

int UpnpPortMapperPrivate::upnpClientCallback(Upnp_EventType eventType,
                                              void *event)
{
	switch (eventType)
	{
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		case UPNP_DISCOVERY_SEARCH_RESULT:
			handleNewUpnpDevice(*static_cast<Upnp_Discovery*>(event));
			break;
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
			// Check to see if InternetGatewayDevice and remove from list.
			break;
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
			// Search didn't turn up any results
			break;
		case UPNP_EVENT_RECEIVED:
			// upnp event received. Check to see if it's from a device we care
			// about and do something with it.
			break;
		default:
			// Do nothing
			break;
	}

	return 0;
}

void UpnpPortMapperPrivate::upnpSearchAsync(const std::string &target,
                                            int timeout)
{
	UpnpSearchAsync(m_upnpClient, timeout, target.c_str(), this);
}

void UpnpPortMapperPrivate::handleNewUpnpDevice(const Upnp_Discovery &discovery)
{
	// Either a new UPnP device came online, or our search returned a
	// result. Either way, if it's a router, we need to add our port
	// mapping to it.
	if (discovery.ErrCode != UPNP_E_SUCCESS)
	{
		throw UpnpException("discovery error", discovery.ErrCode);
	}

	IXML_Document *document = nullptr;
	auto returnCode = UpnpDownloadXmlDoc(discovery.Location, &document);
	if (returnCode != UPNP_E_SUCCESS)
	{
		throw UpnpException("failed to download XML document", returnCode);
	}

	if (!document)
	{
		throw PortMappingException("XML document was unexpectedly NULL");
	}

	IXML_Node *deviceNode = searchForDevice(&document->n,
	                                        INTERNET_GATEWAY_DEVICE_TYPE);
	if (deviceNode)
	{
		std::string baseUrl = getNodeText(getChildWithName(&document->n,
		                                                   "URLBase"));
		if (baseUrl.empty())
		{
//			std::regex pattern("^(?:([^/:]+):/{2,})?([^/]+).*$");
//			std::cmatch match;

//			std::regex_match(discovery.Location, match, pattern);
//			if (match.size() < 3)
//			{
//				std::stringstream stream;
//				stream << "failed to parse location '" << discovery.Location << "'"
//				       << std::endl;
//				throw PortMappingException(stream.str());
//			}

//			std::string scheme = match[1];
//			std::string base = match[2];

//			baseUrl = scheme + "://" + base;
			baseUrl = discovery.Location;
		}

		{
			std::lock_guard<std::mutex> lock(m_devicesMutex);
			std::shared_ptr<UpnpDevice> device(new UpnpDevice(deviceNode, baseUrl,
			                                                  discovery.Expires));
			m_devices[device->udn()] = device;
		}

		updatePortMappings();
	}

	ixmlDocument_free(document);
}

void UpnpPortMapperPrivate::checkExpirationTimes(
      const boost::system::error_code &errorCode)
{
	if (errorCode)
	{
		throw PortMappingException(
		         "failed to run expiration timer: " + errorCode.message());
	}

	// Decrement expiration times for all devices
	std::lock_guard<std::mutex> lock(m_devicesMutex);
	for (auto iterator = m_devices.cbegin(); iterator != m_devices.cend();)
	{
		std::shared_ptr<UpnpDevice> device = iterator->second;

		int newExpiration = device->expiration() -
		                    EXPIRATION_TIMER_PERIOD_SECONDS;

		// If we've passed expiration, remove this device from the list
		if (newExpiration <= 0)
		{
			iterator = m_devices.erase(iterator);
		}
		else
		{
			// Otherwise, simply update the expiration of this device
			device->setExpiration(newExpiration);

			// If we're nearing expiration time, search again to renew
			if (newExpiration <= (EXPIRATION_TIMER_PERIOD_SECONDS * 2))
			{
				upnpSearchAsync(device->udn(), EXPIRATION_TIMER_PERIOD_SECONDS);
			}

			++iterator;
		}
	}

	// Continue running the timer
	m_expirationTimer.expires_at(m_expirationTimer.expires_at() +
	                             boost::posix_time::seconds(
	                                EXPIRATION_TIMER_PERIOD_SECONDS));
	m_expirationTimer.async_wait(std::bind(
	                                &UpnpPortMapperPrivate::checkExpirationTimes,
	                                shared_from_this(), std::placeholders::_1));
}

void UpnpPortMapperPrivate::updatePortMappings()
{
	std::vector<std::shared_ptr<UpnpService>> portMappingServices;

	std::lock_guard<std::mutex> lock(m_devicesMutex);
	for (const auto &pair : m_devices)
	{
		pair.second->getServicesByType(WANIP_SERVICE_TYPE, portMappingServices);
		pair.second->getServicesByType(WANPPP_SERVICE_TYPE, portMappingServices);
	}

	for (const auto &service : portMappingServices)
	{
		std::cout << "Updating port mapping for " << service->serviceType() << std::endl;
		service->enabled(m_upnpClient);
	}
}
