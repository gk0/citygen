#include "stdafx.h"
#include "Geometry.h"

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


bool Geometry::polygonInset(Ogre::Real inset, std::vector<Ogre::Vector2> &polyPoints)
{
	std::vector< std::pair<Vector2, Vector2> > edges;
	edges.reserve(polyPoints.size());
	Radian rad90(Math::PI / 2);

	size_t i, j, N = polyPoints.size();
	for(i = 0; i < N; i++)
	{
		j = (i + 1) % N;
		Vector2 dir(polyPoints[j] - polyPoints[i]);
		//dir *= Quaternion(

		//Vector2 perp(dir.perpendicular());
		//rotate(dir, rad90);
		dir = dir.perpendicular();
		//perp.normalise();
		dir.normalise();
		dir *= inset;
		//perp *= inset;
		edges.push_back(std::make_pair(polyPoints[i] + dir, polyPoints[j] + dir));
	}

	std::vector< Ogre::Vector2 > results(N);
	for(i = 0; i < N; i++)
	{
		j = (i + 1) % N;
		bool intersects = lineIntersect(edges[i].first, edges[i].second, 
			edges[j].first, edges[j].second, results[i]);
		if(!intersects) 
			return false;

		if(!isInside(results[i], polyPoints))
			return false;
	}
	polyPoints = results;
	return true;
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
