#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "stdafx.h"
#include <boost/regex.hpp>

class Property;

class Validator
{
public:
	std::string _errorMsg;
	Validator(const std::string& err) : _errorMsg(err) {}
	virtual bool isOK(const std::string& str) = 0;
};

class ValidatorNull : public Validator
{
public:
	ValidatorNull() : Validator("") {}
	virtual bool isOK(const std::string& str) {return true;}
	static ValidatorNull DEFAULT;
};

class ValidatorRE : public Validator
{
public:
	boost::regex _re; 
	ValidatorRE(const std::string& reg, std::string& err) : Validator(err), _re(reg) {}
	bool isOK(const std::string& str);
};

class ValidatorURange : public Validator
{
	uint _min, _max;
public:
	ValidatorURange(uint min, uint max, const std::string& err) : Validator(err), _min(min), _max(max) {}
	bool isOK(const std::string& str);

	static ValidatorURange UINTMAX;
	static ValidatorURange USHORTMAX;
};

class ValidatorTexture : public Validator
{
public:
	ValidatorTexture() : Validator("The texture image width and height must be a power of 2!") {}
	bool isOK(const std::string& str);
};

class ValidatorHeightmap : public Validator
{
public:
	ValidatorHeightmap() : Validator("") {}
	bool isOK(const std::string& str);
};

#endif

