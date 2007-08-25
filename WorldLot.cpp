#include "stdafx.h"
#include "WorldLot.h"
#include "Triangulate.h"
#include "Geometry.h"
#include "skeleton.h"


using namespace Ogre;
using namespace std;

WorldLot::WorldLot(const LotBoundary &footprint, const CellGenParams &gp, const Real fnd, const Real ht)
{
	// Note: gp._lotSize is desired lot size not actual
	LotBoundary b;
	
	//LotBoundary b(footprint);
	switch(gp._type)
	{
	case 0: //downtown
		b = footprint;
		break;
	case 1:
		b = insetBoundary(footprint, 0.1*gp._lotSize, 0.1*gp._lotSize);
		break;
	case 2:
		b = insetBoundary(footprint, 0.2*gp._lotSize, 0.1*gp._lotSize);
		break;
	default:
		b = footprint;
	}

	_height = ht;
	_foundation = fnd;
	
	_footprint.reserve(b.size());
	BOOST_FOREACH(const LotBoundaryPoint &p, b) _footprint.push_back(p._pos);

	// build cube
	//_error = buildCube(tmp, fnd, ht, m);
	_vertices.reserve(footprint.size()*2);

	//Vector2 bottomLeft(0,0), bottomRight(1,0), topLeft(0,1), topRight(1,1);
	//_texCoords.push_back(bottomLeft);
	//_texCoords.push_back(bottomRight);
	//_texCoords.push_back(topLeft);
	//_texCoords.push_back(topRight);


	//// roof
	if(Triangulate::Process(_footprint, _roofPolys))
	{
		//ok life might be ok

		//load in the roof footprint into vertices
		BOOST_FOREACH(Vector2 &fp, _footprint)
			_vertices.push_back(Vector3(fp.x, ht, fp.y));
	
		//load in the base footprint into vertices
		BOOST_FOREACH(Vector2 &fp, _footprint)
			_vertices.push_back(Vector3(fp.x, fnd, fp.y));

		// add zero texCoord
		_texCoords.push_back(Vector2(0,0));

		// sides
		size_t i, j, N = footprint.size();
		for(i = 0; i < N; i++)
		{
			j = (i + 1) % N;

			// calc texCoords
			Real uMax =	Math::Floor((_footprint[i] - _footprint[j]).length() * 4) / 5;		//horiz
			Real vMax =	Math::Floor((ht - fnd) * 4) / 4;		//vert
			_texCoords.push_back(Vector2(0,vMax));
			_texCoords.push_back(Vector2(uMax,vMax));
			_texCoords.push_back(Vector2(0,0));
			_texCoords.push_back(Vector2(uMax,0));
			size_t texIndex = _texCoords.size() - 4;

			//calc normal
			Ogre::Vector2 perp((_footprint[i] - _footprint[j]).perpendicular());
			Ogre::Vector3 normal(perp.x, 0, perp.y);
			normal.normalise();
			_vertices.push_back(normal);

			_sidePolys.push_back(N+j);			//pos
			_sidePolys.push_back(texIndex+1);	//tex
			_sidePolys.push_back(N+i);
			_sidePolys.push_back(texIndex);
			_sidePolys.push_back(i);
			_sidePolys.push_back(texIndex+2);
			_sidePolys.push_back(_vertices.size()-1);	//norm
			
			_sidePolys.push_back(N+j);
			_sidePolys.push_back(texIndex+1);
			_sidePolys.push_back(i);
			_sidePolys.push_back(texIndex+2);
			_sidePolys.push_back(j);
			_sidePolys.push_back(texIndex+3);
			_sidePolys.push_back(_vertices.size()-1);
		}

		//// add normal for all roof polys
		//_vertices.push_back(Vector3::UNIT_Y);

		//// copy in the roof polys	
		//for(size_t i=0; i<result.size(); i+=3)
		//{
		//	_sidePolys.push_back(result[i]);
		//	_sidePolys.push_back(0);
		//	_sidePolys.push_back(result[i+1]);
		//	_sidePolys.push_back(0);
		//	_sidePolys.push_back(result[i+2]);
		//	_sidePolys.push_back(0);
		//	_sidePolys.push_back(_vertices.size()-1);
		//}

		_error = false;
	}
	else
	{
		_sidePolys.clear();
		_error = true;
	}
}

#define RAND_MAX_HALF RAND_MAX/2

#define RAND_1QT RAND_MAX/4
#define RAND_2QT RAND_MAX/2
#define RAND_3QT RAND_2QT+RAND_1QT

#define RAND_MAX_THRD RAND_MAX/3
#define RAND_MAX_2THRD RAND_MAX_THRD*2

bool WorldLot::build(ManualObject* m)
{
	// WARNING: don't do m->begin m->end too much it makes it *very* slow to render

	size_t i,N=_sidePolys.size();
	for(i=0; i<N; i+=7)
	{
		m->position(_vertices[_sidePolys[i]]);
		m->textureCoord(_texCoords[_sidePolys[i+1]]);
		m->normal(_vertices[_sidePolys[i+6]]);
		m->position(_vertices[_sidePolys[i+2]]);
		m->textureCoord(_texCoords[_sidePolys[i+3]]);
		m->normal(_vertices[_sidePolys[i+6]]);
		m->position(_vertices[_sidePolys[i+4]]);
		m->textureCoord(_texCoords[_sidePolys[i+5]]);
		m->normal(_vertices[_sidePolys[i+6]]);
	}

	//roof
	N=_roofPolys.size();
	for(i=0; i<N; i+=3)
	{
		m->position(_vertices[_roofPolys[i]]);
		m->textureCoord(Vector2::ZERO);
		m->normal(Vector3::UNIT_Y);
		m->position(_vertices[_roofPolys[i+1]]);
		m->textureCoord(Vector2::ZERO);
		m->normal(Vector3::UNIT_Y);
		m->position(_vertices[_roofPolys[i+2]]);
		m->textureCoord(Vector2::ZERO);
		m->normal(Vector3::UNIT_Y);
	}

	return true;
}

void WorldLot::build2(ManualObject* m)
{
	//// the library expects the footprint to be in the xy-plane, but we are
	//// drawing them in the xz-plane so we need to transform the footprint and
	//// rotate the building into position
	//vector<Vector2> lot;
	//int N = _footprint.size();
	//// invert the footprint through the x-axis and reverse the vertex order 
	//// to maintain anti-clockwise ordering
	//for(int i = (N-1); i >= 0; --i)
	//	lot.push_back(Vector2(_footprint[i].x, -_footprint[i].y));
	//Quaternion orient;
	//// rotate the building into the xz-plane
	//orient.FromAngleAxis(Radian(-0.5 * ProArc::PI), Vector3::UNIT_X);
	//// set the buildings y-position
	//Vector3 pos(0.0, (_foundation + (0.5 * _height)), 0.0);
	//// create the building and specify the material to use
	//ProArc::Building b(lot, _height, pos, orient);
	//b.wallMaterial() = "gk/WhiteTile";
	//b.altWallMaterial() = "gk/Brick";
	//b.windowMaterial() = "gk/Window";
	//b.ledgeMaterial() = "gk/Brick2";
	//b.doorMaterial() = "gk/Window";
	//// add the building to the ManualObject
	//b.addTo(m);
	//return;
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