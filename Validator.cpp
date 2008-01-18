#include "stdafx.h"
#include "Validator.h"
#include "PropertyList.h"
#include <wx/image.h>
#include <OgreImage.h>
#include <OgreResourceGroupManager.h>

using namespace std;

ValidatorNull ValidatorNull::DEFAULT;
ValidatorURange ValidatorURange::USHORTMAX(numeric_limits<ushort>::min(), numeric_limits<ushort>::max(),
										   "Please enter a value between 0 and "+Ogre::StringConverter::toString(numeric_limits<ushort>::max())+"!");
ValidatorURange ValidatorURange::UINTMAX(numeric_limits<uint>::min(), numeric_limits<uint>::max(),
	"Please enter a value between 0 and "+Ogre::StringConverter::toString(numeric_limits<uint>::max())+"!");

bool ValidatorURange::isOK(const std::string& str)
{
	uint test = Ogre::StringConverter::parseUnsignedInt(str);
	return _min <= test && test <= _max;
}

bool ValidatorRE::isOK(const std::string& str) 
{ 
	return boost::regex_match(str, _re); 
}



bool isPow2(int d)
{
	for(int i = 0; i < 31; i++ )
	{
		if((1 << i) == d) return true;
	}
	return false;
}

bool ValidatorHeightmap::isOK(const std::string& str)
{
	// retrieve image file info
	//wxImage img;
	//img.LoadFile(_U(str.c_str()));
	//// check width and height are x^2 + 1
	//if(isPow2(img.GetWidth()-1) && isPow2(img.GetHeight()-1)) return true;
	//else return false;
	Ogre::Image img;
	img.load(str, Ogre::ResourceGroupManager::getSingleton().getWorldResourceGroupName());
	//// check width and height are x^2 + 1
	if(!isPow2(img.getWidth()-1) || !isPow2(img.getHeight()-1))
	{
		_errorMsg = "The heightmap image is an invalid size: " +
			Ogre::StringConverter::toString(img.getWidth()) +
			" x " + Ogre::StringConverter::toString(img.getHeight()) +
			". It should be 2^n+1, 2^n+1.";
		return false;
	}
	else if(img.getFormat() != Ogre::PF_L8)
	{
		_errorMsg = "The heightmap image is not a grayscale image.";
		return false;
	}
	else return true;
}

bool ValidatorTexture::isOK(const std::string& str)
{
	Ogre::Image img;
	img.load(str, Ogre::ResourceGroupManager::getSingleton().getWorldResourceGroupName());
	//// check width and height are x^2
	if(!isPow2(img.getWidth()) || !isPow2(img.getHeight()))
	{
		_errorMsg = "The texture image is an invalid size: " +
			Ogre::StringConverter::toString(img.getWidth()) +
			" x " + Ogre::StringConverter::toString(img.getHeight()) +
			". It should be 2^m, 2^n.";
		return false;
	}
	return true;
}

