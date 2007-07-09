#include "stdafx.h"
#include "Geometry.h"
#include "skeleton.h"

using namespace Ogre;

Real Geometry::dotPerp(const Vector2& v0, const Vector2& v1)
{
	return (v0.x * v1.y) - (v1.x * v0.y);
}

//http://www.acm.org/pubs/tog/editors/erich/ptinpoly/#ref1
bool Geometry::rayCross(const Vector2& loc, const Vector2& pt1, const Vector2& pt2)
{
	return ((((pt2.y <= loc.y) && (loc.y < pt1.y)) ||
			((pt1.y <= loc.y) && (loc.y < pt2.y))) &&
			(loc.x < (pt1.x - pt2.x) * (loc.y - pt2.y) / (pt1.y - pt2.y) + pt2.x));
}

Real Geometry::polygonArea(const std::vector<Vector2> &polyPoints)
{
	size_t i, j, N = polyPoints.size();
	Real area = 0;

	for (i = 0; i < N; i++)
	{
		j = (i + 1) % N;
		area += polyPoints[i].x * polyPoints[j].y;
		area -= polyPoints[i].y * polyPoints[j].x;
	}
	area /= 2.0;
	return area;
}


Vector2 Geometry::centerOfMass(const std::vector<Vector2> &polyPoints)
{
	Real cx = 0, cy = 0;
	Real area = polygonArea(polyPoints);
	// could change this to Point2D.Float if you want to use less memory
	Vector2 res;
	size_t i, j, N = polyPoints.size();

	Real factor = 0;
	for (i = 0; i < N; i++) {
		j = (i + 1) % N;
		factor = (polyPoints[i].x * polyPoints[j].y
				- polyPoints[j].x * polyPoints[i].y);
		cx += (polyPoints[i].x + polyPoints[j].x) * factor;
		cy += (polyPoints[i].y + polyPoints[j].y) * factor;
	}
	area *= 6.0f;
	factor = 1 / area;
	cx *= factor;
	cy *= factor;
	return Vector2(cx, cy);
}

bool Geometry::isInside(const Ogre::Vector2 &loc, const std::vector<Ogre::Vector2> &polyPoints)
{
	bool bRayCross = false;
	size_t i, j, N = polyPoints.size();
	for(i = 0; i < N; i++)
	{
		j = (i + 1) % N;
		if(rayCross(loc, polyPoints[i], polyPoints[j])) bRayCross = !bRayCross;
	}
	return bRayCross;
}


bool Geometry::polygonInset(Ogre::Real inset, std::vector<Ogre::Vector2> &polyPoints)
{
	vector<Ogre::Vector2> newFootprint, originalFootprint(polyPoints.rbegin(), polyPoints.rend());

	// Skeleton Code
	Felkel::Skeleton s(originalFootprint);
	Felkel::VertexList& vl(s.getVertexList());
	const Felkel::Vertex *vPtr, *firstvPtr = 0, *prevvPtr = 0;


	// for each vertex
	for(Felkel::VertexList::iterator vi = vl.begin(); vi != vl.end (); vi++)
    {
		// if vertex is on boundary
		if((*vi).atContour())
		{
			// get vertex pointer
			vPtr = &(*vi);
			Felkel::SkeletonLine *sl;

			//
			if(prevvPtr)
			{
				Vector2 edge(prevvPtr->point - vPtr->point);
				Vector2 offset = edge.perpendicular();
				offset.normalise();
				offset *= inset;
				Vector2 a, b;
				a = prevvPtr->point + offset;
				b = vPtr->point + offset;

				for(; vPtr->advancingSkeletonLine != 0; vPtr = vPtr->advancingSkeletonLine->higher.vertex)
				{
					sl = vPtr->advancingSkeletonLine;
					Vector2 intersection;
					Ogre::Real r,s;
					if(Geometry::lineIntersect(a, b, sl->lower.vertex->point, sl->higher.vertex->point, intersection, r, s)
						&& s > 0 && s < 1) // must be on skeleton segment, not infinite line
					{
						newFootprint.push_back(intersection);
						break;
					}
				}
				// check that intersection did occur
				//if(vPtr->advancingSkeletonLine == 0) return false;

				// check if intersection occured on first skeleton line
				if(sl != (*vi).advancingSkeletonLine)
				{
					// advance to right most child vertex
					for(; vPtr->rightSkeletonLine != 0; vPtr = vPtr->rightSkeletonLine->lower.vertex);
					for(; vi != vl.end() && vPtr != &(*vi); vi++);
					if(vi == vl.end()) break;
					prevvPtr = &(*vi);
				}
				else
					prevvPtr = &(*vi);
			}
			else
			{
				firstvPtr = prevvPtr = &(*vi);
			}

			//// DEBUG: print skeleton path for boundary vertex
			//stringstream oss;
			//for(; vPtr->advancingSkeletonLine != 0; vPtr = vPtr->advancingSkeletonLine->higher.vertex)
			//{
			//	oss << vPtr->ID << "->";
			//}
			//oss << vPtr->ID;
			//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
		}
	}

	if(prevvPtr && firstvPtr)
	{
		vPtr = firstvPtr;
		Vector2 edge(prevvPtr->point - vPtr->point);
		Vector2 offset = edge.perpendicular();
		offset.normalise();
		offset *= inset;
		Vector2 a, b;
		a = prevvPtr->point + offset;
		b = vPtr->point + offset;

		for(Felkel::SkeletonLine *sl = vPtr->advancingSkeletonLine; vPtr->advancingSkeletonLine != 0; vPtr = vPtr->advancingSkeletonLine->higher.vertex)
		{
			Vector2 intersection;
			Ogre::Real r,s;
			if(Geometry::lineIntersect(a, b, sl->lower.vertex->point, sl->higher.vertex->point, intersection, r, s)
				&& s > 0 && s < 1) // must be on skeleton segment, not infinite line
			{
				newFootprint.push_back(intersection);
				break;
			}
		}

		// check that intersection did occur
		if(vPtr->advancingSkeletonLine == 0) return false;
	}

	polyPoints = newFootprint;
	return true;
}


