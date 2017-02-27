#ifndef XML_PARSING_H
#define XML_PARSING_H

#include <string>
#include <vector>

#include <upnp/ixml.h>

#include "types.h"

namespace Overpass
{
	namespace internal
	{
		class UpnpDevice;

		class UpnpException : public Exception
		{
			public:
				UpnpException(const std::string &what, int errorCode);
		};

		std::string getNodeText(IXML_Node *node);

		void getChildrenWithName(IXML_Node *rootNode, const std::string &name,
		                         std::vector<IXML_Node*> &children);

		IXML_Node *getChildWithName(IXML_Node *rootNode, const std::string &name);

		IXML_Node *searchForDevice(IXML_Node *rootNode,
		                           const std::string &deviceType);
	}
}

#endif // XML_PARSING_H
