#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "WorldLot.h"

class WorldBlock
{
private:
	std::vector<WorldLot> _lots;
	//std::vector<LotBoundary> _lots;

public:
	WorldBlock(const std::vector<Ogre::Vector2> &boundary, const CellGenParams &gp);

	void build(Ogre::ManualObject* mObject, Ogre::ManualObject* dob = 0);

	static bool getLongestSideAboveLimit(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool getLongestRoadSideAboveLimit(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool getLongestSideIndex(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool splitBoundary(const size_t &index, const Ogre::Real &deviance, const LotBoundary &input, std::vector<LotBoundary> &output);

};

#endif
