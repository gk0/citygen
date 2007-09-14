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


bool Geometry::polygonInsetFast(Ogre::Real inset, std::vector<Ogre::Vector2> &polyPoints)
{
	// get size
	size_t i, j, N = polyPoints.size();

	// check cycle
	if(N < 3)
		throw Exception(Exception::ERR_INVALIDPARAMS, 
		"Invalid number of points in polygon", "Geometry::polygonInsetFast");

	// create footprint edge structure
	vector< pair<Vector2, Vector2> > edges;
	edges.reserve(N);
	vector<Vector2> newFootprint;
	newFootprint.reserve(N);

	// get footprint edge vectors
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 dir(polyPoints[j] - polyPoints[i]);
		dir = dir.perpendicular();
		dir.normalise();
		dir *= inset;
		edges.push_back(make_pair(polyPoints[i] + dir, polyPoints[j] + dir));
	}

	// calculate footprint points from edges
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		// get edge intersection point
		Ogre::Real r,s;
		Vector2 intscn;
		if(Geometry::lineIntersect(edges[i].first, edges[i].second, 
			edges[j].first, edges[j].second, intscn, r, s) && r >= 0 && s <= 1)
		{
			newFootprint.push_back(intscn);
		}
		else
		{
			// no intersection, could be parallel could be mad lets average the pair
			newFootprint.push_back((edges[i].second + edges[j].first)/2);
		}
	}
	polyPoints.swap(newFootprint);
	return true;
}

