#ifndef WORLDTERRAIN_H
#define WORLDTERRAIN_H

#include "stdafx.h"

namespace Ogre {
class SceneManager;
}

class WorldTerrain
{

private:
	std::string _heightmapImage;
	ushort  _pageSize;
	ushort  _tileSize;
	ushort   _maxPixelError;
	uint	_pageWorldX;
	uint	_pageWorldZ;
	ushort  _maxHeight;
	ushort  _maxMipMapLevel;
	bool	_vertexProgramMorph;
	float	_LODMorphStart;
	std::string _materialName;

public:
	WorldTerrain();

	void load(Ogre::SceneManager* sm);

};

#endif
