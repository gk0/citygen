#include "stdafx.h"
#include "WorldLot.h"
#include "Triangulate.h"
#include "Geometry.h"
#include "Statistics.h"
#include "MeshBuilder.h"
#include "WorldMaterials.h"

using namespace Ogre;
using namespace std;

std::vector<WorldLot::MaterialInfo> WorldLot::_wallMaterialCache;

WorldLot::WorldLot(vector<Vector3> &poly, vector<bool> &isExterior, const CellParams &gp, rando rg)
{
   switch(gp._type)
   {
   case 0: //downtown
      buildExtrusion(poly, gp, rg);
      return;
   case 1:
      insetBoundary(poly, isExterior, 0.1*gp._lotWidth, 0.075*gp._lotWidth);
      buildExtrusion(poly, gp, rg);
      return;
   case 2:
      {
         // calculate minimum acceptable area
         Real minimumArea = Math::Sqr(std::min(gp._lotWidth, gp._lotDepth) * (1 - gp._lotDeviance));
         minimumArea /= 2;
         Vector2 midPoint2D;
         Real area;
         if(Geometry::calcCentroid2D(poly, midPoint2D, area) || area < minimumArea)
            return;

         // find first boundary side
         uint32 bIndex = 0;
         for(; bIndex < isExterior.size() && !isExterior[bIndex]; bIndex++); 

         // get the boundary side defining points
         Vector3 bp = poly[bIndex];
         Vector3 bp2 = poly[(bIndex + 1)%poly.size()];
         Vector3 bVec =  bp2 - bp;

         // find lot width
         // check for intersection 2
         bVec.normalise();
         Vector3 midPoint(midPoint2D.x, poly[0].y, midPoint2D.y);
         Vector3 p1 = midPoint, p2 = midPoint + bVec;
         Real r,s;
         Vector3 intscn;
         vector<Real> inscnLocs;
         for(size_t i = 0; i<poly.size(); i++)
         {
            size_t j = (i+1)%poly.size();
            if(Geometry::lineIntersect2D(p1, p2, poly[i], poly[j], intscn, r, s)
               && s >= 0 && s <= 1) inscnLocs.push_back(r);
         }
         if(inscnLocs.size() < 2) return;
         Real length = Math::Abs(inscnLocs[0] - inscnLocs[1]);

         Vector3 bHalfVec = bVec * (length * 0.42);
         Vector3 bpPerp(-bVec.z, bVec.y, bVec.x);
         bpPerp = bpPerp * (gp._lotDepth * 0.16); 

         vector<Vector3> tmpPoly;        
         tmpPoly.push_back(midPoint - bpPerp - bHalfVec);
         tmpPoly.push_back(midPoint - bpPerp + bHalfVec); 
         tmpPoly.push_back(midPoint + bpPerp + bHalfVec);
         tmpPoly.push_back(midPoint + bpPerp - bHalfVec);
         poly.swap(tmpPoly);
      }
      buildExtrusion(poly, gp, rg);
      return;
   }
}

