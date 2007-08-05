#include "stdafx.h"
#include "WorldBlock.h"
#include "Geometry.h"
#include "CellGenParams.h" 
#include "WorldFrame.h"

using namespace Ogre;
using namespace std;

WorldBlock::WorldBlock(const vector<Vector2> &boundary, const CellGenParams &gp)
{
		// Do something to extract lots
	queue< LotBoundary > q;

	// the bool stores is the side is adjacent a road
	LotBoundary blockBoundary;
	vector< LotBoundary > lotBoundaries;
	BOOST_FOREACH(const Vector2 &b, boundary) blockBoundary.push_back(LotBoundaryPoint(true, b));
	q.push(blockBoundary);
	Real lotSplitSz = Math::Sqr(2*gp.lotSize);

	size_t count=0;
	while(!q.empty())
	{
		if(count>10000) return;
		count++;
		LotBoundary b(q.front());
		q.pop();
		
		//	find the longest side
		size_t lsIndex;

		//if(!getLongestSideAboveLimit(b, lotSplitSz, lsIndex))
		if(!getLongestSideIndex(b, lotSplitSz, lsIndex))
		{
			// add to lot boundaries
			lotBoundaries.push_back(b);
			continue;
		}

		std::vector<LotBoundary> splitBoundaries;
		if(splitBoundary(lsIndex, gp.lotDeviance, b, splitBoundaries))
		{
			// check boundary borders road
			for(size_t j, i=0; i<splitBoundaries.size(); i++)
			{
				for(j=0; j<splitBoundaries[i].size(); j++)
				{
					if(splitBoundaries[i][j]._roadAccess)
					{
						q.push(splitBoundaries[i]);
						break;
					}
				}
			}
			//LogManager::getSingleton().logMessage("Road border: "+StringConverter::toString(k));
		}
		//else
		//	LogManager::getSingleton().logMessage("WorldBlock::splitLotBoundary() failed.");
	}

	//_lots = lotBoundaries;
	Real buildingDeviance = gp.buildingHeight * gp.buildingDeviance;

	size_t fail = 0;
	BOOST_FOREACH(LotBoundary &b, lotBoundaries)
	{
		Vector3 p;
		WorldFrame::getSingleton().plotPointOnTerrain(b[0]._pos, p);
		Real foundation = p.y;
		Real buildingHeight = foundation + gp.buildingHeight - (buildingDeviance / 2);

		WorldLot lot(b, gp, foundation, buildingHeight + (buildingDeviance * ((float)rand()/(float)RAND_MAX)));
		if(lot.hasError())
			fail++;
		else
			_lots.push_back(lot);

	}
}


void WorldBlock::build(ManualObject* mObject,  ManualObject* dob)
{
	BOOST_FOREACH(WorldLot& lot, _lots)
		lot.build(mObject);
	//	// draw debug lines so
	//	if(dob)
	//	{
	//		for(size_t i=0; i<splitBoundaries.size(); i++)
	//		{
	//			for(size_t k,j=0; j<splitBoundaries[i].size(); j++)
	//			{	
	//				k = (j+1)%splitBoundaries[i].size();

	//				Vector3 a,b;
	//				WorldFrame::getSingleton().plotPointOnTerrain(splitBoundaries[i][j]._pos, a);
	//				WorldFrame::getSingleton().plotPointOnTerrain(splitBoundaries[i][k]._pos, b);
	//				dob->position(a+Vector3(0,1.3,0));
	//				dob->position(b+Vector3(0,1.3,0));
	//			}
	//		}
	//	}
	//}


	// build lots
	//LogManager::getSingleton().logMessage("Lots: "+StringConverter::toString(lotBoundaries.size()));
	//Real buildingDeviance = gp.buildingHeight * gp.buildingDeviance;

	//size_t fail = 0;
	//BOOST_FOREACH(LotBoundary &b, _lots)
	//{
	//	Vector3 p;
	//	WorldFrame::getSingleton().plotPointOnTerrain(b[0]._pos, p);
	//	Real foundation = p.y;
	//	Real buildingHeight = foundation + gp.buildingHeight - (buildingDeviance / 2);
	//	if(!WorldLot::build(b, gp, foundation, buildingHeight + (buildingDeviance * ((float)rand()/(float)RAND_MAX)), mObject))
	//		fail++;
	//}

	//DEBUG
	//LogManager::getSingleton().logMessage("WorldBlock::build() "+StringConverter::toString(count)+" ops "+StringConverter::toString(fail)+" failed.");
}


