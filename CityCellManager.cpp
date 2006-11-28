#include "stdafx.h"
#include "CityCellManager.h"


CityCellManager::CityCellManager()
{
}

void CityCellManager::setCells(RoadGraph* parent, const PrimitiveVec& primitives)
{
	// for each primitive create a cell for closed loops
	for(PrimitiveVec::const_iterator pit = primitives.begin(); pit != primitives.end(); pit++)
	{
		const Primitive& primitive(*pit);
		if(primitive.type == MINIMAL_CYCLE)
		{
			//Create Cell
			mCityCells.push_back(CityCell(parent, primitive)); 
		}
	}
}

std::pair<const CityCellIterator, const CityCellIterator> CityCellManager::getCells()
{
	return std::make_pair(mCityCells.begin(), mCityCells.end());
}