bool Geometry::lineInset(Ogre::Real inset, std::vector<Ogre::Vector2> &linePoints)
{
	size_t i, N = linePoints.size();
	vector<Vector2> insetLinePoints(N);

	// create footprint edge structure
	vector< pair<Vector2, Vector2> > edges;
	edges.reserve(N);

	// get footprint edge vectors
	for(i=0; i<(N-1); i++)
	{
		Vector2 dir(linePoints[i+1] - linePoints[i]);
		dir = dir.perpendicular();
		dir.normalise();
		dir *= inset;
		edges.push_back(make_pair(linePoints[i] + dir, linePoints[i+1] + dir));
	}

	// calculate footprint points from edges
	insetLinePoints[0] = edges[0].first;		// first point
	insetLinePoints[N-1] = edges[N-2].second;	// last point
	for(i=0; i<(N-2); i++)
	{
		// get edge intersection point
		Ogre::Real r,s;
		if(!Geometry::lineIntersect(edges[i].first, edges[i].second, edges[i+1].first, 
			edges[i+1].second, insetLinePoints[i+1], r, s))
		{
			// if parallel
			if(r==s)
				 insetLinePoints[i+1] = edges[i].second;
			else
				return false;
		}
	}

	linePoints = insetLinePoints;
	return true;
}


bool Geometry::unionPolyAndLine(std::vector<Ogre::Vector2> &polyPoints, std::vector<Ogre::Vector2> &linePoints)
{
	size_t i, j, k, polyN = polyPoints.size(), lineN = linePoints.size();

	size_t polyTailIndex, polyHeadIndex, lineTailIndex, lineHeadIndex;
	Ogre::Vector2 intscn, dist, shortestDist, polyTailIntscn, polyHeadIntscn;
	bool intscnFound = false;
	Ogre::Real r,s;


	//find the location of the line head intersection
	for(i=0; i<(lineN-1); i++)
	{
		for(j=0; j<polyN; j++)
		{
			k = (j+1)%polyN;
			bool intersects = Geometry::lineIntersect(linePoints[i], linePoints[i+1], polyPoints[j], 
				polyPoints[k], intscn, r, s);  // must be on poly segment and be on extension of ba
			
			if(r==s) 
				intscn = linePoints[i];
			if(intersects && s > 0 && s < 1 && r <= 1)
			{
				if(!intscnFound)
				{
					shortestDist = (intscn - linePoints[i]).squaredLength();
					intscnFound = true;
					polyHeadIndex = j;
					polyHeadIntscn = intscn;
				}
				else
				{
					dist = (intscn - linePoints[i]).squaredLength();
					if(dist < shortestDist)
					{
						shortestDist = dist;
						polyHeadIndex = j;
						polyHeadIntscn = intscn;
					}
				}
			}
		}
		if(intscnFound) break;
	}
	if(!intscnFound) return false;
	lineHeadIndex = i;

	//find the location of the line tail intersection
	intscnFound = false;
	for(i=(lineN-2); i<lineN; i--)
	{
		for(j=0; j<polyN; j++)
		{
			k = (j+1)%polyN;
			if(Geometry::lineIntersect(linePoints[i], linePoints[i+1], polyPoints[j], polyPoints[k], intscn, r, s)
				&& s > 0 && s < 1 && r >= 0) // must be on poly segment and be on extension of ab
			{
				if(!intscnFound)
				{
					shortestDist = (intscn - linePoints[i+1]).squaredLength();
					intscnFound = true;
					polyTailIndex = j;
					polyTailIntscn = intscn;
				}
				else
				{
					dist = (intscn - linePoints[i+1]).squaredLength();
					if(dist < shortestDist)
					{
						shortestDist = dist;
						polyTailIndex = j;
						polyTailIntscn = intscn;
					}
				}
			}
		}
		if(intscnFound) break;
	}
	if(!intscnFound) return false;
	lineTailIndex = i;

	// merge the two
	vector<Ogre::Vector2> newPoly;
	newPoly.push_back(polyHeadIntscn);
	for(i=polyHeadIndex; i!=polyTailIndex; i=(i+1)%polyN) 
		newPoly.push_back(polyPoints[(i+1)%polyN]);
	newPoly.push_back(polyTailIntscn);
	for(i=lineTailIndex; i>lineHeadIndex; i--) newPoly.push_back(linePoints[i]);

	polyPoints = newPoly;
	return true;
}

