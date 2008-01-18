#include "stdafx.h"
#include "PropertyList.h"
#include <tinyxml.h>

using namespace std;

PropertyList::~PropertyList()
{
	BOOST_FOREACH(Property* p, _propertyList) delete p;
}

void PropertyList::addProperty(Property* p)
{
	_propertyMap[p->_name] = p;
	_propertyList.insert(_propertyList.end(), p);
}

Property* PropertyList::getPropertyPtr(const std::string &key)
{
	return _propertyMap[key];
}

const std::list<Property*>& PropertyList::getList() const
{
	return _propertyList;
}

const std::map<std::string, Property*>& PropertyList::getMap() const
{
	return _propertyMap;
}

std::string PropertyList::toString() const { return ""; }
void PropertyList::fromString(const std::string& str) {}

bool PropertyList::loadXML(const TiXmlHandle& root, const std::string &filePath)
{
	TiXmlElement* pElem=root.FirstChild().Element();
	for (; pElem; pElem=pElem->NextSiblingElement())
	{
		Property* p;
		if(find(string(pElem->Value()), p))
			p->loadXML(TiXmlHandle(pElem), filePath);
	}
	return true;
}

TiXmlElement* PropertyList::saveXML(const std::string &filePath)
{
	TiXmlElement* root = new TiXmlElement(_name.c_str());
	BOOST_FOREACH(Property* p, _propertyList)
		root->LinkEndChild(p->saveXML(filePath));
	return root;
}

bool PropertyList::find(const std::string& str, Property* &p) const 
{
	map<string, Property*>::const_iterator pIt = _propertyMap.find(str);
	if(pIt != _propertyMap.end())
	{
		p = pIt->second;
		return true;
	}
	return false;
}

bool PropertyList::findAll(const std::string& str, Property* &p) const 
{
	map<string, Property*>::const_iterator pIt = _propertyMap.find(str);
	if(pIt != _propertyMap.end())
	{
		p = pIt->second;
		return true;
	}
	BOOST_FOREACH(Property* p2, _propertyList)
		if(typeid(*p2)==typeid(PropertyList))
			if(static_cast<PropertyList*>(p2)->findAll(str, p)) return true;

	return false;
}

bool Property::loadXML(const TiXmlHandle& root, const std::string &filePath)
{
	fromString(string(root.Element()->Attribute("value")));
	return true;
}

TiXmlElement* Property::saveXML(const std::string &filePath)
{
	TiXmlElement *elem = new TiXmlElement(_name.c_str());
	elem->SetAttribute("value", toString().c_str());
	return elem;
}

bool PropertyImage::loadXML(const TiXmlHandle& root, const std::string &filePath)
{
	string val(root.Element()->Attribute("value"));
	if(val.substr(0,2) == "./")
	{
		size_t filePathLast = filePath.find_last_of('/')+1;
		string path = filePath.substr(0, filePathLast);
		fromString(path+val.substr(2));
	}
	else fromString(val);
	return true;
}

TiXmlElement* PropertyImage::saveXML(const std::string &filePath)
{
	replace(_data.begin(), _data.end(), '\\', '/');
	TiXmlElement *elem = new TiXmlElement(_name.c_str());
	// if the path of the files is the same
	size_t filePathLast = filePath.find_last_of('/')+1;
	if(toString().size() > filePathLast &&
		toString().substr(0, filePathLast) == filePath.substr(0, filePathLast))
	{
		string relativePath = "./"+toString().substr(filePathLast);
		elem->SetAttribute("value", relativePath.c_str());
	}
	else elem->SetAttribute("value", toString().c_str());
	return elem;
}
