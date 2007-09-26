#include "stdafx.h"
#include "WorldBlock.h"
#include "Geometry.h"
#include "CellGenParams.h"
#include "WorldFrame.h"

using namespace Ogre;
using namespace std;

//THIS CLASS IS FUCKING HORRIBLE


WorldBlock::WorldBlock(const vector<Vector3> &boundary, const CellGenParams &gp, rando rg, bool debug)
//  : _randGen(rg)  // not assignable see docs, maybe can be
{
	// 
	size_t i,j,N = boundary.size(), N2 = N * 2;
	vector<Vector3> innerBoundary;
	innerBoundary.reserve(N);
	vector<Vector3> outerBoundary(boundary);

	// build a footpath
	_vertices.reserve(3*N);
	_normals.reserve(N);
	_footpathPolys.reserve(12*N);

	// raise footpath
	// footpath does not support inset properly
	// different number of vertices is possible
	for(i=0; i<N; i++) outerBoundary[i].y += 0.035; 
	//_vertices.insert(_vertices.end(), outerBoundary.begin(), outerBoundary.end());
	innerBoundary.insert(innerBoundary.end(), outerBoundary.begin(), outerBoundary.end());

	if(Geometry::polygonInsetFast(0.2, innerBoundary))
	{
		//Geometry::polyRepair(innerBoundary, N/2);
		if(innerBoundary.size() != N)
		{
			N = innerBoundary.size();
			N2 = N * 2;
			vector<Vector3> tmp;
			tmp.insert(tmp.end(), innerBoundary.begin(), innerBoundary.end());
			if(!Geometry::polygonInsetFast(-0.2, tmp)) return;
			//Geometry::polygonInsetFFast(-0.2, tmp);
			if(tmp.size() != N)
			{
				//LogManager::getSingleton().logMessage("Outset Fail!!");
				return;
			}
			//else
			//	LogManager::getSingleton().logMessage("Outset Pass.");

			_vertices.insert(_vertices.end(), tmp.begin(), tmp.end());
			BOOST_FOREACH(Vector3 &v, _vertices) v.y -= 0.05;
			_vertices.insert(_vertices.end(), tmp.begin(), tmp.end());
		}
		else
		{
			_vertices.insert(_vertices.end(), boundary.begin(), boundary.end());
			_vertices.insert(_vertices.end(), outerBoundary.begin(), outerBoundary.end());
		}
		_vertices.insert(_vertices.end(), innerBoundary.begin(), innerBoundary.end());


		for(i=0; i<N; i++)
		{
			j = (i+1)%N;
			
			// build polygons for footpath

			// get normal for side
			_normals.push_back((innerBoundary[i]-innerBoundary[j]).perpendicular().normalisedCopy());


			// side
			_footpathPolys.push_back(j);
			_footpathPolys.push_back(i);
			_footpathPolys.push_back(N+i);
			_footpathPolys.push_back(j);
			_footpathPolys.push_back(N+i);
			_footpathPolys.push_back(N+j);

			// top
			_footpathPolys.push_back(N+j);
			_footpathPolys.push_back(N+i);
			_footpathPolys.push_back(N2+i);
			_footpathPolys.push_back(N+j);
			_footpathPolys.push_back(N2+i);
			_footpathPolys.push_back(N2+j);
		}
	}

	vector<size_t> tmp;
	if(!Triangulate::Process(innerBoundary, tmp))
	{
		//LogManager::getSingleton().logMessage("Boundary Fail!!"+StringConverter::toString(N));
		if(!Geometry::polyRepair(innerBoundary, 100))
		{
			_footpathPolys.clear();
			return;
		}
		else
		{
			N = innerBoundary.size();
			N2 = N * 2;
		}
	}

//	vector<Vector2> innerBoundary2;
//	innerBoundary2.reserve(innerBoundary.size());
//	BOOST_FOREACH(Vector3 &v, innerBoundary) innerBoundary2.push_back(Geometry::V2(v));

	// Do something to extract lots
	queue< LotBoundary > q;

	// the bool stores is the side is adjacent a road
	LotBoundary blockBoundary;
	vector< LotBoundary > lotBoundaries;
	BOOST_FOREACH(const Vector3 &b, innerBoundary) blockBoundary.push_back(LotBoundaryPoint(true, b));
	q.push(blockBoundary);
	Real lotSplitSzSq = Math::Sqr(2*gp._lotSize);

	size_t count=0;
	while(!q.empty())
	{
		//DEBUG
		if(count>10000) return;
		//if(gp._roadLimit && count > gp._roadLimit) return;
		count++;

		// get lot boundary from queue
		LotBoundary b(q.front());
		q.pop();

		//	find the longest side
		size_t lsIndex;

		//if(!getLongestSideAboveLimit(b, lotSplitSz, lsIndex))
		if(!getLongestSideIndex(b, lotSplitSzSq, lsIndex))
		{
			// add to lot boundaries
			lotBoundaries.push_back(b);

			if(debug)
			{
				std::vector<Vector3> debugLot;
				debugLot.reserve(b.size());
				BOOST_FOREACH(LotBoundaryPoint& bp, b)	debugLot.push_back(bp._pos);
				_debugLots.push_back(debugLot);
			}
			continue;
		}

		std::vector<LotBoundary> splitBoundaries;
		if(splitBoundary(lsIndex, gp._lotDeviance, rg, b, splitBoundaries))
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

				// if no road access
				if(j>=splitBoundaries[i].size())
				{
					if(debug)
					{
						std::vector<Vector3> debugLot;
						debugLot.reserve(splitBoundaries[i].size());
						BOOST_FOREACH(LotBoundaryPoint& bp, splitBoundaries[i])	debugLot.push_back(bp._pos);
						_debugLots.push_back(debugLot);
					}
				}
			}
			//LogManager::getSingleton().logMessage("Road border: "+StringConverter::toString(k));
		}
	}

	//_lots = lotBoundaries;
	Real buildingDeviance = gp._buildingHeight * gp._buildingDeviance;

	size_t fail = 0;
	BOOST_FOREACH(LotBoundary &b, lotBoundaries)
	{
		Real buildingHeight = gp._buildingHeight - (buildingDeviance / 2);
		WorldLot lot(b, gp, buildingHeight + (buildingDeviance * rg()));
		if(lot.hasError())
			fail++;
		else
			_lots.push_back(lot);
	}
}

