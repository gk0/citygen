#include "stdafx.h"
#include "WorldLot.h"
#include "Triangulate.h"
#include "Geometry.h"
#include "skeleton.h"

using namespace Ogre;
using namespace std;

bool WorldLot::build(const LotBoundary &footprint, const GrowthGenParams &gp,
	const Real fnd, const Real ht, ManualObject* m)
{
	// Note: gp.lotSize is desired lot size not actual
	LotBoundary b;
	
	//LotBoundary b(footprint);
	switch(gp.type)
	{
	case 0: //downtown
		b = footprint;
		break;
	case 1:
		b = insetBoundary(footprint, 0.1*gp.lotSize, 0.1*gp.lotSize);
		break;
	case 2:
		b = insetBoundary(footprint, 0.4*gp.lotSize, 0.1*gp.lotSize);
		break;
	default:
		b = footprint;
	}

	std::vector<Ogre::Vector2> tmp;
	tmp.reserve(b.size());
	BOOST_FOREACH(const LotBoundaryPoint &p, b) tmp.push_back(p._pos);

	// build cube
	return buildCube(tmp, fnd, ht, m);
}


void outFace(Felkel::Vertex &v, vector<Vector3> &poly, Real h)
{
	Felkel::SkeletonLine *l = v.advancingSkeletonLine;
	if (l == NULL) { cerr << "Integrity error !!!\n"; return; }
	Felkel::SkeletonLine *next = l -> higher.right;
	Ogre::Vector2 last = l -> lower.vertex -> point;

	//stream << last.x << ',' << last.y << ' ';
	poly.push_back(Vector3(last.x, h, last.y));
	do {
		if(!equ(last, l -> lower.vertex -> point))
		{
			last = l -> lower.vertex -> point;
			//stream << last.x << ',' << last.y << ' ';
			poly.push_back(Vector3(last.x, h, last.y));
		}
		else if(!equ(last, l -> higher.vertex -> point))
		{
			last = l -> higher.vertex -> point;
			//stream << last.x << ',' << last.y << ' ';
			poly.push_back(Vector3(last.x, h+0.3, last.y));
		}
		if (next == NULL) break;
		if (next -> lower.left == l)
		{ 
			l = next; 
			next = next -> higher.right;
		}
		else if (next -> higher.left == l) 
		{ 
			l = next; 
			next = next -> lower.right; 
		}
		else assert (false);
	} while (l);
}


bool WorldLot::buildHousey(const std::vector<Ogre::Vector2> &footprint, 
	const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m)
{
	std::vector<Ogre::Vector2> result;

	// roof
	if(Triangulate::Process(footprint, result))
	{
		// sides
		size_t i, j, N = footprint.size();
		for(i = 0; i < N; i++)
		{
			j = (i + 1) % N;
			Ogre::Vector2 perp((footprint[i] - footprint[j]).perpendicular());
			Ogre::Vector3 normal(perp.x, 0, perp.y);
			normal.normalise();
			m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, foundation, footprint[i].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);

			m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[j].x, height, footprint[j].y));
			m->normal(normal);

			// inner side DEBUG
			m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);	
			m->position(Ogre::Vector3(footprint[i].x, foundation, footprint[i].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);

			m->position(Ogre::Vector3(footprint[j].x, height, footprint[j].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
		}

		// lets try an sk roof instead
		//std::vector<Ogre::Vector2> result2(footprint.begin(), footprint.end());
		std::vector<Ogre::Vector2> result2(footprint.rbegin(), footprint.rend());
		Felkel::Skeleton s(result2);

		// for each face
		Felkel::VertexList& vl(s.getVertexList());
		for(Felkel::VertexList::iterator vi = vl.begin(); vi != vl.end (); vi++)
		{
			if ((*vi).atContour())
			{
				vector<Vector3> poly;
				outFace (*vi, poly, height);

					if(poly.size() == 3)
				{
					//Ogre::EdgeListBuilder elb;
					//elb.addVertexData(&poly[0]);
					m->position(poly[2]);
					m->position(poly[1]);
					m->position(poly[0]);
				}
				else if(poly.size() == 4)
				{
					m->position(poly[3]);
					m->position(poly[2]);
					m->position(poly[0]);

					m->position(poly[2]);
					m->position(poly[1]);
					m->position(poly[0]);
				}
				else
					Ogre::LogManager::getSingleton().logMessage("got"+StringConverter::toString(poly.size())+" sides");
				//if(poly.size() == 3)
				//{
				//	//Ogre::EdgeListBuilder elb;
				//	//elb.addVertexData(&poly[0]);
				//	m->position(Ogre::Vector3(poly[2].x, height, poly[2].z));
				//	m->position(Ogre::Vector3(poly[1].x, height+0.2, poly[1].z));
				//	m->position(Ogre::Vector3(poly[0].x, height, poly[0].z));
				//}
				//else if(poly.size() == 4)
				//{
				//	m->position(Ogre::Vector3(poly[3].x, height, poly[3].z));
				//	m->position(Ogre::Vector3(poly[2].x, height+0.2, poly[2].z));
				//	m->position(Ogre::Vector3(poly[0].x, height, poly[0].z));

				//	m->position(Ogre::Vector3(poly[2].x, height+0.2, poly[2].z));
				//	m->position(Ogre::Vector3(poly[1].x, height+0.2, poly[1].z));
				//	m->position(Ogre::Vector3(poly[0].x, height, poly[0].z));
				//}
				//else
				//	Ogre::LogManager::getSingleton().logMessage("got"+StringConverter::toString(poly.size())+" sides");
			}
		}
/*
		for(size_t i=0; i<result.size(); i+=3)
		{
			m->position(Ogre::Vector3(result[i+2].x, height, result[i+2].y));
			m->normal(Ogre::Vector3::UNIT_Y);
			m->position(Ogre::Vector3(result[i+1].x, height, result[i+1].y));
			m->normal(Ogre::Vector3::UNIT_Y);
			m->position(Ogre::Vector3(result[i].x, height, result[i].y));
			m->normal(Ogre::Vector3::UNIT_Y);
		}
*/
		return true;
	}
	else
	{
		//Ogre::LogManager::getSingleton().logMessage("WorldLot::build() failed.");
		return false;
	}
}


