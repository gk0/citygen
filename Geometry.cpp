#include "stdafx.h"
#include "Geometry.h"
#include "Triangulate.h"

using namespace std;
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

bool Geometry::polygonInset(const vector<Real>& insets, vector<Vector3> &poly)
{
	if(poly.size() < 3) return false;
	SLAV sv(insets, poly);
	poly.clear();
	return Skeletor::processInset(sv, poly);
}

bool Geometry::polygonInset(Real inset, vector<Vector3> &poly)
{
	if(poly.size() < 3) return false;
	SLAV sv(inset, poly);
	poly.clear();
	return Skeletor::processInset(sv, poly);
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