#define RAND_MAX_THRD RAND_MAX/3
#define RAND_MAX_2THRD RAND_MAX_THRD*2

void WorldBlock::build(ManualObject* f1, ManualObject* m1, ManualObject* m2, 
					   ManualObject* m3, uint16 &o0, uint16 &o1, uint16 &o2)
{
	// footpath
	size_t i, normIndex = 0, N=_footpathPolys.size();
	for(i=0; i<N; i+=12)
	{
		// side
		f1->position(_vertices[_footpathPolys[i]]);
		f1->normal(_normals[normIndex]);
		f1->position(_vertices[_footpathPolys[i+1]]);
		f1->normal(_normals[normIndex]);
		f1->position(_vertices[_footpathPolys[i+2]]);
		f1->normal(_normals[normIndex]);
		f1->position(_vertices[_footpathPolys[i+3]]);
		f1->normal(_normals[normIndex]);
		f1->position(_vertices[_footpathPolys[i+4]]);
		f1->normal(_normals[normIndex]);
		f1->position(_vertices[_footpathPolys[i+5]]);
		f1->normal(_normals[normIndex]);

		// top
		f1->position(_vertices[_footpathPolys[i+6]]);
		f1->normal(Vector3::UNIT_Y);
		f1->position(_vertices[_footpathPolys[i+7]]);
		f1->normal(Vector3::UNIT_Y);
		f1->position(_vertices[_footpathPolys[i+8]]);
		f1->normal(Vector3::UNIT_Y);
		f1->position(_vertices[_footpathPolys[i+9]]);
		f1->normal(Vector3::UNIT_Y);
		f1->position(_vertices[_footpathPolys[i+10]]);
		f1->normal(Vector3::UNIT_Y);
		f1->position(_vertices[_footpathPolys[i+11]]);
		f1->normal(Vector3::UNIT_Y);

		normIndex++;
	}
	// WARNING: don't do m->begin m->end too much it makes it *very* slow to render

	// shit rand in the  build
	BOOST_FOREACH(WorldLot& lot, _lots) 
	{
		int rnd = rand();
		if(rnd < RAND_MAX_THRD)
		{
			lot.addVertexData(m1);
			o0 = lot.addIndexData(m1, o0);
		}
		else if(rnd < RAND_MAX_2THRD)
		{
			lot.addVertexData(m2);
			o1 = lot.addIndexData(m2, o1);
		}
		else
		{
			lot.addVertexData(m3);
			o2 = lot.addIndexData(m3, o2);
		}

		
	}
}