bool WorldLot::buildCube(const std::vector<Ogre::Vector2> &footprint, 
	const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m)
{
	std::vector<Ogre::Vector2> result;

	// roof
	if(Triangulate::Process(footprint, result))
	{
		// sides
		size_t i, j, N = footprint.size();
		for(i = 0; i < N; i++)
		{
			j = (i + 1) % N;
			Ogre::Vector2 perp((footprint[i] - footprint[j]).perpendicular());
			Ogre::Vector3 normal(perp.x, 0, perp.y);
			normal.normalise();
			m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, foundation, footprint[i].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);

			m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);
			m->position(Ogre::Vector3(footprint[j].x, height, footprint[j].y));
			m->normal(normal);
		}

		for(size_t i=0; i<result.size(); i+=3)
		{
			m->position(Ogre::Vector3(result[i+2].x, height, result[i+2].y));
			m->normal(Ogre::Vector3::UNIT_Y);
			m->position(Ogre::Vector3(result[i+1].x, height, result[i+1].y));
			m->normal(Ogre::Vector3::UNIT_Y);
			m->position(Ogre::Vector3(result[i].x, height, result[i].y));
			m->normal(Ogre::Vector3::UNIT_Y);
		}
		return true;
	}
	else
	{
		//Ogre::LogManager::getSingleton().logMessage("WorldLot::build() failed.");
		return false;
	}
}


LotBoundary WorldLot::insetBoundary(const LotBoundary &b, const Real &roadInset, const Real &standardInset)
{
	// get size
	size_t i, j, N = b.size();

	// check cycle
	if(N < 3)
		throw Exception(Exception::ERR_INVALIDPARAMS, 
		"Invalid number of nodes in cycle", "WorldLot::extractFootprint");

	// create footprint edge structure
	vector< pair<Vector2, Vector2> > edges;
	edges.reserve(N);
	LotBoundary newBoundary;
	newBoundary.reserve(N);

	// get footprint edge vectors
	for(i=0; i<N; i++)
	{
		j = (i+1)%N;
		Vector2 dir(b[j]._pos - b[i]._pos);
		dir = dir.perpendicular();
		dir.normalise();
		dir *= b[i]._roadAccess ? roadInset : standardInset;
		edges.push_back(make_pair(b[i]._pos + dir, b[j]._pos + dir));
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
			newBoundary.push_back(LotBoundaryPoint(b[j]._roadAccess, intscn));
		}
		else
		{
			// no intersection, couldbe parallel could be mad lets average the pair
			newBoundary.push_back(LotBoundaryPoint(b[j]._roadAccess, (edges[i].second + edges[j].first)/2));
		}
	}

	return newBoundary;
}