/*
bool Geometry::polygonInsetFast(Ogre::Real inset, std::vector<Ogre::Vector3> &polyPoints)
{
	// get size
	size_t i, j, N = polyPoints.size();

	// negate inset
	inset = -inset;

	// check cycle
	if(N < 3)
		throw Exception(Exception::ERR_INVALIDPARAMS, 
		"Invalid number of points in polygon", "Geometry::polygonInsetFast");

	// create footprint edge structure
	vector< pair<Vector2, Vector2> > edges;
	edges.reserve(N);
	vector<Vector2> newFootprint;
	newFootprint.reserve(N);

	// get footprint edge vectors
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 dir(polyPoints[i].x - polyPoints[j].x, polyPoints[i].z - polyPoints[j].z);
		dir = dir.perpendicular();
		dir.normalise();
		dir *= inset;
		edges.push_back(make_pair(
			Vector2(polyPoints[i].x + dir.x, polyPoints[i].z + dir.y), 
			Vector2(polyPoints[j].x + dir.x, polyPoints[j].z + dir.y)));
	}

	// calculate footprint points from edges
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		// get edge intersection point
		Ogre::Real r,s;
		Vector2 intscn;
		if(Geometry::lineIntersect(edges[i].first, edges[i].second, 
			edges[j].first, edges[j].second, intscn, r, s) && r >= 0 && s <= 1)
		{
			newFootprint.push_back(intscn);
		}
		else
		{
			// no intersection, could be parallel could be mad lets average the pair
			newFootprint.push_back((edges[i].second + edges[j].first)/2);
		}
	}

	assert(newFootprint.size() == N);
	for(size_t i=0; i<N; i++)
	{
		j = (i+1)%N;
		polyPoints[j].x = newFootprint[i].x;
		polyPoints[j].z = newFootprint[i].y;
	}
	return true;
}*/

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
			Felkel::SkeletonLine *sl=0;

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
				if(sl && sl != (*vi).advancingSkeletonLine)
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
			edges[i+1].second, insetLinePoints[i+1], r, s) && r >= 0 && s <= 1)
		{
			insetLinePoints[i+1] = (edges[i].second + edges[i+1].first) / 2;
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
			if(Geometry::lineIntersect(linePoints[i], linePoints[i+1], polyPoints[j], 
				polyPoints[k], intscn, r, s) && s > 0 && s < 1 && r <= 1)  // must be on poly segment and be on extension of ba
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

void Geometry::polygonInset(const vector<Real>& insets, vector<Vector3> &poly)
{
	size_t i,j,N = poly.size();
	vector<Vector3> tmpPoly;
	tmpPoly.reserve(N);
	vector<Vector2> bisectorTargets(N);

	Vector2 prevPolyi2D(poly[N-1].x, poly[N-1].z);
	Vector2 prevSegInsetVec(calcNormVec2D(poly[N-1], poly[0]).perpendicular());
	Real prevInset = insets[N-1];

	for(i=0; i<N; i++)
	{
		j = (i+1)%N;

		Vector2 polyi2D(poly[i].x, poly[i].z);
		Vector2 polyj2D(poly[j].x, poly[j].z);

		// Segment vectors
		Vector2 segInsetVec = calcNormVec2D(polyi2D, polyj2D).perpendicular();

		// mmm which is quicker

		if(insets[i] == prevInset)
		// Bisector method 2, fine for constant inset
		{
			bisectorTargets[i] = (polyi2D + calcInsetVector(prevSegInsetVec, segInsetVec, insets[i]));
		}
		else
		// Bisector method 3
		{
			Vector2 prevSegmentInset(prevSegInsetVec * prevInset);
			Vector2 segmentInset(segInsetVec *insets[i]);

			Vector2 bisectorVector3;
			if(!Geometry::lineIntersect(prevPolyi2D + prevSegmentInset, polyi2D+prevSegmentInset, 
				polyi2D+segmentInset, polyj2D+segmentInset, bisectorVector3))
			{
				// parallel so intersection point is the minimum inset perpendicular
				bisectorVector3 = prevInset < insets[i] ?  prevSegInsetVec : segInsetVec;
				bisectorTargets[i] = (polyi2D + bisectorVector3);
			}
			else
				bisectorTargets[i] = bisectorVector3;
		}

		// update prev vars
		prevPolyi2D = polyi2D;
		prevInset = insets[i];
		prevSegInsetVec = segInsetVec;
	}

	//for(size_t i=0; i<N; i++) tmpPoly.push_back(Vector3(bisectorTargets[i].x, poly[i].y, bisectorTargets[i].y));
	//poly.swap(tmpPoly);
	//return;

	// processing 2nd round
	size_t offset = 0;
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		size_t k = (i+2)%N;
		Vector2 polyi2D(poly[i].x, poly[i].z);
		Vector2 polyj2D(poly[j].x, poly[j].z);
		Vector2 polyk2D(poly[k].x, poly[k].z);

		// check if the bisectors intersect with their immediate neighbors
		Vector2 interscn;
		if(Geometry::lineSegmentIntersect(polyi2D, bisectorTargets[i], polyk2D, bisectorTargets[k], interscn))
		{
			size_t l = (i+2)%N;
			Vector2 polyl2D(poly[l].x, poly[l].z);
			if(Geometry::lineSegmentIntersect(polyi2D, bisectorTargets[i], polyl2D, bisectorTargets[l], interscn))
			{
				Vector2 tmp((bisectorTargets[i]+bisectorTargets[l])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[i].y + poly[l].y)/2, tmp.y));
				if(i>=(N-3))
					offset = N - i + 1;
				i += 3;
			}
			else
			{
				Vector2 tmp((bisectorTargets[i]+bisectorTargets[k])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[i].y + poly[k].y)/2, tmp.y));
				if(i>=(N-2))
					offset = N - i + 1;
				i += 2;
			}
		}
		else if(Geometry::lineSegmentIntersect(polyi2D, bisectorTargets[i], polyj2D, bisectorTargets[j], interscn))
		{
			if(Geometry::lineSegmentIntersect(polyj2D, bisectorTargets[j], polyk2D, bisectorTargets[k], interscn))
			{
				Vector2 tmp((bisectorTargets[j]+bisectorTargets[k])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[j].y + poly[k].y)/2, tmp.y));
				if(i>=(N-2)) 
					offset = N - i + 1;
				i+=2;
			}
			else
			{
				Vector2 tmp((bisectorTargets[i]+bisectorTargets[j])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[i].y + poly[j].y)/2, tmp.y));
				if(i==(N-1)) 
					offset = 1;
				i++;
			}
		}
		else
		{
			// pretend its ok, could be intersecting with other less immediate neighbors
			tmpPoly.push_back(Vector3(bisectorTargets[i].x, poly[i].y, bisectorTargets[i].y));
		}
	}

	if(!offset)
		poly.swap(tmpPoly);
	else
	{
		N = tmpPoly.size();
		poly.clear();
		for(i=offset; i<N; i++) poly.push_back(tmpPoly[i]);
	}
}




