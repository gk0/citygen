#include "stdafx.h"
#include "Geometry.h"
#include "Triangulate.h"
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
			edges[i+1].second, insetLinePoints[i+1], r, s))
			if(r >= 0 && s <= 1)
				insetLinePoints[i+1] = (edges[i].second + edges[i+1].first) / 2;
	}

	linePoints = insetLinePoints;
	return true;
}


vector< pair<Vector3, Vector2> >  
Geometry::calcInsetVectors(const vector<Real>& insets, vector<Vector3> &poly)
{
	size_t i,j,N = poly.size();
	vector< pair<Vector3, Vector2> > iv;
	iv.reserve(N);

	Vector2 prevPolyi2D(poly[N-1].x, poly[N-1].z);
	Vector2 prevSegInsetVec(calcNormVec2D(poly[N-1], poly[0]).perpendicular());
	Real prevInset = insets[N-1];

	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 polyi2D(poly[i].x, poly[i].z);
		Vector2 polyj2D(poly[j].x, poly[j].z);
		Vector2 segVec(calcNormVec2D(polyi2D, polyj2D));
		Vector2 segInsetVec(segVec.perpendicular());

		if(polyj2D == prevPolyi2D)
		// Back track check for terminal segments
		{
			Vector2 segmentPerpInset(segInsetVec * insets[i]);
			Vector2 segmentDirInset(segVec * -insets[i]);
			iv.push_back(make_pair(poly[i], polyi2D-segmentPerpInset+segmentDirInset));
			iv.push_back(make_pair(poly[i], polyi2D+segmentPerpInset+segmentDirInset));
		}
		else if(insets[i] == prevInset)
		// Bisector method 2, fine for constant inset
		{
			Vector2 bisectorVector2(polyi2D + calcInsetVector(prevSegInsetVec, segInsetVec, insets[i]));
			iv.push_back(make_pair(poly[i], bisectorVector2));
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
				iv.push_back(make_pair(poly[i], polyi2D + bisectorVector3));
			}
			else
				iv.push_back(make_pair(poly[i], bisectorVector3));
		}
		// update prev vars
		prevPolyi2D = polyi2D;
		prevInset = insets[i];
		prevSegInsetVec = segInsetVec;
	}
	return iv;
}


vector< pair<Vector3, Vector2> >  
Geometry::calcInsetVectors(const Real inset, vector<Vector3> &poly)
{
	size_t i,j,N = poly.size();
	vector< pair<Vector3, Vector2> > iv;
	iv.reserve(N);

	Vector2 prevPolyi2D(poly[N-1].x, poly[N-1].z);
	Vector2 prevSegInsetVec(calcNormVec2D(poly[N-1], poly[0]).perpendicular());
	Real prevInset = inset;

	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 polyi2D(poly[i].x, poly[i].z);
		Vector2 polyj2D(poly[j].x, poly[j].z);
		Vector2 segVec(calcNormVec2D(polyi2D, polyj2D));
		Vector2 segInsetVec(segVec.perpendicular());

		if(polyj2D == prevPolyi2D)
			// Back track check for terminal segments
		{
			Vector2 segmentPerpInset(segInsetVec * inset);
			Vector2 segmentDirInset(segVec * -inset);
			iv.push_back(make_pair(poly[i], polyi2D-segmentPerpInset+segmentDirInset));
			iv.push_back(make_pair(poly[i], polyi2D+segmentPerpInset+segmentDirInset));
		}
		else if(inset == prevInset)
			// Bisector method 2, fine for constant inset
		{
			Vector2 bisectorVector2(polyi2D + calcInsetVector(prevSegInsetVec, segInsetVec, inset));
			iv.push_back(make_pair(poly[i], bisectorVector2));
		}
		else
			// Bisector method 3
		{
			Vector2 prevSegmentInset(prevSegInsetVec * prevInset);
			Vector2 segmentInset(segInsetVec * inset);

			Vector2 bisectorVector3;
			if(!Geometry::lineIntersect(prevPolyi2D + prevSegmentInset, polyi2D+prevSegmentInset, 
				polyi2D+segmentInset, polyj2D+segmentInset, bisectorVector3))
			{
				// parallel so intersection point is the minimum inset perpendicular
				bisectorVector3 = prevInset < inset ?  prevSegInsetVec : segInsetVec;
				iv.push_back(make_pair(poly[i], polyi2D + bisectorVector3));
			}
			else
				iv.push_back(make_pair(poly[i], bisectorVector3));
		}
		// update prev vars
		prevPolyi2D = polyi2D;
		prevInset = inset;
		prevSegInsetVec = segInsetVec;
	}
	return iv;
}

