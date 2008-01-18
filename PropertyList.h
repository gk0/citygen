#ifndef PROPERTYLIST_H
#define PROPERTYLIST_H

#include "stdafx.h"
#include <OgrePrerequisites.h>
#include <OgreStringConverter.h>
#include "Validator.h"

class TiXmlElement;
class TiXmlHandle;

class Property 
{ 
public:
	std::string _name;
	Validator& _validator;
	Property(const std::string &n) : _name(n), _validator(ValidatorNull::DEFAULT) {}
	Property(const std::string &n, Validator &v) : _name(n), _validator(v) {}
	virtual ~Property(){} 
	virtual std::string toString() const = 0; 
	virtual void fromString(const std::string& str) = 0;
	virtual bool loadXML(const TiXmlHandle& root, const std::string &filePath);
	virtual TiXmlElement* saveXML(const std::string &filePath);
	virtual bool validate(const std::string& str) { return _validator.isOK(str); }
};

class PropertyReal : public Property
{
public:
	Ogre::Real _data;
	PropertyReal(const std::string &n, Ogre::Real d) : Property(n) { _data = d; }
	PropertyReal(const std::string &n, Ogre::Real d, Validator &v) : Property(n,v) { _data = d; }
	std::string toString() const { return Ogre::StringConverter::toString(_data); }
	void fromString(const std::string& str) { _data = Ogre::StringConverter::parseReal(str); }
};

class PropertyString : public Property
{
public:
	std::string _data;
	PropertyString(const std::string &n, const std::string& d) : Property(n) { _data = d; }
	PropertyString(const std::string &n, const std::string& d, Validator &v) : Property(n,v) { _data = d; }
	std::string toString() const { return _data; }
	void fromString(const std::string& str) { _data = str; }
};

class PropertyImage : public Property
{
public:
	std::string _data;
	PropertyImage(const std::string &n, const std::string& d) : Property(n) { _data = d; }
	PropertyImage(const std::string &n, const std::string& d, Validator &v) : Property(n,v) { _data = d; }
	std::string toString() const { return _data; }
	void fromString(const std::string& str) { _data = str; }
	bool loadXML(const TiXmlHandle& root, const std::string &filePath);
	TiXmlElement* saveXML(const std::string &filePath);
};

class PropertyUShort : public Property
{
public:
	ushort _data;
	PropertyUShort(const std::string &n, ushort d) : Property(n) { _data = d; }
	PropertyUShort(const std::string &n, ushort d, Validator &v) : Property(n,v) { _data = d; }
	std::string toString() const { return Ogre::StringConverter::toString(_data); }
	void fromString(const std::string& str) { _data = Ogre::StringConverter::parseUnsignedInt(str); }
};

class PropertyUInt : public Property
{
public:
	uint _data;
	PropertyUInt(const std::string &n, uint d) : Property(n) { _data = d; }
	PropertyUInt(const std::string &n, uint d, Validator &v) : Property(n,v) { _data = d; }
	std::string toString() const { return Ogre::StringConverter::toString(_data); }
	void fromString(const std::string& str) { _data = Ogre::StringConverter::parseUnsignedInt(str); }
};

class PropertyBool : public Property
{
public:
	bool _data;
	PropertyBool(const std::string &n, bool d) : Property(n) { _data = d; }
	std::string toString() const { return Ogre::StringConverter::toString(_data); }
	void fromString(const std::string& str) { _data = Ogre::StringConverter::parseBool(str); }
};

class PropertyEnum : public Property
{
public:
	size_t _data;
	std::vector<std::string> _enumNames;

	PropertyEnum(const std::string &n, size_t d, const std::vector<std::string> &en) : Property(n), _enumNames(en) { _data = d; }
	std::string toString() const { return _enumNames[_data]; }
	void fromString(const std::string& str)
	{ 
		//TODO: some form of error handling
		_data = std::find(_enumNames.begin(), _enumNames.end(), str) - _enumNames.begin(); 
	}
};

class PropertyList : public Property
{
private:
	std::map<std::string, Property*> _propertyMap;
	std::list<Property*> _propertyList;

public:
	PropertyList() : Property("Default") {}
	PropertyList(const std::string &str) : Property(str) {}
	~PropertyList();

	void addProperty(Property* p);
	Property* getPropertyPtr(const std::string &key);
	const std::list<Property*>& getList() const;
	const std::map<std::string, Property*>& getMap() const;
	std::string toString() const;
	void fromString(const std::string& str);
	bool loadXML(const TiXmlHandle& root, const std::string &filePath);
	TiXmlElement* saveXML(const std::string &filePath);
	
	bool find(const std::string& str, Property* &p) const;
	bool findAll(const std::string& str, Property* &p) const;
};

#endif
