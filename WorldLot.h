#ifndef WORLDLOT_H
#define WORLDLOT_H

#include "stdafx.h"
#include "CellParams.h"
#include "Triangulate.h"

class Ogre::Material;
class MeshBuilder;

class WorldLot
{
private:
	bool						_error;

	std::vector<Ogre::Real>		_vertexData;
	std::vector<Ogre::uint16>	_indexData;
   Ogre::Material* _wallMaterial;

   // cache vars
   struct MaterialInfo{
      Ogre::Material* _material; 
      Ogre::Real _u;
      Ogre::Real _v;
   };
   static std::vector<MaterialInfo> _wallMaterialCache;

public:

   WorldLot(std::vector<Ogre::Vector3> &poly, std::vector<bool> &isExterior, const CellParams &gp, rando rg);

	bool hasError() { return _error; }

	static void insetBoundary(std::vector<Ogre::Vector3> &poly, 
      std::vector<bool> &isExterior, const Ogre::Real &roadInset, 
		const Ogre::Real &standardInset);

	const std::vector<Ogre::Real>& getVertexData() { return _vertexData; }
	const std::vector<Ogre::uint16>& getIndexData() { return _indexData; }
	//void installVertexData(float*& vPtr);g
	//void installIndexData(Ogre::uint16*& iPtr, size_t& voffset);

   void registerData(MeshBuilder &mb);

private:
   void buildExtrusion(std::vector<Ogre::Vector3> &poly, const CellParams &gp, rando rg);

};

#endif