bool Geometry::polygonInsetFast(Real inset, vector<Vector3> &poly)
{
	size_t i,j,N = poly.size();
	if(N < 3) return false;
	vector<Vector3> tmpPoly;
	tmpPoly.reserve(N);
	vector<Vector2> bisectorTargets(N);

	Vector2 prevPolyi2D(poly[N-1].x, poly[N-1].z);
	Vector2 prevSegInsetVec(calcNormVec2D(poly[N-1], poly[0]).perpendicular());

	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 polyi2D(poly[i].x, poly[i].z);
		Vector2 polyj2D(poly[j].x, poly[j].z);

		// Segment inset vectors
		Vector2 segInsetVec = calcNormVec2D(polyi2D, polyj2D).perpendicular();

		// Bisector method 2, fine for constant inset
		bisectorTargets[i] = (polyi2D + calcInsetVector(prevSegInsetVec, segInsetVec, inset));
		
		// update prev vars
		prevPolyi2D = polyi2D;
		prevSegInsetVec = segInsetVec;
	}

	//for(size_t i=0; i<N; i++) tmpPoly.push_back(Vector3(bisectorTargets[i].x, poly[i].y, bisectorTargets[i].y));
	//poly.swap(tmpPoly);
	//return;
	// processing 2nd round
	size_t offset = 0;
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		size_t k = (i+2)%N;
		Vector2 polyi2D(poly[i].x, poly[i].z);
		Vector2 polyj2D(poly[j].x, poly[j].z);
		Vector2 polyk2D(poly[k].x, poly[k].z);
		
		// check if the bisectors intersect with their immediate neighbors
		Vector2 interscn;
		if(Geometry::lineSegmentIntersect(polyi2D, bisectorTargets[i], polyk2D, bisectorTargets[k], interscn))
		{
			size_t l = (i+2)%N;
			Vector2 polyl2D(poly[l].x, poly[l].z);
			if(Geometry::lineSegmentIntersect(polyi2D, bisectorTargets[i], polyl2D, bisectorTargets[l], interscn))
			{
				Vector2 tmp((bisectorTargets[i]+bisectorTargets[l])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[i].y + poly[l].y)/2, tmp.y));
				if(i>=(N-3))
					offset = N - i + 1;
				i += 3;
			}
			else
			{
				Vector2 tmp((bisectorTargets[i]+bisectorTargets[k])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[i].y + poly[k].y)/2, tmp.y));
				if(i>=(N-2))
					offset = N - i + 1;
				i += 2;
			}
		}
		else if(Geometry::lineSegmentIntersect(polyi2D, bisectorTargets[i], polyj2D, bisectorTargets[j], interscn))
		{
			if(Geometry::lineSegmentIntersect(polyj2D, bisectorTargets[j], polyk2D, bisectorTargets[k], interscn))
			{
				Vector2 tmp((bisectorTargets[j]+bisectorTargets[k])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[j].y + poly[k].y)/2, tmp.y));
				if(i>=(N-2)) 
					offset = N - i + 1;
				i+=2;
			}
			else
			{
				Vector2 tmp((bisectorTargets[i]+bisectorTargets[j])/2);
				tmpPoly.push_back(Vector3(tmp.x, (poly[i].y + poly[j].y)/2, tmp.y));
				if(i==(N-1)) 
					offset = 1;
				i++;
			}
		}
		else
		{
			// pretend its ok, could be intersecting with other less immediate neighbors
			tmpPoly.push_back(Vector3(bisectorTargets[i].x, poly[i].y, bisectorTargets[i].y));
		}
	}

	if(!offset)
		poly.swap(tmpPoly);
	else
	{
		N = tmpPoly.size();
		poly.clear();
		for(i=offset; i<N; i++) poly.push_back(tmpPoly[i]);
	}
	return true;
}


