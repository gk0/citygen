#include "stdafx.h"
#include "WorldTerrain.h"
#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <sstream>
using namespace std;

WorldTerrain::WorldTerrain()
{
	_heightmapImage="terrain.png";
	_pageSize=513;
	_tileSize=65;
	_maxPixelError=3;
	_pageWorldX=50000;
	_pageWorldZ=50000;
	_maxHeight=3000;
	_maxMipMapLevel=5;
	_vertexProgramMorph=true;
	_LODMorphStart=0.2;
	_materialName="gk/Terrain";
}

void WorldTerrain::load(Ogre::SceneManager* sm)
{
	// B. manual approach - this does seem a tad silly but it seems
	// the only way to do it harhar
	stringstream oss;
	oss << "PageSource=Heightmap\n";
	oss << "Heightmap.image=" << _heightmapImage << "\n";
	oss << "PageSize=" << _pageSize << "\n";
	oss << "TileSize=" << _tileSize << "\n";
	oss << "MaxPixelError=" << _maxPixelError << "\n";
	oss << "PageWorldX=" << _pageWorldX << "\n";
	oss << "PageWorldZ=" << _pageWorldZ << "\n";
	oss << "MaxHeight=" << _maxHeight << "\n";
	oss << "MaxMipMapLevel=" << _maxMipMapLevel << "\n";
	oss << "VertexProgramMorph=" << (_vertexProgramMorph ? "yes" : "no") << "\n";
	oss << "LODMorphStart=" << _LODMorphStart << "\n";
	oss << "CustomMaterialName=" << _materialName << "\n";
	Ogre::LogManager::getSingleton().logMessage(oss.str());
	string configStr(oss.str());

	void *pMem = (void *)new unsigned char[configStr.length()+1];
	memset(pMem, 0, configStr.length()+1);
	memcpy(pMem, configStr.c_str(), configStr.length() + 1);
	// stuff this into a MemoryDataStream
	Ogre::DataStreamPtr pStr(new Ogre::MemoryDataStream(pMem, configStr.length() + 1));
	sm->setWorldGeometry(pStr);
}

