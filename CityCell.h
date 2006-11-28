#ifndef CITYCELL_H
#define CITYCELL_H

#include "stdafx.h"
#include "RoadGraph.h"

typedef struct {
	int seed;
	Ogre::Real segmentSize;
	float	segmentDeviance;
	unsigned int degree;
	float	degreeDeviance;
	Ogre::Real snapSize;
	float snapDeviance;
} GrowthGenParams;

class CityCell {

private:
	RoadGraph* mParentRoadGraph;

public:
	Primitive mBoundaryPrimitive;
	GrowthGenParams mGrowthGenParams;
	RoadGraph mRoadGraph;

public:
	//CityCell(RoadGraph* parent);
	CityCell(RoadGraph* parent, const Primitive &boundary);

	/** Function for writing to a stream.
    */
    inline friend std::ostream& operator <<
        ( std::ostream& o, const CityCell& c)
    {
		Primitive::const_iterator pIt, pEnd;
		o << "CityCell: [ ";
		o << "Boundary: ";
		for(pIt = c.mBoundaryPrimitive.begin(), pEnd = c.mBoundaryPrimitive.end(); pIt != pEnd; pIt++) 
			o << *pIt << " ";
		o << "]";
        return o;
    }

	void growthGenerate();
	void growthGenerate(const GrowthGenParams& gp);

};

#endif 
