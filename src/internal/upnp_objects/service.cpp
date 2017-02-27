#include <boost/algorithm/string.hpp>

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "types.h"
#include "internal/upnp_objects/xml_parsing.h"

#include "internal/upnp_objects/service.h"

using namespace Overpass::internal;

UpnpServiceEnabledException::UpnpServiceEnabledException(
      const std::string &what, const UpnpService &service) :
   Exception("unable to determine if service of type '" + service.serviceType()
             + "' is enabled: " + what)
{
}

UpnpService::UpnpService(IXML_Node *node, const std::string &baseUrl) :
   m_serviceType(getNodeText(getChildWithName(node, "servicetype"))),
   m_controlUrl(baseUrl + getNodeText(getChildWithName(node, "controlurl"))),
   m_eventSubUrl(baseUrl + getNodeText(getChildWithName(node, "eventsuburl")))
{
}

const std::string& UpnpService::serviceType() const
{
	return m_serviceType;
}

const std::string& UpnpService::controlUrl() const
{
	return m_controlUrl;
}

const std::string& UpnpService::eventSubUrl() const
{
	return m_eventSubUrl;
}

bool UpnpService::enabled(int controlPoint) const
{
	const char *serviceType = m_serviceType.c_str();
	IXML_Document *action = UpnpMakeAction(
	                           "GetStatusInfo", serviceType, 0, nullptr);


	IXML_Document *response = nullptr;
	auto returnCode = UpnpSendAction(
	                     controlPoint, m_controlUrl.c_str(), serviceType, NULL,
	                     action, &response);
	if (returnCode != UPNP_E_SUCCESS)
	{
		throw UpnpException("failed to get UPnP service info", returnCode);
	}

	IXML_Node *infoResponseNode = getChildWithName(&response->n,
	                                               "GetStatusInfoResponse");
	if (!infoResponseNode)
	{
		throw UpnpServiceEnabledException("failed to get service status info",
		                                  *this);
	}

	IXML_Node *newConnectionStatusNode = getChildWithName(infoResponseNode,
	                                                      "NewConnectionStatus");
	if (!newConnectionStatusNode)
	{
		throw UpnpServiceEnabledException(
		         "failed to get service connection status", *this);
	}

	std::string connectedString = getNodeText(newConnectionStatusNode);
	boost::to_lower(connectedString);
	bool connected = (connectedString == "connected");

	ixmlDocument_free(response);
	ixmlDocument_free(action);

	return connected;
}
