#include "stdafx.h"
#include "WorldBlock.h"
#include "Geometry.h"
#include "CellParams.h"
#include "WorldFrame.h"
#include "WorldLot.h"
#include "MeshBuilder.h"
#include "WorldMaterials.h"
#include "Region.h"

#include <OgreManualObject.h>

using namespace Ogre;
using namespace std;
using namespace citygen;

WorldBlock::WorldBlock(const vector<Vector3> &boundary, const CellParams &gp, rando rg,
	MeshBuilder* mb, bool debug)
{
	//BOOST_FOREACH(Vector3 v, boundary)
	//{
	//	if(_isnan(v.x) || _isnan(v.y) || _isnan(v.z))
	//		throw Exception(Exception::ERR_INVALIDPARAMS, "Nan", "WorldBlock");
	//}


	//
	_error = false;
	size_t i,j,N = boundary.size(), N2 = N * 2;
	vector<Vector3> innerBoundary;
	innerBoundary.reserve(N);
	vector<Vector3> outerBoundary(boundary);

	_pavementVertexData.reserve(N * 8 * 8);
	_pavementPolys.reserve(N * 4 * 3);

	//TODO:
	//mat->getTechnique()->getPass()->getTextureUnitState()->getTextureUScale();

	// raise pavement
	// pavement does not support inset properly
	// different number of vertices is possible
	for(i=0; i<N; i++) outerBoundary[i].y += gp._pavementHeight;
	innerBoundary.insert(innerBoundary.end(), outerBoundary.begin(), outerBoundary.end());

	if(Geometry::polygonInset(gp._pavementWidth, innerBoundary))
	{
		// if the inner boundary has a different number of vertices
		if(innerBoundary.size() != N)
		{
			// try and create an outer boundary with the same number of vertices
			N = innerBoundary.size();
			N2 = N * 2;
			vector<Vector3> tmp;
			tmp.insert(tmp.end(), innerBoundary.begin(), innerBoundary.end());
			if(!Geometry::polygonInset(-gp._pavementWidth, tmp)) return;
			if(tmp.size() != N)
			{
				//LogManager::getSingleton().logMessage("Outset Fail!!");
				_error = true;
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
         uint16 offset = static_cast<uint16>(_pavementVertexData.size()/8);
         MeshBuilder::addVData3(_pavementVertexData, outerBoundary[i]);
         MeshBuilder::addVData3(_pavementVertexData, normal);
         MeshBuilder::addVData2(_pavementVertexData, uMinOutside, gp._pavementHeight);

         MeshBuilder::addVData3(_pavementVertexData, outerBoundary[j]);
         MeshBuilder::addVData3(_pavementVertexData, normal);
         MeshBuilder::addVData2(_pavementVertexData, uMaxOutside, gp._pavementHeight);

         MeshBuilder::addVData3(_pavementVertexData, outerBoundary[i].x, outerBoundary[i].y - gp._pavementHeight, outerBoundary[i].z);
         MeshBuilder::addVData3(_pavementVertexData, normal);
         MeshBuilder::addVData2(_pavementVertexData, uMinOutside, 0);

         MeshBuilder::addVData3(_pavementVertexData, outerBoundary[j].x, outerBoundary[j].y - gp._pavementHeight, outerBoundary[j].z);
         MeshBuilder::addVData3(_pavementVertexData, normal);
         MeshBuilder::addVData2(_pavementVertexData, uMaxOutside, 0);

         MeshBuilder::addIData3(_pavementPolys, offset, offset + 1, offset + 2);
         MeshBuilder::addIData3(_pavementPolys, offset + 2, offset + 1, offset+3);

         // build top
         offset = static_cast<uint16>(_pavementVertexData.size()/8);
         MeshBuilder::addVData3(_pavementVertexData, innerBoundary[i]);
         MeshBuilder::addVData3(_pavementVertexData, 0.0f, 1.0f, 0.0f);
         MeshBuilder::addVData2(_pavementVertexData, uMinInside, 0);

         MeshBuilder::addVData3(_pavementVertexData, innerBoundary[j]);
         MeshBuilder::addVData3(_pavementVertexData, 0.0f, 1.0f, 0.0f);
         MeshBuilder::addVData2(_pavementVertexData, uMaxInside, 0);

         MeshBuilder::addVData3(_pavementVertexData, outerBoundary[i]);
         MeshBuilder::addVData3(_pavementVertexData, 0.0f, 1.0f, 0.0f);
         MeshBuilder::addVData2(_pavementVertexData, uMinOutside, gp._pavementWidth);

         MeshBuilder::addVData3(_pavementVertexData, outerBoundary[j]);
         MeshBuilder::addVData3(_pavementVertexData, 0.0f, 1.0f, 0.0f);
         MeshBuilder::addVData2(_pavementVertexData, uMaxOutside, gp._pavementWidth);

         MeshBuilder::addIData3(_pavementPolys, offset, offset + 1, offset + 2);
         MeshBuilder::addIData3(_pavementPolys, offset + 2, offset + 1, offset+3);
			/*
			// TODO: could use two vertices less here
			uint16 zeebo = innerBoundary.size() * 6;
			appendPoly(_pavementPolys, offset, (zeebo + offset - 6)%zeebo, offset + 1);
			appendPoly(_pavementPolys, offset + 1, (zeebo + offset - 6)%zeebo, (zeebo + offset - 5)%zeebo);
			*/

			// update outside texture coordinates
			uMinOutside = uMaxOutside;
		}
	}

   Material* _pavementMaterial(WorldMaterials::getSingleton().getDefaultMaterial("pavement"));
	vector<uint16> tmp;
	if(!Triangulate::Process(innerBoundary, tmp))
	{
		//LogManager::getSingleton().logMessage("Boundary Fail!!"+StringConverter::toString(N));
		if(!Geometry::polyRepair(innerBoundary, 100))
		{
			_pavementVertexData.clear();
			_pavementPolys.clear();
			_error = true;
			return;
		}
		else
		{
			LogManager::getSingleton().logMessage("Polygon Repaired!!");
			mb->registerData(_pavementMaterial, _pavementVertexData, _pavementPolys);
			N = innerBoundary.size();
			N2 = N * 2;
		}
	}else
		mb->registerData(_pavementMaterial, _pavementVertexData, _pavementPolys);

   //vector< vector<Vector3> > outputPolys;
   //vector< vector<bool> > outputIsExteriors;
   //subdivide(innerBoundary, gp, outputPolys, outputIsExteriors, debug, _debugLots, rg);
	//list<Vector3> innerBound(innerBoundary.begin(), innerBoundary.end());

	/*vector<Vector3> testo;
	testo.push_back(Vector3(0,0,0));
	testo.push_back(Vector3(200,0,0));
	testo.push_back(Vector3(200,0,100));
	testo.push_back(Vector3(0,0,100));

	Vector3 p1(100, 0, 0);
	Vector3 p2(100, 0, 100);*/

	/*if(gp._roadLimit != 0)
	{
		BOOST_FOREACH(Vector3 point, innerBoundary)
		{
			LogManager::getSingleton().logMessage(
				"points.Add(new Vector2(" +
				StringConverter::toString(point.x) +
				", "+StringConverter::toString(point.z) +
				");",LML_CRITICAL);
		}
	}
	LogManager::getSingleton().logMessage(" ");*/

	//WorldFrame::getSingleton().cpf2.pause();
	Region* innerRegion = new Region(innerBoundary);
	//delete innerRegion;
	innerBoundary.clear();
	innerBoundary.resize(0);
	//innerBoundary.trim();

	//WorldFrame::getSingleton().cpf2.resume();
	list<Region*> lotRegions = subdivide(innerRegion, gp, rg);

	//LogManager::getSingleton().logMessage("Subdivided into " + StringConverter::toString(lotRegions.size()));

	//BOOST_FOREACH(Region *r, lotRegions)
	//	delete r;

	//Region* innerRegion = new Region(testo);
	//list<Region*> lotRegions = Region::SplitRegion(innerRegion, p1, p2);

	vector<Vector3> poly;
	vector<bool> isExterior;
	poly.reserve(64);
	isExterior.reserve(64);

	BOOST_FOREACH(Region *r, lotRegions)
	{
		//WorldFrame::getSingleton().cpf2.pause();
		poly.clear();
		isExterior.clear();
		DirectedEdge* curr = r->first();
		do
		{
			poly.push_back(curr->srcPos());
			isExterior.push_back(curr->_exterior);
			curr = curr->next();
		} while (curr != r->first());

		delete r;
		//WorldFrame::getSingleton().cpf2.resume();

		WorldLot* lot = new WorldLot(poly, isExterior, gp, rg);
		if(lot->hasError())
		{
			//fail++;
			delete lot;
		}
		else
		{
			_lots.push_back(lot);
			lot->registerData(*mb);
		}
	}

/*
	//_lots = lotBoundaries;
	size_t fail = 0;
	_lots.reserve(outputPolys.size());
   for(size_t i=0; i<outputPolys.size(); i++)
	{
		WorldLot* lot = new WorldLot(outputPolys[i], outputIsExteriors[i], gp, rg);
		if(lot->hasError())
		{
			fail++;
			delete lot;
		}
		else
		{
			_lots.push_back(lot);
         lot->registerData(*mb);
		}
	}
*/
}

WorldBlock::~WorldBlock()
{
   BOOST_FOREACH(WorldLot* l, _lots) delete l;
}


list<Region*> WorldBlock::subdivide(Region* innerRegion, const CellParams &params, rando rg)
{
	list<Region*> outputRegions;

	// a queue is used to store the polygon data set to be subdivided
	queue<Region*>  regionQueue;
	regionQueue.push(innerRegion);



	//size_t count=0;
	while(!regionQueue.empty())
	{
		// calculate width and depth variables
		Real lotSplitWidthSq = Math::Sqr(params._lotWidth);
		Real lotSplitDepthSq = Math::Sqr(params._lotDepth);
		//DEBUG
		//if(count>10000)
		//	return outputRegions;
		//if(params._roadLimit && count > params._roadLimit)
		//	return outputRegions;
		//count++;

		// get lot boundary from queue
		Region* region = regionQueue.front();

		//	find the longest side
		Real  lsLengthSq;
		Real splitSize;

		// get the longest EXTERIOR side first
		DirectedEdge* longestEdge = region->getLongestEdge(true, lsLengthSq);
		if(lsLengthSq < lotSplitWidthSq)
		{
			// get the longest NON-EXTERIOR side second
			longestEdge = region->getLongestEdge(false, lsLengthSq);
			if(lsLengthSq < lotSplitDepthSq)
			{
				// add region and pop from queue
				outputRegions.insert(outputRegions.end(), region);
				if(params._debug) addDebugLot(region);
				regionQueue.pop();
				continue;
			}
			else
				splitSize = params._lotDepth;
		}
		else
			splitSize = params._lotWidth;


		// calculate the split factor
		Real  lsLength = Math::Sqrt(lsLengthSq);

		uint32 factor = Math::Floor(std::max(2.0f, (lsLength / splitSize)+1.0f));
		Real fraction = (Real)(1.0f/factor);
		Real midPos = factor/2 * fraction;

		// calculate the split points
		Vector3 leVec = longestEdge->dstPos() - longestEdge->srcPos();

		// calculate a deviated mid point for p1
		Vector3 p1 = longestEdge->srcPos() +
			leVec*(midPos + (params._lotDeviance * (rg() - 0.5) * fraction));

		// calculate a point perpendicular to the longest edge
		Vector3 p2(p1.x - leVec.z, p1.y, p1.z + leVec.x);
/*
		if(params._roadLimit != 0)
		{
			LogManager::getSingleton().logMessage(
				"Vector2 p1 = new Vector2(" +
				StringConverter::toString(p1.x) +
				", "+StringConverter::toString(p1.z) +
				");",LML_CRITICAL);

			LogManager::getSingleton().logMessage(
				"Vector2 p2 = new Vector2(" +
				StringConverter::toString(p2.x) +
				", "+StringConverter::toString(p2.z) +
				");",LML_CRITICAL);

			if(region->size() == 68)
				string hi ="hi";
		}
*/

		// split the region
		list<Region*> newRegions = Region::SplitRegion(region, p1, p2);
		//LogManager::getSingleton().logMessage("Split into " + StringConverter::toString(newRegions.size()));

		// pop region from queue once processed
		regionQueue.pop();

		BOOST_FOREACH(Region* r, newRegions)
		{
			if(r->hasExterior())
				regionQueue.push(r);
			else
			{
				if(params._debug) addDebugLot(r);
				delete r;
			}
		}
	}

	// return the list of regions
	return outputRegions;
}
/*
Vector3 WorldBlock::calcDevMidPoint(Vector3 &src, Vector3 &edgeVec, )
{

}
*/
void WorldBlock::addDebugLot(Region* r)
{
	vector<Vector3> debugLot;
	debugLot.reserve(r->size());
	DirectedEdge* curr = r->first();
	do
	{
		debugLot.push_back(curr->srcPos());
		curr = curr->next();
	}
	while(curr != r->first());
	_debugLots.push_back(debugLot);
}

void WorldBlock::drawDebug(ManualObject* debugMO)
{
	if(debugMO)
	{
		BOOST_FOREACH(vector<Vector3> &debugLot, _debugLots)
		{
			debugMO->begin("gk/Hilite/Red", RenderOperation::OT_LINE_STRIP);

			BOOST_FOREACH(Vector3& pos, debugLot)
				debugMO->position(pos);

			debugMO->position(debugLot[0]);
			debugMO->end();
		}
	}
}
