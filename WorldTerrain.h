#ifndef WORLDTERRAIN_H
#define WORLDTERRAIN_H

#include "stdafx.h"
#include "PropertyList.h"

namespace Ogre {
class SceneManager;
}

class WorldTerrain
{

private:
	PropertyList _properties;
	ValidatorTexture _validatorTex;
	ValidatorHeightmap _validatorHM;

public:
	WorldTerrain();


	void load(Ogre::SceneManager* sm);
	bool loadXML(const TiXmlHandle& worldRoot, const std::string &filePath);
	const PropertyList& getProperties() const
	{
		return _properties;
	}
	TiXmlElement* saveXML(const std::string &filePath);



};

#endif
