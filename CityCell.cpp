#include "stdafx.h"
#include "CityCell.h"

using namespace Ogre;

/*
CityCell::CityCell(RoadGraph* parent)
 : mParentRoadGraph(parent),
   mBoundaryPrimitive() 
{
}*/

CityCell::CityCell(RoadGraph* parent, const Primitive &boundary)
 : mParentRoadGraph(parent),
   mBoundaryPrimitive(boundary)
{
	// set up some default growth gen params
	mGrowthGenParams.seed = 0;
	mGrowthGenParams.segmentSize = 5;
	mGrowthGenParams.segmentDeviance = 0.4;
	mGrowthGenParams.degree = 4;
	mGrowthGenParams.degreeDeviance = 0.1;
	mGrowthGenParams.snapSize = 2;
	mGrowthGenParams.snapDeviance = 0.1;
}

Vector2 rotate(Vector2 loc, Vector2 origin, float angle)
{
	//rotate loc around origin
	float sinLoc = sin(angle);
	float cosLoc = cos(angle);
	return Ogre::Vector2(loc.x * cosLoc - loc.y * sinLoc - origin.x * cosLoc + origin.y * sinLoc + origin.x, 
		loc.x * sinLoc + loc.y * cosLoc - origin.x * sinLoc - origin.y * cosLoc + origin.y);
}

Vector2 rotate(Vector2 direction, float angle)
{
	return (Math::Cos(angle) * direction) + (Math::Sin(angle) * direction.perpendicular());
}

void CityCell::growthGenerate(const GrowthGenParams& gp)
{
	mGrowthGenParams = gp;
	growthGenerate();
}

