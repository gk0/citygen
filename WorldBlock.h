#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "WorldLot.h"

class WorldBlock
{
private:
	std::vector<WorldLot>						_lots;
	std::vector< std::vector<Ogre::Vector3> >	_debugLots;

	std::vector<Ogre::Vector3>					_vertices;
	std::vector<Ogre::Vector3>					_normals;
	std::vector<Ogre::Vector2>					_texCoords;
	std::vector<size_t>							_footpathPolys;	

public:
	WorldBlock(const std::vector<Ogre::Vector3> &boundary, const CellGenParams &gp, rando rg, bool debug=false);

	void build(Ogre::ManualObject* f1, Ogre::ManualObject* m1, Ogre::ManualObject* m2, Ogre::ManualObject* m3, 
		Ogre::uint16 &o0, Ogre::uint16 &o1, Ogre::uint16 &o2);
	void build(Ogre::ManualObject* f1, Ogre::ManualObject* m1, Ogre::ManualObject* m2, Ogre::ManualObject* m3,
		Ogre::ManualObject* dob, Ogre::uint16 &o0, Ogre::uint16 &o1, Ogre::uint16 &o2);

	static bool getLongestSideIndex(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool splitBoundary(const size_t &index, const Ogre::Real &deviance, rando rg, const LotBoundary &input, std::vector<LotBoundary> &output);

};

#endif
