#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "WorldLot.h"
#include "MeshBuilder.h"

class WorldBlock
{
private:
	std::vector<WorldLot*>						_lots;
	std::vector< std::vector<Ogre::Vector3> >	_debugLots;

	std::vector<Ogre::uint16>					_footpathPolys;		
	std::vector<Ogre::Real>						_footpathVertexData;

public:
	WorldBlock(const std::vector<Ogre::Vector3> &boundary, const CellGenParams &gp, rando rg, bool debug=false);
	~WorldBlock()
	{
		BOOST_FOREACH(WorldLot* l, _lots) delete l;
	}

	void build(MeshBuilder &meshBuilder, std::vector<Ogre::Material*> &materials);
	void build(MeshBuilder &meshBuilder, std::vector<Ogre::Material*> &materials, Ogre::ManualObject* dob);

	static bool getLongestSideIndex(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool splitBoundary(const size_t &index, const Ogre::Real &deviance, rando rg, const LotBoundary &input, std::vector<LotBoundary> &output);

private:
	inline void appendVector3(std::vector<Ogre::Real> &vertexData, const Ogre::Vector3& v)
	{
		vertexData.push_back(v.x);
		vertexData.push_back(v.y);
		vertexData.push_back(v.z);
	}

	inline void appendVector3(std::vector<Ogre::Real> &vertexData, 
		const Ogre::Real &x, const Ogre::Real &y, const Ogre::Real &z)
	{
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
	}

	inline void appendVector2(std::vector<Ogre::Real> &vertexData, 
		const Ogre::Real &x, const Ogre::Real &y)
	{
		vertexData.push_back(x);
		vertexData.push_back(y);
	}

	inline void appendPoly(std::vector<Ogre::uint16> &indexData, 
		const Ogre::uint16 &a, const Ogre::uint16 &b, const Ogre::uint16 &c)
	{
		indexData.push_back(a);
		indexData.push_back(b);
		indexData.push_back(c);
	}
};

#endif
