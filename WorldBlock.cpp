#include "stdafx.h"
#include "WorldBlock.h"
#include "WorldLot.h"
#include "Geometry.h"
#include "WorldCell.h" 

using namespace Ogre;
using namespace std;


void WorldBlock::build(const vector<Vector2> &boundary, Real f, const GrowthGenParams &gp, ManualObject* mObject)
{
	// Do something to extract lots
	queue< vector< pair<bool, Vector2> > > q;

	// the bool stores is the side is adjacent a road
	vector<pair<bool, Vector2>> blockBoundary;
	vector<vector<Vector2>> lotBoundaries;
	BOOST_FOREACH(const Vector2 &b, boundary) blockBoundary.push_back(make_pair<bool, Vector2>(true, b));
	q.push(blockBoundary);
	Real lotSz = Math::Sqr(gp.lotSize);

	while(!q.empty())
	{
		vector< vector<pair<bool, Vector2>> > boundaries;
		boundaries.push_back(q.front());
		q.pop();
		
		//	find the longest side
		size_t lsIndex;
		if(!getLongestSideAboveLimit(boundaries[0], lotSz, lsIndex))
		{
			// create lot boundary
			vector<Vector2> lotBoundary;
			for(size_t i=0; i<boundaries[0].size(); i++) lotBoundary.push_back(boundaries[0][i].second);
			lotBoundaries.push_back(lotBoundary);
			continue;
		}

		if(splitBoundary(lsIndex, gp.lotDeviance, boundaries))
		{
			size_t k = 0;
			// check boundary borders road
			for(size_t j, i=0; i<boundaries.size(); i++)
			{
				for(j=0; j<boundaries[i].size(); j++)
				{
					if(boundaries[i][j].first)
					{
						q.push(boundaries[i]);
						k++;
						break;
					}
				}
			}
			//LogManager::getSingleton().logMessage("Road border: "+StringConverter::toString(k));
		}else
			LogManager::getSingleton().logMessage("Shit");

	}


	// build lots
	//LogManager::getSingleton().logMessage("Lots: "+StringConverter::toString(lotBoundaries.size()));
	Real buildingDeviance = gp.buildingHeight * gp.buildingDeviance;
	Real buildingHeight = f + gp.buildingHeight - (buildingDeviance / 2);
	BOOST_FOREACH(vector<Vector2> &b, lotBoundaries)
	{

		WorldLot::build(b, f, buildingHeight + (buildingDeviance * ((float)rand()/(float)RAND_MAX)), mObject);
	}
}

bool WorldBlock::splitBoundary(const size_t &index, const Real &deviance, vector<vector<pair<bool, Vector2>>> &b)
{
	//
	size_t i, j, N = b[0].size();
	assert(N >= 3);

	// use normal of longest side to divide the
	Vector2 longestSide = b[0][(index+1)%N].second - b[0][index].second;
	Vector2 longestSidePerp = longestSide.perpendicular();

	//TODO: simplify/optimise this line, is too long
	Vector2 longestSideMid = b[0][index].second + (longestSide/2) - (longestSide*(deviance/2)) + (longestSide*(deviance * ((float)rand()/(float)RAND_MAX)));

	// now i
	vector<size_t> inscnIndices;
	vector<Vector2> inscnPoints;
	for(i=0,j=0; i<N; i++)
	{
		j = (i+1) % N;
		Real r,s;
		Vector2 inscn;
		if(Geometry::lineIntersect(b[0][i].second, b[0][j].second, longestSideMid, 
			longestSideMid+longestSidePerp, inscn, r, s) && r>=0 && r<=1)
		{
			inscnIndices.push_back(i);
			inscnPoints.push_back(inscn);
		}
	}
	if(inscnIndices.size() != 2) return false;
	//LogManager::getSingleton().logMessage("Inscn: "+StringConverter::toString(inscnIndices.size()));


	vector<vector<pair<bool,Vector2>>> lotBoundaries;
	vector<pair<bool,Vector2>> lotBoundary;
	lotBoundary.push_back(make_pair<bool,Vector2>(b[0][inscnIndices[0]].first, inscnPoints[0]));
	for(i=inscnIndices[0]+1,j=1; ; i=(i+1)%N)
	{
		if(i==((inscnIndices[j]+1)%N))
		{
			lotBoundary.push_back(make_pair<bool,Vector2>(false, inscnPoints[j]));
			lotBoundaries.push_back(lotBoundary);
			lotBoundary.clear();
			if(j == 0) break;
			lotBoundary.push_back(make_pair<bool,Vector2>(b[0][inscnIndices[j]].first, inscnPoints[j]));
			lotBoundary.push_back(b[0][i]);
			if((j+1) == inscnIndices.size()) j = 0;
		}
		else
		{
			lotBoundary.push_back(b[0][i]);
		}
	}

	b.swap(lotBoundaries);
	return true;

}

bool WorldBlock::getLongestSideAboveLimit(const vector<pair<bool, Vector2>> &b, const Real limitSq, size_t &index)
{
	// get size and check it is a poly
	size_t N = b.size();
	assert(N >= 3);

	// declare vars and set initial longest to first segment
	Ogre::Real currLSq, lsLSq = (b[0].second - b[1].second).squaredLength();
	size_t lsIndex = 0;

	for(size_t i=1,j=0; i<N; i++)
	{
		// j = next point index
		j = (i+1) % N;

		// get current segment length
		currLSq = (b[i].second - b[j].second).squaredLength();

		// if current length is longest store
		if(currLSq > lsLSq)
		{
			lsLSq = currLSq;
			lsIndex = i;
		}
	}
	// if longest segment is above limit return true
	if(lsLSq > limitSq)
	{
		index = lsIndex;
		return true;
	}
	else
		return false;
}