void WorldLot::buildExtrusion(vector<Vector3> &poly, const CellParams &gp, rando rg)
{
   // calculate the height
   Real height = gp._buildingHeight + gp._buildingHeight * gp._buildingDeviance * rg();

   // find the minimum y value for foundation
   Real bottom = numeric_limits<Real>::max();
   BOOST_FOREACH(const Vector3 &p, poly)
      if(p.y < bottom) bottom = p.y;

   // set height relative to foundation
   Real top = bottom + height;

   // select materials
   if(WorldLot::_wallMaterialCache.size() == 0)
   {
      const vector<Material*> wallMaterials(WorldMaterials::getSingleton().getMaterials("wall"));
      BOOST_FOREACH(Material* m, wallMaterials)
      {
         MaterialInfo mi;
         mi._material = m;
         mi._u = 1, mi._v = 1;
         WorldMaterials::getMaterialUV(mi._material, mi._u, mi._v);
         _wallMaterialCache.push_back(mi);
      }
   }
   size_t wallIndex = Math::Floor(_wallMaterialCache.size() * rg());
   _wallMaterial = _wallMaterialCache[wallIndex]._material;
   Real uScale = _wallMaterialCache[wallIndex]._u;
   Real vScale = _wallMaterialCache[wallIndex]._v;

   if(Triangulate::Process(poly, _indexData))
   {
      // reserve data for vertices & polygons
      _vertexData.reserve(poly.size() * 5 * 8);
      _indexData.reserve(_indexData.size() + (poly.size() * 6));

      // load in the roof footprint into vertex data
      BOOST_FOREACH(Vector3 &fp, poly)
      {
         MeshBuilder::addVData3(_vertexData, fp.x, top, fp.z);	// position
         MeshBuilder::addVData3(_vertexData, 0.0f, 1.0f, 0.0f);		// normal
         MeshBuilder::addVData2(_vertexData, 0.0f, 0.0f);				// texture coordinates
      }

      // process the sides
      size_t i,j,N = poly.size();
      for(i=0; i<N; i++)
      {
         j = (i+1)%N;

         // calculate texCoords
         Real uMax =	Math::Ceil((poly[i] - poly[j]).length() / (uScale));
         Real vMax =	Math::Ceil(height / (vScale));

         //calculate normal
         Ogre::Vector3 normal(-poly[i].z + poly[j].z, 0, poly[i].x - poly[j].x);
         normal.normalise();

         // add vertex data with normals and texture coordinates
         // v1.
         MeshBuilder::addVData3(_vertexData, poly[i].x, bottom, poly[i].z);
         MeshBuilder::addVData3(_vertexData, normal);
         MeshBuilder::addVData2(_vertexData, uMax, vMax);
         // v2.
         MeshBuilder::addVData3(_vertexData, poly[j].x, bottom, poly[j].z);
         MeshBuilder::addVData3(_vertexData, normal);
         MeshBuilder::addVData2(_vertexData, 0.0f, vMax);
         // v3.
         MeshBuilder::addVData3(_vertexData, poly[i].x, top, poly[i].z);
         MeshBuilder::addVData3(_vertexData, normal);
         MeshBuilder::addVData2(_vertexData, uMax, 0.0f);
         // v4.
         MeshBuilder::addVData3(_vertexData, poly[j].x, top, poly[j].z);
         MeshBuilder::addVData3(_vertexData, normal);
         MeshBuilder::addVData2(_vertexData, 0.0f, 0.0f);

         // polygons
         uint16 vertexPos = static_cast<uint16>(N + (i*4));
         MeshBuilder::addIData3(_indexData, vertexPos+3, vertexPos+1, vertexPos);		//v4,v2,v1
         MeshBuilder::addIData3(_indexData, vertexPos, vertexPos+2, vertexPos+3);		//v1,v3,v4
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

void WorldLot::registerData(MeshBuilder &mb)
{
   mb.registerData(_wallMaterial, _vertexData, _indexData);
}

void WorldLot::insetBoundary(vector<Vector3> &poly,
   vector<bool> &isExterior,const Real &roadInset, const Real &standardInset)
{
	vector<Real> insets;
	BOOST_FOREACH(bool b, isExterior) insets.push_back(b ? roadInset : standardInset);
	Geometry::polygonInset(insets, poly);
}
/*
bool WorldLot::sliceBoundary(Vector3 &p1, Vector3 &p2, Vector3 &bVec, 
                                    LotBoundary &b)
{
   Vector3 p3 = p1 + bVec, p4 = p2 + bVec;
   Vector3 int1,int2,int3,int4;
   size_t i1, i2, i3, i4;
   Real r,s;
   LotBoundary newBoundary;

   // need to loop around boundary
   size_t i;
   
   // check for intersection 1
   for(i = 0; i<b.size(); i++)
   {
      i1 = i;
      size_t j = (i1+1)%b.size();
      if(Geometry::lineIntersect2D(p1, p3, b[i1]._pos, b[j]._pos, int1, r, s)
         && s >= 0 && s <= 1) break;
   }
   if(i >= b.size()) return false;

   // check for intersection 2
   for(i = 0; i<b.size(); i++)
   {
      i2 = (i1 + i)%b.size();
      size_t j = (i2+1)%b.size();
      if(Geometry::lineIntersect2D(p1, p3, b[i2]._pos, b[j]._pos, int2, r, s)
         && s >= 0 && s <= 1) break;
   }
   if(i >= b.size()) return false;

   // check for intersection 3
   for(i = 0; i<b.size(); i++)
   {
      i3 = (i2 + i)%b.size();
      size_t j = (i3+1)%b.size();
      if(Geometry::lineIntersect2D(p2, p4, b[i3]._pos, b[j]._pos, int3, r, s)
         && s >= 0 && s <= 1) break;
   }
   if(i >= b.size()) return false;

   // check for intersection 4
   for(i = 0; i<b.size(); i++)
   {
      i4 = (i3 + i)%b.size();
      size_t j = (i4+1)%b.size();
      if(Geometry::lineIntersect2D(p2, p4, b[i4]._pos, b[j]._pos, int4, r, s)
         && s >= 0 && s <= 1) break;
   }
   if(i >= b.size()) return false;

   LotBoundaryPoint bp(true, int1);
   newBoundary.push_back(bp);
   LotBoundaryPoint bp2(false, int2);
   newBoundary.push_back(bp2);

   for(i=0; i<b.size(); i++)
   {
      size_t j = (i2 + i + 1) % b.size();
      newBoundary.push_back(b[j]);
      if(j == i3) break;
   }
   newBoundary.push_back(LotBoundaryPoint(false, int3));
   newBoundary.push_back(LotBoundaryPoint(false, int4));
   for(i=0; i<b.size(); i++)
   {
      size_t j = (i4 + i + 1) % b.size();
      newBoundary.push_back(b[j]);
      if(j == i1) break;
   }

   b.swap(newBoundary);
	return true;
}
*/
