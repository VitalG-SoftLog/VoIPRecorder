#pragma once

#include <tinyxml.h>
#include <string>

namespace XmlUtils
{
	bool SaveValStr(const std::string &name, const std::string &value, TiXmlElement *pElem);
	bool LoadValStr(const std::string &name, std::string &value, TiXmlElement *pElem);

	bool SaveValInt(const std::string &name, int value, TiXmlElement *pElem);
	bool LoadValInt(const std::string &name, int &value, TiXmlElement *pElem);
	bool LoadValInt(const std::string &name, unsigned short &value, TiXmlElement *pElem);

};