void CityCell::growthGenerate()
{
	mRoadGraph.clear();

	// 1. find the centre point from the boundary
	Vector2 location = mParentRoadGraph->findPrimitiveCenter(mBoundaryPrimitive);

	// 2. find longest edge vector
	Primitive& p(mBoundaryPrimitive);
	Primitive::const_iterator pIt, pIt2, pEnd;
	Ogre::Vector2 direction(0,0);
	Ogre::Real length = 0;
	for(pIt2 = p.begin(), pEnd = p.end(), pIt = pIt2++; pIt2 != pEnd; pIt = pIt2++)
	{
		Vector2 currEdgeVector(mParentRoadGraph->getNodePosition(*pIt) - mParentRoadGraph->getNodePosition(*pIt2));
		Ogre::Real currentLength(currEdgeVector.squaredLength());
		if(currentLength > length) 
		{
			direction = currEdgeVector;
			length = currentLength;
		}
	}
	if(p.begin() != pEnd)
	{
		Vector2 currEdgeVector(mParentRoadGraph->getNodePosition(*(p.begin())) - mParentRoadGraph->getNodePosition(*pIt));
		if(currEdgeVector.squaredLength() > length) direction = currEdgeVector;
	}

	// 3. normalize direction
	direction.normalise();
	direction *= mGrowthGenParams.segmentSize;

	NodeDescriptor locDesc = mRoadGraph.createNode(location);

	// work out how many times to rotate 
	//unsigned int rotateCount = (Math::TWO_PI / theta);
	//unsigned int degree = 4;




	Ogre::Real snapSzSquared = mGrowthGenParams.snapSize * mGrowthGenParams.snapSize;
	//-direction;

	int seed = mGrowthGenParams.seed;

	Ogre::Real segDevSz = mGrowthGenParams.segmentSize * mGrowthGenParams.segmentDeviance;
	Ogre::Real segSzBase = mGrowthGenParams.segmentSize - segDevSz;

	float degDev = mGrowthGenParams.degree * mGrowthGenParams.degreeDeviance;
	int degBase = mGrowthGenParams.degree - degDev;

	std::queue< std::pair< Vector2, NodeDescriptor > > q;
	q.push(std::make_pair(location, locDesc));

	srand(seed++);

	while(!q.empty())
	{
		boost::tie(location, locDesc) = q.front();
		q.pop();



		float theta = Math::TWO_PI / (degBase + (degDev *  ((float)rand()/(float)RAND_MAX)));

		// alter our direction vector 
		for(unsigned int i=0; i < mGrowthGenParams.degree; i++)
		{
			// get a candidate
			direction = rotate(direction, theta);
			direction.normalise();
			//direction *= 
			direction *= (segSzBase + (segDevSz *  ((float)rand()/(float)RAND_MAX)));



			Vector2 cursor(direction + location);

			if(mParentRoadGraph->pointInPolygon(cursor, mBoundaryPrimitive))
			{
				RoadDescriptor rd;
				Vector2 intersection;

				if(mRoadGraph.findClosestIntersection(location, cursor, rd, intersection))
				{
					// if intersection within snap distance of location break; 
					//if((location - intersection).squaredLength() < snapSzSquared) 
					//	break;

					// get the source and target of rd
					NodeDescriptor source, target;
					source = mRoadGraph.getRoadSource(rd);
					target = mRoadGraph.getRoadTarget(rd);

					// try and snap intersection to either source or target of road if possible
					// TODO: this new proposed line could cause another intersection
					// TODO: should prefer source or target on which is closer
					if((mRoadGraph.getNodePosition(source) - intersection).squaredLength() < snapSzSquared)
					{
						mRoadGraph.createRoad(locDesc, source);
					}
					else if((mRoadGraph.getNodePosition(target) - intersection).squaredLength() < snapSzSquared)
					{
						mRoadGraph.createRoad(locDesc, target);
					}
					else
					{
						//i'm not sure about this surely if i'm going to join with some line it'll be the closest there
						//Intersection deserves a change to find the closest node within a range
						//NodeDescriptor closestNode;
						//if(mRoadGraph.findClosestNode(cursor, nd, range))
						//{
						//	mRoadGraph.createRoad(locDesc, closestNode);
						//}
						//else
						{
							// delete the road rd
							mRoadGraph.removeRoad(source, target);

							// create new intersection node
							NodeDescriptor intersectionNode = mRoadGraph.createNode(intersection);

							// reconstruct road in the form of two segments
							mRoadGraph.createRoad(source, intersectionNode);
							mRoadGraph.createRoad(intersectionNode, target);

							// create the new road
							mRoadGraph.createRoad(locDesc, intersectionNode);
						}

					}
				} 
				else 
				{
					// check the cursor to see if it is within 
					// a certain distance of existing nodes
					//TODO: can't snap if an intersection is caused
					Ogre::Real closestDist = snapSzSquared;
					bool snapped = false;
					NodeDescriptor cursorDesc;
					NodeIterator nIt, nEnd;
					for(boost::tie(nIt, nEnd) = mRoadGraph.getNodes(); nIt != nEnd; nIt++)
					{
						Ogre::Vector2 testVect = cursor - mRoadGraph.getNodePosition(*nIt);
						Ogre::Real testDist = testVect.squaredLength();

						if(testDist < closestDist){
							snapped = true;
							closestDist = testDist;
							cursorDesc = *nIt;
						}
					}

					if(!snapped) 
					{
						//Ogre::Real segSz = (location - cursor).length();
						// create the node and road
						cursorDesc = mRoadGraph.createNode(cursor);
						q.push(std::make_pair(cursor, cursorDesc));
					}
					mRoadGraph.createRoad(locDesc, cursorDesc);
				}
			} 
			else
			{
				RoadDescriptor rd;
				Vector2 intersection;
				if(mParentRoadGraph->findClosestIntersection(location, cursor, rd, intersection))
				{
					NodeDescriptor cursorDesc = mRoadGraph.createNode(intersection);
					mRoadGraph.createRoad(locDesc, cursorDesc);
				}
				
				// get the intersection point and use that instead of cursor
				//return;
			}
		}
	}
}
