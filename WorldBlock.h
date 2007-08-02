#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "WorldCell.h"
#include "WorldLot.h"

//#ifndef GrowthGenParams
//#define GrowthGenParams
//class GrowthGenParams;
//#endif

class WorldBlock
{
private:

public:
	static void build(const std::vector<Ogre::Vector2> &boundary, 
		Ogre::Real f, const GrowthGenParams &gp, Ogre::ManualObject* mObject, Ogre::ManualObject* dob = 0);

	static bool getLongestSideAboveLimit(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool getLongestRoadSideAboveLimit(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool getLongestSideIndex(const LotBoundary &b, 
		const Ogre::Real limitSq, size_t &index);

	static bool splitBoundary(const size_t &index, const Ogre::Real &deviance, const LotBoundary &input, std::vector<LotBoundary> &output);

};

#endif