//
//// draw inset vectors
//vector< vector<NodeInterface*> > cycs;
//_roadGraph.extractFootprints(cycs, _genParams._lotSize);
//
//_debugMO = new ManualObject(_name+"do");
//_debugMO->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
//
//BOOST_FOREACH(vector<NodeInterface*> &cycle, cycs)
//{
//	size_t i,j,k,N = cycle.size();
//	vector<Vector2> bisectorTargets(N);
//
//	for(i=0; i<N; i++)
//	{
//		j = (i+1)%N;
//		k = (i+2)%N;
//		Vector3& iPos(cycle[i]->getPosition3D());
//		Vector3& jPos(cycle[j]->getPosition3D());
//		Vector3& kPos(cycle[k]->getPosition3D());
//		Vector2 iPos2(iPos.x, iPos.z);
//		Vector2 jPos2(jPos.x, jPos.z);
//		Vector2 kPos2(kPos.x, kPos.z);
//		//_debugMO->position(footprints[0][i]);
//		//_debugMO->position(footprints[0][j]);
//
//		// Segment Vectors
//		Vector2 segmentVec1(jPos2 - iPos2);
//		Vector2 segmentVec2(kPos2 - jPos2);
//		segmentVec1.normalise();
//		segmentVec2.normalise();
//
//		// Perpendiculars
//		Vector2 perp1 = segmentVec1.perpendicular();
//		perp1.normalise();
//		Vector2 perp2 = segmentVec2.perpendicular();
//		perp2.normalise();
//
//		// Segment Widths
//		Real rw1 = _roadGraph.getRoad(_roadGraph.getRoad(cycle[i]->_nodeId, cycle[j]->_nodeId))->getWidth();
//		Real rw2 = _roadGraph.getRoad(_roadGraph.getRoad(cycle[j]->_nodeId, cycle[k]->_nodeId))->getWidth();
//
//		// Segment inset vectors
//		Vector2 segmentInsetVector1 = perp1 * rw1;
//		Vector2 segmentInsetVector2 = perp2 * rw2;
//
//		_debugMO->position(iPos.x, iPos.y+0.3, iPos.z);
//		_debugMO->position(iPos.x+perp1.x, iPos.y+0.3, iPos.z+segmentInsetVector1.y);
//		_debugMO->position(jPos.x, jPos.y+0.3, jPos.z);
//		_debugMO->position(jPos.x+perp1.x, jPos.y+0.3, jPos.z+segmentInsetVector1.y);
//
//		// Bisector method 1
//		//Vector2 bisectorVector1(segmentVec1 - segmentVec2);		// method 1
//		//if(bisectorVector1 == Vector2::ZERO)					// doesn't always point inside
//		//	bisectorVector1 = perp1;
//		//else
//		//	bisectorVector1.normalise();
//
//		//// Bisector method 2
//		//{
//		//	Vector2 bisectorVector2(perp1 + perp2);					// method 2
//		//	bisectorVector2.normalise();							// always points inside, fine for constant inset
//		//	Vector2 tmp2 = bisectorVector2*4;
//		//	_debugMO->position(jPos.x, jPos.y+0.3, jPos.z);
//		//	_debugMO->position(jPos.x+tmp2.x, jPos.y+0.3, jPos.z+tmp2.y);
//		//}
//
//		// Bisector method 3
//		{
//			Vector2 bisectorVector3;
//			if(!Geometry::lineIntersect(iPos2 + segmentInsetVector1, jPos2+segmentInsetVector1, 
//				jPos2+segmentInsetVector2, kPos2+segmentInsetVector2, bisectorVector3))
//				// parallel so intersection point is the minimum inset perpendicular
//				bisectorVector3 = rw1 < rw2 ?  segmentInsetVector1 : segmentInsetVector2;
//			else
//				bisectorVector3 -= jPos2;
//
//			Vector2 tmp2 = bisectorVector3*5;
//			_debugMO->position(jPos.x, jPos.y+0.3, jPos.z);
//			_debugMO->position(jPos.x+tmp2.x, jPos.y+0.3, jPos.z+tmp2.y);
//
//			bisectorTargets[j] = (jPos2 + bisectorVector3);
//		}
//	}
//
//	vector<Vector3> boundary;
//
//	// processing 2nd round
//	for(i=0; i<N; i++)
//	{
//		j = (i+1)%N;
//		k = (i+2)%N;
//		Vector3& iPos(cycle[i]->getPosition3D());
//		Vector3& jPos(cycle[j]->getPosition3D());
//		Vector3& kPos(cycle[k]->getPosition3D());
//		Vector2 iPos2(iPos.x, iPos.z);
//		Vector2 jPos2(jPos.x, jPos.z);
//		Vector2 kPos2(kPos.x, kPos.z);
//
//		// check if the bisectors intersect with their immediate neighbors
//		Vector2 interscn;
//		if(Geometry::lineSegmentIntersect(iPos2, bisectorTargets[i], kPos2, bisectorTargets[k], interscn))
//		{
//			//boundary.push_back(Vector3(interscn.x, (iPos.y + kPos.y)/2, interscn.y));
//			Vector2 tmp((bisectorTargets[i]+bisectorTargets[k])/2);
//			boundary.push_back(Vector3(tmp.x, (iPos.y + kPos.y)/2, tmp.y));
//		}
//		else if(Geometry::lineSegmentIntersect(iPos2, bisectorTargets[i], jPos2, bisectorTargets[j], interscn))
//		{
//			//boundary.push_back(Vector3(interscn.x, (iPos.y + jPos.y)/2, interscn.y));
//			Vector2 tmp((bisectorTargets[i]+bisectorTargets[j])/2);
//			boundary.push_back(Vector3(tmp.x, (iPos.y + jPos.y)/2, tmp.y));
//		}
//		else
//		{
//			// pretend its ok, could be intersecting with other less immediate neighbors
//			boundary.push_back(Vector3(bisectorTargets[i].x, iPos.y, bisectorTargets[i].y));
//		}
//	}
//
//	N = boundary.size();
//	for(i=0; i<N; i++)
//	{
//		j = (i+1)%N;
//		_debugMO->position(boundary[i]);
//		_debugMO->position(boundary[j]);
//	}
//}
//}