void Geometry::processInsetVectors(const vector< pair<Vector3, Vector2> > &iv, vector<Vector3> &poly)
{
	size_t i, j, offset = 0, N=iv.size();
	if(poly.size() != 0) poly.clear();
	poly.reserve(N);
	
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		size_t k = (i+2)%N;
		Vector2 polyi2D(iv[i].first.x, iv[i].first.z);
		Vector2 polyj2D(iv[j].first.x, iv[j].first.z);
		Vector2 polyk2D(iv[k].first.x, iv[k].first.z);

		// check if the bisectors intersect with their immediate neighbors
		Vector2 interscn;
		if(Geometry::lineSegmentIntersect(polyi2D, iv[i].second, polyk2D, iv[k].second, interscn))
		{
			size_t l = (i+2)%N;
			Vector2 polyl2D(iv[l].first.x, iv[l].first.z);
			if(Geometry::lineSegmentIntersect(polyi2D, iv[i].second, polyl2D, iv[l].second, interscn))
			{
				Vector2 tmp((iv[i].second+iv[l].second)/2);
				poly.push_back(Vector3(tmp.x, (iv[i].first.y + iv[l].first.y)/2, tmp.y));
				if(i>=(N-3))
					offset = N - i + 1;
				i += 3;
			}
			else
			{
				Vector2 tmp((iv[i].second+iv[k].second)/2);
				poly.push_back(Vector3(tmp.x, (iv[i].first.y + iv[k].first.y)/2, tmp.y));
				if(i>=(N-2))
					offset = N - i + 1;
				i += 2;
			}
		}
		else if(Geometry::lineSegmentIntersect(polyi2D, iv[i].second, polyj2D, iv[j].second, interscn))
		{
			if(Geometry::lineSegmentIntersect(polyj2D, iv[j].second, polyk2D, iv[k].second, interscn))
			{
				Vector2 tmp((iv[j].second+iv[k].second)/2);
				poly.push_back(Vector3(tmp.x, (iv[j].first.y + iv[k].first.y)/2, tmp.y));
				if(i>=(N-2)) 
					offset = N - i + 1;
				i+=2;
			}
			else
			{
				Vector2 tmp((iv[i].second+iv[j].second)/2);
				poly.push_back(Vector3(tmp.x, (iv[i].first.y + iv[j].first.y)/2, tmp.y));
				if(i==(N-1)) 
					offset = 1;
				i++;
			}
		}
		else
		{
			// pretend its ok, could be intersecting with other less immediate neighbors
			poly.push_back(Vector3(iv[i].second.x, iv[i].first.y, iv[i].second.y));
		}
	}

	if(!offset)
	{
		vector<Vector3> tmpPoly;
		tmpPoly.reserve(N - offset);
		for(size_t i=offset; i<poly.size(); i++) tmpPoly.push_back(poly[i]);
		poly.swap(tmpPoly);
	}
}

void Geometry::polygonInset(const vector<Real>& insets, vector<Vector3> &poly)
{
	if(poly.size() < 3) return;
	// get the inset vectors
	vector< pair<Vector3, Vector2> > iv(calcInsetVectors(insets, poly));
	// process them for anomalies
	processInsetVectors(iv, poly);
}

bool Geometry::polygonInsetFast(Real inset, vector<Vector3> &poly)
{
	if(poly.size() < 3) return false;
	// get the inset vectors
	//vector< pair<Vector3, Vector2> > iv(calcInsetVectors(inset, poly));
	// process them for anomalies
	//processInsetVectors(iv, poly);
	// 1. Prepare initial inset data structures
	///////////////////////////////////////////////////////////////////////
	list<InsetVertex> ivList;
	size_t i,j,N = poly.size();
	size_t lastI = N-1;
	for(i=0; i<N; i++)
	{
		j = (i+1) % N;
		InsetVertex insetVx;
		insetVx._pos = poly[i];
		insetVx._insetTarget = Geometry::calcInsetTarget(poly[lastI], poly[i], 
				poly[j], inset, inset);
		insetVx._inset = inset;
		insetVx._intersectionTested = false;
		ivList.insert(ivList.end(), insetVx);

		lastI = i;
	}
	poly.clear();
	processInset(ivList, poly);

	return true;
}


