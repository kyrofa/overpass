#include <boost/algorithm/string.hpp>

#include <upnp/upnptools.h>

#include "internal/upnp_objects/xml_parsing.h"

namespace
{
	bool upnpStringCompare(const std::string &upnpString,
	                       const std::string &other)
	{
		std::string upnpStringLower = boost::to_lower_copy(upnpString);
		std::string otherLower = boost::to_lower_copy(other);

		if (upnpStringLower == otherLower)
		{
			return true;
		}

		// If they're not directly equal, it's possible for the UPnP string to
		// actually take the form "u:ActualString". Rip off the "u:" and try the
		// comparison again.
		size_t indexOfColon = upnpStringLower.find(':');
		if ((indexOfColon != std::string::npos) &&
		    (upnpStringLower.substr(indexOfColon+1) == otherLower))
		{
			return true;
		}

		return false;
	}
}

using namespace Overpass::internal;

Overpass::internal::UpnpException::UpnpException(const std::string &what, int errorCode) :
   Exception(what + ": " + UpnpGetErrorMessage(errorCode))
{
}

std::string Overpass::internal::getNodeText(IXML_Node *node)
{
	if (!node)
	{
		return "";
	}

	IXML_Node *textNode = ixmlNode_getFirstChild(node);
	if (!textNode)
	{
		return "";
	}

	return ixmlNode_getNodeValue(textNode);
}

void Overpass::internal::getChildrenWithName(IXML_Node *rootNode,
                                             const std::string &name,
                                             std::vector<IXML_Node*> &children)
{
	IXML_Node *child = ixmlNode_getFirstChild(rootNode);
	while (child)
	{
		if (ixmlNode_getNodeType(child) == eELEMENT_NODE)
		{
			std::string childName(ixmlNode_getNodeName(child));
			if (upnpStringCompare(childName, name))
			{
				children.push_back(child);
			}
		}

		child = ixmlNode_getNextSibling(child);
	}
}

IXML_Node *Overpass::internal::getChildWithName(IXML_Node *rootNode,
                                                const std::string &name)
{
	std::vector<IXML_Node*> matchingChildren;
	getChildrenWithName(rootNode, name, matchingChildren);
	if (matchingChildren.empty())
	{
		return nullptr;
	}

	return matchingChildren.front();
}

IXML_Node *Overpass::internal::searchForDevice(IXML_Node *rootNode,
                                               const std::string &deviceType)
{
	IXML_Node *child = ixmlNode_getFirstChild(rootNode);
	while (child)
	{
		if (ixmlNode_getNodeType(child) == eELEMENT_NODE)
		{
			std::string childName(ixmlNode_getNodeName(child));
			boost::algorithm::to_lower(childName);
			if (childName == "device")
			{
				// Check to see if this device is the device type for which we're
				// searching.
				IXML_Node *deviceTypeNode = getChildWithName(child, "devicetype");
				if (upnpStringCompare(getNodeText(deviceTypeNode), deviceType))
				{
					return child;
				}
			}
		}

		IXML_Node *deviceNode = searchForDevice(child, deviceType);
		if (deviceNode)
		{
			return deviceNode;
		}

		child = ixmlNode_getNextSibling(child);
	}

	return nullptr;
}
