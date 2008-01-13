#include "stdafx.h"
#include "WorldBlock.h"
#include "Geometry.h"
#include "CellParams.h"
#include "WorldFrame.h"

#include <OgreManualObject.h>

using namespace Ogre;
using namespace std;

//THIS CLASS IS FUCKING HORRIBLE
// BUT ITS GETTING BETTER

WorldBlock::WorldBlock(const vector<Vector3> &boundary, const CellParams &gp, rando rg, 
	MeshBuilder* mb, vector<Material*> &materials, bool debug)
{
	// 
	size_t i,j,N = boundary.size(), N2 = N * 2;
	vector<Vector3> innerBoundary;
	innerBoundary.reserve(N);
	vector<Vector3> outerBoundary(boundary);

	_footpathVertexData.reserve(N * 8 * 8);
	_footpathPolys.reserve(N * 4 * 3);

	//TODO:
	//mat->getTechnique()->getPass()->getTextureUnitState()->getTextureUScale();

	// raise footpath
	// footpath does not support inset properly
	// different number of vertices is possible
	for(i=0; i<N; i++) outerBoundary[i].y += gp._footpathHeight;
	innerBoundary.insert(innerBoundary.end(), outerBoundary.begin(), outerBoundary.end());

	if(Geometry::polygonInsetFast(gp._footpathWidth, innerBoundary))
	{
		// if the inner boundary has a different number of vertices
		if(innerBoundary.size() != N)
		{
			// try and create an outer boundary with the same number of vertices
			N = innerBoundary.size();
			N2 = N * 2;
			vector<Vector3> tmp;
			tmp.insert(tmp.end(), innerBoundary.begin(), innerBoundary.end());
			if(!Geometry::polygonInsetFast(-gp._footpathWidth, tmp)) return;
			if(tmp.size() != N)
			{
				//LogManager::getSingleton().logMessage("Outset Fail!!");
				return;
			}
			outerBoundary = tmp;
		}

		Real uMinInside = 0, uMinOutside = 0, uMaxInside = 0, uMaxOutside = 0;
		bool texCoordsDirection = false;
		for(i=0; i<N; i++)
		{
			j = (i+1)%N;

			// get normal for side
			Vector3 innerB(innerBoundary[i]-innerBoundary[j]);
			Vector3 outerB(outerBoundary[i]-outerBoundary[j]);
			Vector3 normal(innerB.perpendicular().normalisedCopy());

			// we need to offset uMinInside to keep set the texture straight
			Vector2 adjacent(innerBoundary[j].x - innerBoundary[i].x, innerBoundary[j].z- innerBoundary[i].z);
			Vector2 hypotenuse(outerBoundary[i].x - innerBoundary[i].x, outerBoundary[i].z- innerBoundary[i].z);
			adjacent.normalise();
			Real coso = Math::Abs(adjacent.dotProduct(hypotenuse.normalisedCopy()));

			// reverse the direction of the texture coordinates for 
			// each boundary segment, so that the mitered edges match
			if(texCoordsDirection)
			{
				uMinInside = uMinOutside - hypotenuse.length() * coso;
				uMaxInside = uMinInside - innerB.length();
				uMaxOutside = uMinOutside - outerB.length();
			}
			else
			{
				uMinInside = uMinOutside + hypotenuse.length() * coso;
				uMaxInside = uMinInside + innerB.length();
				uMaxOutside = uMinOutside + outerB.length();
			}
			texCoordsDirection = !texCoordsDirection;

			//build sides
			uint16 offset = static_cast<uint16>(_footpathVertexData.size()/8);
			appendVector3(_footpathVertexData, outerBoundary[i]);
			appendVector3(_footpathVertexData, normal);
			appendVector2(_footpathVertexData, uMinOutside, gp._footpathHeight);

			appendVector3(_footpathVertexData, outerBoundary[j]);
			appendVector3(_footpathVertexData, normal);
			appendVector2(_footpathVertexData, uMaxOutside, gp._footpathHeight);

			appendVector3(_footpathVertexData, outerBoundary[i].x, outerBoundary[i].y - gp._footpathHeight, outerBoundary[i].z);
			appendVector3(_footpathVertexData, normal);
			appendVector2(_footpathVertexData, uMinOutside, 0);

			appendVector3(_footpathVertexData, outerBoundary[j].x, outerBoundary[j].y - gp._footpathHeight, outerBoundary[j].z);
			appendVector3(_footpathVertexData, normal);
			appendVector2(_footpathVertexData, uMaxOutside, 0);

			appendPoly(_footpathPolys, offset, offset + 1, offset + 2);
			appendPoly(_footpathPolys, offset + 2, offset + 1, offset+3);

			// build top
			offset = static_cast<uint16>(_footpathVertexData.size()/8);
			appendVector3(_footpathVertexData, innerBoundary[i]);
			appendVector3(_footpathVertexData, 0.0f, 1.0f, 0.0f);
			appendVector2(_footpathVertexData, uMinInside, 0);

			appendVector3(_footpathVertexData, innerBoundary[j]);
			appendVector3(_footpathVertexData, 0.0f, 1.0f, 0.0f);
			appendVector2(_footpathVertexData, uMaxInside, 0);

			appendVector3(_footpathVertexData, outerBoundary[i]);
			appendVector3(_footpathVertexData, 0.0f, 1.0f, 0.0f);
			appendVector2(_footpathVertexData, uMinOutside, gp._footpathWidth);

			appendVector3(_footpathVertexData, outerBoundary[j]);
			appendVector3(_footpathVertexData, 0.0f, 1.0f, 0.0f);
			appendVector2(_footpathVertexData, uMaxOutside, gp._footpathWidth);

			appendPoly(_footpathPolys, offset, offset + 1, offset + 2);
			appendPoly(_footpathPolys, offset + 2, offset + 1, offset+3);
			/*
			// TODO: could use two vertices less here
			uint16 zeebo = innerBoundary.size() * 6;
			appendPoly(_footpathPolys, offset, (zeebo + offset - 6)%zeebo, offset + 1);
			appendPoly(_footpathPolys, offset + 1, (zeebo + offset - 6)%zeebo, (zeebo + offset - 5)%zeebo);
			*/

			// update outside texture coordinates
			uMinOutside = uMaxOutside;
		}
	}

	vector<uint16> tmp;
	if(!Triangulate::Process(innerBoundary, tmp))
	{
		//LogManager::getSingleton().logMessage("Boundary Fail!!"+StringConverter::toString(N));
		if(!Geometry::polyRepair(innerBoundary, 100))
		{
			_footpathVertexData.clear();
			_footpathPolys.clear();
			return;
		}
		else
		{
			mb->registerData(materials[materials.size()-1], _footpathVertexData, _footpathPolys);
			N = innerBoundary.size();
			N2 = N * 2;
		}
	}else
		mb->registerData(materials[materials.size()-1], _footpathVertexData, _footpathPolys);

	// Do something to extract lots
	queue< LotBoundary > q;

	// the bool stores is the side is adjacent a road
	LotBoundary blockBoundary;
	vector< LotBoundary > lotBoundaries;
	BOOST_FOREACH(const Vector3 &b, innerBoundary) blockBoundary.push_back(LotBoundaryPoint(true, b));
	q.push(blockBoundary);
	Real lotSplitWidth2Sq = Math::Sqr(2*gp._lotWidth);
	Real lotSplitDepth2Sq = Math::Sqr(2*gp._lotDepth);

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
		if(!getLongestBoundarySideIndex(b, lotSplitWidth2Sq, lsIndex)
			&& !getLongestNonBoundarySideIndex(b, lotSplitDepth2Sq, lsIndex))
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

	// get material scales for lots
	vector< pair<Real, Real> > materialScales(materials.size());
	for(size_t i=0; i<materials.size(); i++)
	{
		bool out = false;
		Real uScale = 1.0f, vScale = 1.0f;
		for(unsigned short j=0; !out && j<materials[i]->getNumTechniques(); j++)
		{
			for(unsigned short k=0; !out && k<materials[i]->getTechnique(j)->getNumPasses(); k++)
			{
				for(unsigned short l=0; !out && l<materials[i]->getTechnique(j)->getPass(k)->getNumTextureUnitStates(); l++)
				{
					TextureUnitState* tus = materials[i]->getTechnique(j)->getPass(k)->getTextureUnitState(l);
					if(tus->getTextureUScale() != 1.0f || tus->getTextureVScale() != 1.0f)
					{
						uScale = tus->getTextureUScale();
						vScale = tus->getTextureVScale();
						out = true;
					}
				}
			}
		}
		//materials[i]->getTechnique(0)->getPass(0)->getTextureUnitState(0);
		materialScales[i].first = uScale;
		materialScales[i].second = vScale;
	}

	size_t fail = 0;
	_lots.reserve(lotBoundaries.size());
	BOOST_FOREACH(LotBoundary &b, lotBoundaries)
	{
		Real buildingHeight = gp._buildingHeight + buildingDeviance;
		size_t matIndex = static_cast<size_t>(Math::Floor(rg() * (materials.size()-1)));
		WorldLot* lot = new WorldLot(b, gp, buildingHeight + (buildingDeviance * rg()), 
			materialScales[matIndex].first, materialScales[matIndex].second);
		if(lot->hasError())
		{
			fail++;
			delete lot;
		}
		else
		{	
			mb->registerData(materials[matIndex], lot->getVertexData(), lot->getIndexData());
			_lots.push_back(lot);
			// lets assign the lot to a pointer array
			// here for building material type
		}
	}

	// also calculate vertex & index counts for all material types
	// store in public vars

	/*
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
	*/

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

bool WorldBlock::getLongestBoundarySideIndex(const LotBoundary &b, const Real limitSq, size_t &index)
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
	return false;
}

bool WorldBlock::getLongestNonBoundarySideIndex(const LotBoundary &b, const Real limitSq, size_t &index)
{
	// get size and check it is a poly
	size_t N = b.size();
	assert(N >= 3);

	// declare vars and set initial longest to first segment
	Ogre::Real currLSq, lsLSq = 0;
	size_t lsIndex = 0;

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

						// add second to boundary
						lotBoundary.push_back(LotBoundaryPoint(
							input[inscns[inscnIndex]._boundaryIndex]._roadAccess, inscns[inscnIndex]._pos)); //error

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