void Geometry::polygonInsetFFast(Real inset, vector<Vector3> &poly)
{
	size_t i,j,N = poly.size();
	if(N < 3) return;
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
		Vector2 insetVec(calcInsetVector(prevSegInsetVec, segInsetVec, inset));
		poly[i].x += insetVec.x;
		poly[i].z += insetVec.y;
		
		// update prev vars
		prevPolyi2D = polyi2D;
		prevSegInsetVec = segInsetVec;
	}
}

bool Geometry::polyRepair(std::vector<Ogre::Vector3> &poly, size_t lookAheadMax)
{
	size_t lookAhead,N = poly.size();
	if(N < 3) return false;
	bool repaired = false;
	lookAheadMax = std::min(N-3, lookAheadMax);
	for(lookAhead=1; lookAhead<=lookAheadMax && !repaired; lookAhead++)
	{
		Geometry::polyRepairCycle(poly, lookAhead);
		if(poly.size() != N)
		{
			N = poly.size();
			lookAheadMax = std::min(N-3, lookAheadMax);
			vector<size_t> tmp; 
			if(Triangulate::Process(poly, tmp))
			{
				//LogManager::getSingleton().logMessage("Poly Repaired: "+StringConverter::toString(lookAhead));
				repaired = true;
			}
			//else
			//	LogManager::getSingleton().logMessage("Poly Attempted Repair: "+StringConverter::toString(lookAhead));
		}
	}
	return repaired;
}

void Geometry::polyRepairCycle(std::vector<Ogre::Vector3> &poly, size_t lookAhead)
{
	size_t i,j,k,l,clipFront=0,N = poly.size();

#ifdef DEBUG
	if(N >= 3)
	{
		throw Ogre::Exception(Ogre::Exception::ERR_INTERNAL_ERROR, "Not a valid polygon", "Geometry::polyRepairCycle");
		return;
	}
	if((N-lookAhead) > 2)
	{
		throw Ogre::Exception(Ogre::Exception::ERR_INTERNAL_ERROR, "Look ahead too high", "Geometry::polyRepairCycle");
		return;
	}
#else
	if(N >= 3 && (N-lookAhead) > 2) return;
#endif
	vector<Vector3> tmpPoly;
	tmpPoly.reserve(N);

	Vector2 prevPolyi2D(poly[N-1].x, poly[N-1].z);
	Vector2 prevSegInsetVec(calcNormVec2D(poly[N-1], poly[0]).perpendicular());
	
	for(j=0; j<N;)
	{
		i = (j+(N-1))%N;
		k = (j+lookAhead)%N;
		l = (k+1)%N;

		// compare i-->j segment with segment k-->l
		Vector2 inscn;
		Real r,s;
		if(lineIntersect(V2(poly[i]),V2(poly[j]),V2(poly[k]),V2(poly[l]),inscn,r,s)
		&& r >= 0 && r <= 1 && s >= 0 && s <= 1)
		{
			Real height = poly[i].y + r*(poly[j].y - poly[i].y);
			tmpPoly.push_back(Vector3(inscn.x, height, inscn.y));

			// gotta check this is right
			if(j >= (N-lookAhead))
				clipFront = lookAhead - (N-j) + 1;

			j += (lookAhead + 1); 
		}
		else
		{
			tmpPoly.push_back(poly[j]);
			j++;
		}
	}
	if(clipFront != 0)
	{
		poly.clear();
		poly.reserve(N-lookAhead);
		for(size_t i=clipFront; i<tmpPoly.size(); i++) poly.push_back(tmpPoly[i]);
	}
	else poly.swap(tmpPoly);
}




