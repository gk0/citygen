#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "WorldCell.h"

//#ifndef GrowthGenParams
//#define GrowthGenParams
//class GrowthGenParams;
//#endif

class WorldBlock
{
private:

public:
	static void build(const std::vector<Ogre::Vector2> &boundary, 
		Ogre::Real f, const GrowthGenParams &gp, Ogre::ManualObject* mObject);

	static bool getLongestSideAboveLimit(const std::vector<std::pair<bool, Ogre::Vector2>>& b, 
		const Ogre::Real limitSq, size_t &index);

	static bool splitBoundary(const size_t &index,  const Ogre::Real &deviance, std::vector<std::vector<std::pair<bool, Ogre::Vector2>>> &b);

};

#endif
