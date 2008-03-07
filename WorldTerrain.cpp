#include "stdafx.h"
#include "WorldTerrain.h"
#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <tinyxml.h>
#include <sstream>

#include "Validator.h"
#include <math.h>
#include <wx/msgdlg.h>

using namespace std;

// TODO property hierarchy, validation using regxp

WorldTerrain::WorldTerrain()
{
	vector<string> yesNo;
	yesNo.push_back("no");
	yesNo.push_back("yes");

	PropertyList* hmOpts = new PropertyList("Heightmap");
	hmOpts->addProperty(new PropertyImage("Heightmap.image", "terrain1.png", _validatorHM));
	hmOpts->addProperty(new PropertyUShort("TileSize", 65, ValidatorURange::USHORTMAX));
	hmOpts->addProperty(new PropertyUShort("MaxPixelError", 3, ValidatorURange::USHORTMAX));
	_properties.addProperty(hmOpts);

	PropertyList* texOpts = new PropertyList("Texture");
	texOpts->addProperty(new PropertyImage("WorldTexture", "terrain_texture1.jpg", _validatorTex));
	texOpts->addProperty(new PropertyImage("DetailTexture", "terrain_detail.jpg", _validatorTex));
	texOpts->addProperty(new PropertyUShort("DetailTile", 3, ValidatorURange::USHORTMAX));
	_properties.addProperty(texOpts);

	PropertyList* scaleOpts = new PropertyList("Scale");
	scaleOpts->addProperty(new PropertyReal("PageWorldX", 8000));
	scaleOpts->addProperty(new PropertyReal("PageWorldZ", 8000));
	scaleOpts->addProperty(new PropertyReal("MaxHeight", 1200));
	_properties.addProperty(scaleOpts);

	PropertyList* renderOpts = new PropertyList("Rendering");
	renderOpts->addProperty(new PropertyEnum("VertexProgramMorph", 1, yesNo));	
	renderOpts->addProperty(new PropertyReal("LODMorphStart", 0.2));
	renderOpts->addProperty(new PropertyUShort("MaxMipMapLevel", 5));
	_properties.addProperty(renderOpts);
}

void WorldTerrain::load(Ogre::SceneManager* sm)
{
	// convert property list into a string config
	std::string configStr = "PageSource=Heightmap\n";
	BOOST_FOREACH(Property* p, _properties.getList())
	{
		if(typeid(*p) == typeid(PropertyList))
		{
			PropertyList* pl = static_cast<PropertyList*>(p);
			BOOST_FOREACH(Property* p2, pl->getList())
				configStr += (p2->_name)+"="+(p2->toString())+"\n";
		}
		else
			configStr += (p->_name)+"="+(p->toString())+"\n";
	}

	// need to get page size from image
	Property* p;
	try
	{
		Ogre::Image img;
		
		_properties.findAll("Heightmap.image", p);
		img.load(p->toString(), Ogre::ResourceGroupManager::getSingleton().getWorldResourceGroupName());
		configStr += "PageSize="+Ogre::StringConverter::toString(std::min(img.getWidth(),img.getHeight()))+"\n";
	}
	catch(...)
	{
		string errMsg = "Could not load the image: \'"+p->toString()+
			"\', specified by the \'Heightmap.image\' property of the terrain."+
			"\n\nIf this file has been moved please correct the path in the Terrain Property Inspector.";

		(void)wxMessageBox(_U(errMsg.c_str()), _("Heightmap image load error"),
			wxOK | wxICON_EXCLAMATION);
		return;
	}

	Ogre::LogManager::getSingleton().logMessage(configStr);
	
	// memory leak version:
	// put configStr into a MemoryDataStream
	//void *pMem = (void *)new unsigned char[configStr.length()+1];
	//memcpy(pMem, configStr.c_str(), configStr.length() + 1);

	// dubious but memory-leak free version
	void *pMem = (void*) configStr.c_str();
	Ogre::DataStreamPtr pStr(new Ogre::MemoryDataStream(pMem, configStr.length() + 1));

	try
	{
		sm->setWorldGeometry(pStr);
	}
	catch(Ogre::Exception &e)
	{
		(void)wxMessageBox(_U(e.getDescription().c_str()), 
			_("Terrain load error!"), wxOK | wxICON_EXCLAMATION);
	}
}

bool WorldTerrain::loadXML(const TiXmlHandle& root, const std::string &filePath)
{
	return _properties.loadXML(root, filePath);
}

TiXmlElement* WorldTerrain::saveXML(const std::string &filePath)
{
	TiXmlElement* root = new TiXmlElement("terrain");
	BOOST_FOREACH(Property* p, _properties.getList())
		root->LinkEndChild(p->saveXML(filePath));
	return root;
}

Ogre::Real WorldTerrain::getTerrainX()
{
	PropertyList* pl = static_cast<PropertyList*>(_properties.getPropertyPtr("Scale"));
	return static_cast<PropertyReal*>(pl->getPropertyPtr("PageWorldX"))->_data;
}

Ogre::Real WorldTerrain::getTerrainZ()
{
	PropertyList* pl = static_cast<PropertyList*>(_properties.getPropertyPtr("Scale"));
	return static_cast<PropertyReal*>(pl->getPropertyPtr("PageWorldZ"))->_data;
}