vector< pair<Vector2, Vector2> >  
Geometry::calcInsetVectors(const vector<Real>& insets, vector<Vector2> &poly)
{
	size_t i,j,N = poly.size();
	vector< pair<Vector2, Vector2> > iv;
	iv.reserve(N);

	Vector2 prevPolyi2D(poly[N-1]);
	Vector2 prevSegInsetVec(calcNormVec2D(poly[N-1], poly[0]).perpendicular());
	Real prevInset = insets[N-1];

	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 polyi2D(poly[i]);
		Vector2 polyj2D(poly[j]);
		Vector2 segVec(calcNormVec2D(polyi2D, polyj2D));
		Vector2 segInsetVec(segVec.perpendicular());

		if(polyj2D == prevPolyi2D)
			// Back track check for terminal segments
		{
			Vector2 segmentPerpInset(segInsetVec * insets[i]);
			Vector2 segmentDirInset(segVec * -insets[i]);
			iv.push_back(make_pair(polyi2D, polyi2D-segmentPerpInset+segmentDirInset));
			iv.push_back(make_pair(polyi2D, polyi2D+segmentPerpInset+segmentDirInset));
		}
		else if(insets[i] == prevInset)
			// Bisector method 2, fine for constant inset
		{
			Vector2 bisectorVector2(polyi2D + calcInsetVector(prevSegInsetVec, segInsetVec, insets[i]));
			iv.push_back(make_pair(polyi2D, bisectorVector2));
		}
		else
			// Bisector method 3
		{
			Vector2 prevSegmentInset(prevSegInsetVec * prevInset);
			Vector2 segmentInset(segInsetVec * insets[i]);

			Vector2 bisectorVector3;
			if(!Geometry::lineIntersect(prevPolyi2D + prevSegmentInset, polyi2D+prevSegmentInset, 
				polyi2D+segmentInset, polyj2D+segmentInset, bisectorVector3))
			{
				// parallel so intersection point is the minimum inset perpendicular
				bisectorVector3 = prevInset < insets[i] ?  prevSegInsetVec : segInsetVec;
				iv.push_back(make_pair(polyi2D, polyi2D + bisectorVector3));
			}
			else
				iv.push_back(make_pair(polyi2D, bisectorVector3));
		}
		// update prev vars
		prevPolyi2D = polyi2D;
		prevInset = insets[i];
		prevSegInsetVec = segInsetVec;
	}
	return iv;
}

void Geometry::processInsetVectors(const vector< pair<Vector2, Vector2> > &iv, vector<Vector2> &poly)
{
	size_t i, j, offset = 0, N=iv.size();
	if(poly.size() != 0) poly.clear();
	poly.reserve(N);

	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		size_t k = (i+2)%N;
		Vector2 polyi2D(iv[i].first);
		Vector2 polyj2D(iv[j].first);
		Vector2 polyk2D(iv[k].first);

		// check if the bisectors intersect with their immediate neighbors
		Vector2 interscn;
		if(Geometry::lineSegmentIntersect(polyi2D, iv[i].second, polyk2D, iv[k].second, interscn))
		{
			size_t l = (i+2)%N;
			Vector2 polyl2D(iv[l].first);
			if(Geometry::lineSegmentIntersect(polyi2D, iv[i].second, polyl2D, iv[l].second, interscn))
			{
				Vector2 tmp((iv[i].second+iv[l].second)/2);
				poly.push_back(tmp);
				if(i>=(N-3))
					offset = N - i + 1;
				i += 3;
			}
			else
			{
				Vector2 tmp((iv[i].second+iv[k].second)/2);
				poly.push_back(tmp);
				if(i>=(N-2))
					offset = N - i + 1;
				i += 2;
			}
		}
		else if(Geometry::lineSegmentIntersect(polyi2D, iv[i].second, polyj2D, iv[j].second, interscn))
		{
			if(Geometry::lineSegmentIntersect(polyj2D, iv[j].second, polyk2D, iv[k].second, interscn))
			{
				Vector2 tmp((iv[j].second+iv[k].second)/2);
				poly.push_back(tmp);
				if(i>=(N-2)) 
					offset = N - i + 1;
				i+=2;
			}
			else
			{
				Vector2 tmp((iv[i].second+iv[j].second)/2);
				poly.push_back(tmp);
				if(i==(N-1)) 
					offset = 1;
				i++;
			}
		}
		else
		{
			// pretend its ok, could be intersecting with other less immediate neighbors
			poly.push_back(iv[i].second);
		}
	}

	if(!offset)
	{
		vector<Vector2> tmpPoly;
		tmpPoly.reserve(N - offset);
		for(size_t i=offset; i<poly.size(); i++) tmpPoly.push_back(poly[i]);
		poly.swap(tmpPoly);
	}
}