void WorldBlock::build(ManualObject* f1, ManualObject* m1, ManualObject* m2, 
					   ManualObject* m3,  ManualObject* dob, uint16 &o0, uint16 &o1, uint16 &o2)
{
	build(f1, m1, m2, m3, o0, o1, o2);

	//
	for(size_t i=0; i<_debugLots.size(); i++)
	{
		for(size_t j=0; j<_debugLots[i].size(); j++)
		{
			size_t k = (j+1)%_debugLots[i].size();
			Vector3 &a(_debugLots[i][j]), &b(_debugLots[i][k]);
			dob->position(a.x, a.y+0.1, a.z);
			dob->position(b.x, b.y+0.1, b.z);
		}
	}
}

bool rayCross(const Ogre::Vector3& pos, const LotBoundary &b)
{
	bool bRayCross = false;
	for(size_t i = 0; i < b.size(); i++)
	{
		size_t j = (i+1) % b.size();
		if(Geometry::rayCross(Geometry::V2(pos), Geometry::V2(b[i]._pos), Geometry::V2(b[j]._pos))) 
			bRayCross = !bRayCross;
	}
	return bRayCross;
}

bool WorldBlock::getLongestSideIndex(const LotBoundary &b, const Real limitSq, size_t &index)
{
	// get size and check it is a poly
	size_t N = b.size();
	assert(N >= 3);

	// declare vars and set initial longest to first segment
	Ogre::Real currLSq, lsLSq = 0;
	size_t lsIndex = 0;

	for(size_t j,i=0; i<N; i++)
	{
		// skip sides with 'no road access'
		if(!b[i]._roadAccess) continue;	

		j = (i+1)%N;

		// get current segment length squared in 2D
		currLSq = Math::Sqr(b[i]._pos.x-b[j]._pos.x) + Math::Sqr(b[i]._pos.z-b[j]._pos.z);

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
		for(size_t j,i=0; i<N; i++)
		{
			// skip sides with 'road access'
			if(b[i]._roadAccess) continue;	

			j = (i+1)%N;

			// get current segment length squared in 2D
			currLSq = Math::Sqr(b[i]._pos.x-b[j]._pos.x) + Math::Sqr(b[i]._pos.z-b[j]._pos.z);

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


struct Intersection
{
	Real _s;
	size_t _index;
	size_t _boundaryIndex;
	size_t _nextIndex;
	Vector3 _pos;
};
struct compare_inscns{
	bool operator()(const Intersection *l, const Intersection *r)
	{
		return l->_s > r->_s;
	}
};


// TODO: doesn't work with concave blocks
// as an alternative to this i could just make a road graph and extract cells
// intersection testing would still be required so it definitely would be more expensive
bool WorldBlock::splitBoundary(const size_t &index, const Real &deviance, rando rg, const LotBoundary &input, std::vector<LotBoundary> &output)
{
	//
	size_t i, j, N = input.size();
	assert(N >= 3 && N < 10000);

	// use normal of longest side to divide the
	j = (index+1)%N;
	Vector3 longestSide(input[j]._pos - input[index]._pos);
	Vector3 longestSidePerp(-longestSide.z, 0, longestSide.x);

	//TODO: simplify/optimise this line, is too long
	Vector3 longestSideMid = input[index]._pos + (longestSide/2) - (longestSide*(deviance/2)) + 
		(longestSide*(deviance * rg()));


	// fill a vector of intersections, ordered as they're encountered
	vector<Intersection> inscns;
	for(i=0,j=0; i<N; i++)
	{
		j = (i+1) % N;
		Real r;
		Intersection inscn;
		if(Geometry::lineIntersect2D(input[i]._pos, input[j]._pos, longestSideMid, 
			longestSideMid+longestSidePerp, inscn._pos, r, inscn._s) && r>=0 && r<=1)
		{
			inscn._index = inscns.size();
			inscn._boundaryIndex = i;
			inscns.push_back(inscn);
		}
	}
	if(inscns.size() <= 1) 
		return false;	// what only one intersection, how?

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

						//break segment link
						first->_nextIndex = numeric_limits<size_t>::max();

						try
						{
							// add second to boundary
							lotBoundary.push_back(LotBoundaryPoint(
								input[inscns[inscnIndex]._boundaryIndex]._roadAccess, inscns[inscnIndex]._pos)); //error
						}
						catch (Exception* e)
						{
							int z = 0;
						}
						catch(...)
						{
							int z = 1;
						}


						// advance index vars
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