bool rayCross(const Ogre::Vector2& pos, const LotBoundary &b)
{
	bool bRayCross = false;
	for(size_t i = 0; i < b.size(); i++)
	{
		size_t j = (i+1) % b.size();
		if(Geometry::rayCross(pos, b[i]._pos, b[j]._pos)) bRayCross = !bRayCross;
	}
	return bRayCross;
}

struct Intersection
{
	Real _s;
	size_t _index;
	size_t _boundaryIndex;
	size_t _nextIndex;
	Vector2 _pos;
};
struct compare_inscns{
	bool operator()(const Intersection *l, const Intersection *r)
	{
		return l->_s > r->_s;
	}
};


// TODO: doesn't work with concave blocks
// as an alternative to this i could just make a road graph and extract cells
// intersection testing would still be required so it definately would be more expensive
bool WorldBlock::splitBoundary(const size_t &index, const Real &deviance, const LotBoundary &input, std::vector<LotBoundary> &output)
{
	//
	size_t i, j, N = input.size();
	assert(N >= 3 && N < 10000);

	// use normal of longest side to divide the
	Vector2 longestSide = input[(index+1)%N]._pos - input[index]._pos;
	Vector2 longestSidePerp = longestSide.perpendicular();

	//TODO: simplify/optimise this line, is too long
	Vector2 longestSideMid = input[index]._pos + (longestSide/2) - (longestSide*(deviance/2)) + 
										(longestSide*(deviance * ((float)rand()/(float)RAND_MAX)));


	// fill a vector of intersections, ordered as they're encountered
	vector<Intersection> inscns;
	for(i=0,j=0; i<N; i++)
	{
		j = (i+1) % N;
		Real r;
		Intersection inscn;
		if(Geometry::lineIntersect(input[i]._pos, input[j]._pos, longestSideMid, 
			longestSideMid+longestSidePerp, inscn._pos, r, inscn._s) && r>=0 && r<=1)
		{
			inscn._index = inscns.size();
			inscn._boundaryIndex = i;
			inscns.push_back(inscn);
		}
	}

	// need sort a copy of intersections by pos on line
	vector<Intersection*> inscnsOrdered;
	BOOST_FOREACH(Intersection& in, inscns) inscnsOrdered.push_back(&in);
	sort(inscnsOrdered.begin(), inscnsOrdered.end(), compare_inscns());
	//sort(inscnsOrdered.begin(), inscnsOrdered.end());


	// create an array of bools that store whether an intersection 
	// segment is contained inside or outside of the polygon
	vector<bool> intersectionSegInside(inscnsOrdered.size()-1);
	for(i=0; i<(inscnsOrdered.size()-1); i++)
	{
		intersectionSegInside[i] = rayCross((inscnsOrdered[i]->_pos + inscnsOrdered[i+1]->_pos)/2, input);
		if(intersectionSegInside[i])
			inscnsOrdered[i]->_nextIndex = inscnsOrdered[i+1]->_index;
		else
			inscnsOrdered[i]->_nextIndex = numeric_limits<size_t>::max();
	}

	for(size_t side=0; side<2; side++)
	{
		// Process side of intersection line
		for(i=0; i<(inscnsOrdered.size()-1); i++)
		{
			Intersection* start = inscnsOrdered[i];

			// declare lot boundary
			LotBoundary lotBoundary;

			// if intersection segment store
			if(start->_nextIndex == numeric_limits<size_t>::max()) continue;
			lotBoundary.push_back(LotBoundaryPoint(false, start->_pos));
			lotBoundary.push_back(LotBoundaryPoint(
				input[inscns[start->_nextIndex]._boundaryIndex]._roadAccess, inscns[start->_nextIndex]._pos));
			
			size_t boundaryIndex = (inscns[start->_nextIndex]._boundaryIndex + 1) % N;
			size_t inscnIndex = (inscns[start->_nextIndex]._index + 1) % inscns.size();

			// break start segment
			start->_nextIndex = numeric_limits<size_t>::max();

			while(true)
			{
				if(boundaryIndex != inscns[inscnIndex]._boundaryIndex)
				{
					lotBoundary.push_back(input[boundaryIndex]);
					boundaryIndex = (boundaryIndex + 1) % N;
				} 
				else
				{

					lotBoundary.push_back(input[boundaryIndex]);

					// if intersection segment
					if(inscns[inscnIndex]._nextIndex < 10000)//!= numeric_limits<size_t>::max())
					{
						Intersection* first = &inscns[inscnIndex];

						// add first to boundary
						lotBoundary.push_back(LotBoundaryPoint(false, first->_pos));
						inscnIndex = first->_nextIndex;

						//break seg
						first->_nextIndex = numeric_limits<size_t>::max();

						// add second to boundary
						lotBoundary.push_back(LotBoundaryPoint(
							input[inscns[inscnIndex]._boundaryIndex]._roadAccess, inscns[inscnIndex]._pos)); //error

						// advance indices
						boundaryIndex = (inscns[inscnIndex]._boundaryIndex + 1) % N;
						inscnIndex = (inscns[inscnIndex]._index + 1) % inscns.size();

					}
					// else intersection point
					else
					{
						if(start == &inscns[inscnIndex])
						{
							output.push_back(lotBoundary);

							//store 
							break;
						}
						else
						{
							// add intersection point
							boundaryIndex = inscns[inscnIndex]._boundaryIndex;
							inscnIndex = (inscnIndex + 1) % inscns.size();
						}
					}
				}
			}
		}
		
		// change vars for other side
		for(i=0; i<(inscnsOrdered.size()-1); i++)
		{
			if(intersectionSegInside[i])
				inscnsOrdered[i+1]->_nextIndex = inscnsOrdered[i]->_index;
			else
				inscnsOrdered[i+1]->_nextIndex = numeric_limits<size_t>::max();
		}
		inscnsOrdered = vector<Intersection*>(inscnsOrdered.rbegin(), inscnsOrdered.rend());

	}
	return true;
}