void Geometry::polygonInset(const vector<Real>& insets, vector<Vector2> &poly)
{
	// get the inset vectors
	vector< pair<Vector2, Vector2> > iv(calcInsetVectors(insets, poly));
	// process them for anomalies
	processInsetVectors(iv, poly);
}


void Geometry::processInset(list<InsetVertex> &ivList, vector<Vector3> &poly)
{
	//TODO: Use a slav and a queue instead of the list
	//queue<InsetVertex*> ivQueue;
	//BOOST_FOREACH(InsetVertex& iv, ivList) ivQueue.push(&iv);

	while(true)
	{
		///////////////////////////////////////////////////////////////////
		// 1. Intersection test for all inset vectors
		///////////////////////////////////////////////////////////////////
		
		// find the earliest intersection
		Real intersectionLocation = 1;
		Vector2 intersection;
		list<InsetVertex>::iterator firstOffender, secondOffender, bisecOffender;

		for(list<InsetVertex>::iterator ivIt = ivList.begin(); ivIt != ivList.end(); ivIt++)
		{
			if(ivIt->_intersectionTested) continue;
			list<InsetVertex>::iterator nextIt = ivIt;
			if(++nextIt == ivList.end()) nextIt = ivList.begin();
			Vector2 ivItPos2D(ivIt->_pos.x, ivIt->_pos.z);

			// check if the pair intersect and store the lowest
			Real r,s;
			Vector2 tmpInscn, nextItPos2D(nextIt->_pos.x, nextIt->_pos.z);
			if(Geometry::lineIntersect(ivItPos2D, ivIt->_insetTarget, 
					nextItPos2D, nextIt->_insetTarget, tmpInscn, r, s) &&
					r >= 0 && r <= 1 && s >= 0 && s <= 1)
			{
				// TODO: tolerance value could be used here
				if(r < intersectionLocation) 
				{
					intersectionLocation = r;
					firstOffender = ivIt;
					secondOffender = nextIt;
					intersection = tmpInscn;
				}
			}
			else
				ivIt->_intersectionTested = true;
		}
		
		///////////////////////////////////////////////////////////////////
		// 2. Process Bisector Intersection
		///////////////////////////////////////////////////////////////////
		
		// find the closest intersection
		if(intersectionLocation != 1.0)
		{
			list<InsetVertex>::iterator ivIt;

			// remove the first offender
			ivList.erase(firstOffender);

			// if there is a valid polygon remaining
			if(ivList.size() >= 3)
			{
				// update the pos and inset of the remaining vertices
				for(ivIt = ivList.begin(); ivIt != ivList.end(); ivIt++)
				{
					ivIt->_pos.x = ivIt->_pos.x + intersectionLocation * (ivIt->_insetTarget.x - ivIt->_pos.x);
					ivIt->_pos.z = ivIt->_pos.z + intersectionLocation * (ivIt->_insetTarget.y - ivIt->_pos.z);
					ivIt->_inset = ivIt->_inset * (1 - intersectionLocation);
				}
				
				// update the second offender
				list<InsetVertex>::iterator prev(secondOffender), next(secondOffender);
				if(prev == ivList.begin()) prev = ivList.end();
				prev--;
				next++;
				if(next == ivList.end()) next = ivList.begin();
				
				//LogManager::getSingleton().logMessage("Int:"+StringConverter::toString(intersection));
				secondOffender->_pos.x = intersection.x;
				secondOffender->_pos.z = intersection.y;
				secondOffender->_insetTarget = Geometry::calcInsetTarget(prev->_pos, secondOffender->_pos, 
						next->_pos, prev->_inset, secondOffender->_inset);

				secondOffender->_intersectionTested = false;
				prev->_intersectionTested = false;
			}
			else
				LogManager::getSingleton().logMessage("Less than 3 vertices after collapse.");
		}
		else
		{
			//LogManager::getSingleton().logMessage("Valid.");
			poly.reserve(ivList.size());
			for(list<InsetVertex>::iterator ivIt = ivList.begin(); ivIt != ivList.end(); ivIt++)
				poly.push_back(Vector3(ivIt->_insetTarget.x, ivIt->_pos.y, ivIt->_insetTarget.y));
			break;
		}
	}
}