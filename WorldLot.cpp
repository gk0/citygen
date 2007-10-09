#include "stdafx.h"
#include "WorldLot.h"
#include "Triangulate.h"
#include "Geometry.h"
#include "skeleton.h"
#include "Statistics.h"

using namespace Ogre;
using namespace std;

WorldLot::WorldLot(const LotBoundary &footprint, const CellGenParams &gp, const Real ht)
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

	// find the minimum y value for foundation
	_foundation = numeric_limits<Real>::max();
	BOOST_FOREACH(const LotBoundaryPoint &bp, footprint)
		if(bp._pos.y < _foundation) _foundation = bp._pos.y;

	// set height relative to foundation
	_height = _foundation + ht;
	
	// get a copy of the footprint
	vector<Vector3> footprint2;
	footprint2.reserve(b.size());
	BOOST_FOREACH(const LotBoundaryPoint &p, b) footprint2.push_back(p._pos);

	if(Triangulate::Process(footprint2, _indexData))
	{
		// reserve data for vertices & polygons
		_vertexData.reserve(footprint2.size() * 5 * 8);
		_indexData.reserve(_indexData.size() + (footprint2.size() * 6));

		// load in the roof footprint into vertex data
		BOOST_FOREACH(Vector3 &fp, footprint2)
		{
			appendVector3(_vertexData, fp.x, _height, fp.z);	// position
			appendVector3(_vertexData, 0.0f, 1.0f, 0.0f);		// normal
			appendVector2(_vertexData, 0.0f, 0.0f);				// texture coordinates
		}

		// process the sides
		size_t i,j,N = footprint2.size();
		for(i=0; i<N; i++)
		{
			j = (i+1)%N;

			// calculate texCoords
			Real uMax =	Math::Floor((footprint2[i] - footprint2[j]).length() * 4) / 5;
			Real vMax =	Math::Floor((_height - _foundation) * 4) / 4;

			//calculate normal
			Ogre::Vector3 normal(-footprint2[i].z + footprint2[j].z, 0, footprint2[i].x - footprint2[j].x);
			normal.normalise();

			// add vertex data with normals and texture coordinates
			// v1.
			appendVector3(_vertexData, footprint2[i].x, _foundation, footprint2[i].z);
			appendVector3(_vertexData, normal);
			appendVector2(_vertexData, 0.0f, 0.0f);
			// v2.
			appendVector3(_vertexData, footprint2[j].x, _foundation, footprint2[j].z);
			appendVector3(_vertexData, normal);
			appendVector2(_vertexData, uMax, 0.0f);
			// v3.
			appendVector3(_vertexData, footprint2[i].x, _height, footprint2[i].z);
			appendVector3(_vertexData, normal);
			appendVector2(_vertexData, 0.0f, vMax);
			// v4.
			appendVector3(_vertexData, footprint2[j].x, _height, footprint2[j].z);
			appendVector3(_vertexData, normal);
			appendVector2(_vertexData, uMax, vMax);

			// polygons
			uint16 vertexPos = static_cast<uint16>(N + (i*4));
			appendPoly(_indexData, vertexPos+3, vertexPos+1, vertexPos);		//v4,v2,v1
			appendPoly(_indexData, vertexPos, vertexPos+2, vertexPos+3);		//v1,v3,v4
		}
		Statistics::incBuildingCount();
		_error = false;
	}
	else
	{
		_vertexData.clear();
		_indexData.clear();
		_error = true;
	}
}

LotBoundary WorldLot::insetBoundary(const LotBoundary &b, const Real &roadInset, const Real &standardInset)
{
	LotBoundary boundary(b);
	vector<Vector3> poly;
	vector<Real> insets;
	poly.reserve(b.size());
	insets.reserve(b.size());
	BOOST_FOREACH(const LotBoundaryPoint& p, b)
	{
		poly.push_back(p._pos);
		insets.push_back(p._roadAccess ? roadInset : standardInset);
	}

	Geometry::polygonInset(insets, poly);
	LotBoundary newBoundary;
	newBoundary.reserve(poly.size());
	BOOST_FOREACH(Vector3& p, poly) newBoundary.push_back(LotBoundaryPoint(true, p));
	return newBoundary;
}
/*
void WorldLot::installVertexData(float*& vPtr)
{
	memcpy(vPtr, _vertexData, _vertexDataSz * sizeof(Real));
	vPtr = vPtr + _vertexDataSz;
}

void WorldLot::installIndexData(Ogre::uint16*& iPtr, size_t& offset)
{
	memcpy(iPtr, _polyData, _polyDataSz * sizeof(uint16));
	iPtr = iPtr + _polyDataSz;
	//size_t i,N=_sidePolys.size();
	//for(i=0; i<N; i+=3)
	//{
	//	*iPtr++ = offset + _sidePolys[i];//(offset);
	//	*iPtr++ = offset + _sidePolys[i+1];//(offset+1);
	//	*iPtr++ = offset + _sidePolys[i+2];
	//}

	////roof
	//N=_roofPolys.size();
	//for(i=0; i<N; i+=3)
	//{
	//	*iPtr++ = offset + _roofPolys[i];//(offset);
	//	*iPtr++ = offset + _roofPolys[i+1];//(offset+1);
	//	*iPtr++ = offset + _roofPolys[i+2];
	//}
	//offset += getVertexCount();
}
*/