bool WorldBlock::getLongestRoadSideAboveLimit(const LotBoundary &b, const Real limitSq, size_t &index)
{
	// get size and check it is a poly
	size_t N = b.size();
	assert(N >= 3);

	// declare vars and set initial longest to first segment
	Ogre::Real currLSq, lsLSq = 0;
	size_t lsIndex = 0;

	for(size_t i=0,j=0; i<N; i++)
	{
		j = (i+1) % N;

		//
		if(!b[i]._roadAccess) continue;

		// get current segment length
		currLSq = (b[i]._pos - b[j]._pos).squaredLength();

		// if current length is longest store
		if(currLSq > lsLSq && currLSq > limitSq)
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

bool WorldBlock::getLongestSideAboveLimit(const LotBoundary &b, const Real limitSq, size_t &index)
{
	// get size and check it is a poly
	size_t N = b.size();
	assert(N >= 3);

	// declare vars and set initial longest to first segment
	Ogre::Real currLSq, lsLSq = (b[0]._pos - b[1]._pos).squaredLength();
	size_t lsIndex = 0;

	for(size_t i=1,j=0; i<N; i++)
	{
		// j = next point index
		j = (i+1) % N;

		// get current segment length
		currLSq = (b[i]._pos - b[j]._pos).squaredLength();

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


bool WorldBlock::getLongestSideIndex(const LotBoundary &b, const Real limitSq, size_t &index)
{
	// get size and check it is a poly
	size_t N = b.size();
	assert(N >= 3);

	// declare vars and set initial longest to first segment
	Ogre::Real currLSq, lsLSq = 0;
	size_t lsIndex = 0;

	for(size_t i=0; i<N; i++)
	{
		if(!b[i]._roadAccess) continue;		// skip sides with 'no road access'	

		// get current segment length
		currLSq = (b[i]._pos - b[(i+1)%N]._pos).squaredLength();

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
	{
		for(size_t i=0; i<N; i++)
		{
			if(b[i]._roadAccess) continue;		// skip sides with 'road access'	

			// get current segment length
			currLSq = (b[i]._pos - b[(i+1)%N]._pos).squaredLength();

			// if current length is longest store
			if(currLSq > lsLSq)
			{
				lsLSq = currLSq;
				lsIndex = i;
			}
		}
		if(lsLSq > limitSq)
		{
			index = lsIndex;
			return true;
		}
	}

	return false;
}
