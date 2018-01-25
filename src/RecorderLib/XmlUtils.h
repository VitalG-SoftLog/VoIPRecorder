// Header: XmlUtils.h
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <tinyxml.h>

//////////////////////////////////////////////////////////////////////////
// Namespace: XmlUtils
// Wrappers around tinyxml library.
//////////////////////////////////////////////////////////////////////////
namespace XmlUtils
{
	// Function: SaveValStr
	// 
	bool SaveValStr(const std::string &name, const std::string &value, TiXmlElement *pElem);
	bool LoadValStr(const std::string &name, std::string &value, TiXmlElement *pElem);

	bool SaveValInt(const std::string &name, int value, TiXmlElement *pElem);
	bool LoadValInt(const std::string &name, int &value, TiXmlElement *pElem);
	bool LoadValInt(const std::string &name, unsigned short &value, TiXmlElement *pElem);

};
