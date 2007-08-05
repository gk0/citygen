#ifndef CELLGENPARAMS_H
#define CELLGENPARAMS_H

typedef struct {
	unsigned int type;
	int seed;
	Ogre::Real segmentSize;
	Ogre::Real segmentDeviance;
	unsigned int degree;
	Ogre::Real degreeDeviance;
	Ogre::Real snapSize;
	Ogre::Real snapDeviance;
	Ogre::Real buildingHeight;
	Ogre::Real buildingDeviance;
	Ogre::Real roadWidth;
	size_t roadLimit;
	Ogre::Real connectivity;
	Ogre::Real lotSize;
	Ogre::Real lotDeviance;
} CellGenParams;

#endif
