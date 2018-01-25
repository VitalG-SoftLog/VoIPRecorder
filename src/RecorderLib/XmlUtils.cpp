#include "StdAfx.h"
#include "XmlUtils.h"
#include "StringUtils.h"

bool XmlUtils::SaveValStr(const std::string &name, 
						  const std::string &value, 
						  TiXmlElement *pElem)
{
	bool bReturn = false;

	if (pElem)
	{
		TiXmlElement *pChild = new TiXmlElement("entry");
		pChild->SetAttribute("name", name.c_str());
		pChild->LinkEndChild(new TiXmlText(value.c_str()));
		pElem->LinkEndChild(pChild);
		bReturn = true;
	}

	return bReturn;
}

bool XmlUtils::LoadValStr(const std::string &name, 
						  std::string &value, 
						  TiXmlElement *pElem)
{
	bool bReturn = false;
	if (pElem)
	{
		TiXmlElement *pChild = pElem->FirstChildElement("entry");
		for(;pChild;pChild=pChild->NextSiblingElement("entry"))
		{
			const char *pchar = pChild->Attribute("name");
			if(pchar)
			{
				if (name == pchar)
				{
					const char *pData = pChild->GetText();
					if (pData)
						value = pData;
					bReturn = true;
					break;
				}
			}
		}
	}
	return bReturn;
}

bool XmlUtils::SaveValInt(const std::string &name, int value, TiXmlElement *pElem)
{
	return XmlUtils::SaveValStr(name, StringUtils::IntToString(value), pElem);
}

bool XmlUtils::LoadValInt(const std::string &name, int &value, TiXmlElement *pElem)
{
	std::string data;
	if(LoadValStr(name, data, pElem))
	{
		value = StringUtils::StringToInt(data);
		return true;
	}
	return false;
}

bool XmlUtils::LoadValInt(const std::string &name, unsigned short &value, TiXmlElement *pElem)
{
	int data = value;
	if (XmlUtils::LoadValInt(name, data, pElem))
	{
		value = data;
		return true;
	}
	return false;
}



