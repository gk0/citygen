#include "stdafx.h"
#include "WorldMaterials.h"
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

using namespace std;
using namespace Ogre;

WorldMaterials::WorldMaterials()
{
   // set up categories
   _materials.insert(make_pair("road", std::vector<Material*>()));
   _materials.insert(make_pair("junction", std::vector<Material*>()));
   _materials.insert(make_pair("pavement", std::vector<Material*>()));
   _materials.insert(make_pair("wall", std::vector<Material*>()));
   _materials.insert(make_pair("roof", std::vector<Material*>()));

   // add materials
   addMaterial("pavement", "gk/Paving");
   addMaterial("wall", "gk/Building1WRelief");
   addMaterial("wall", "gk/Building2WRelief");
   addMaterial("wall", "gk/Building3WRelief");
   addMaterial("wall", "gk/Building4WRelief");
   addMaterial("wall", "gk/Building5WNormalMap");

   addMaterial("junction", "gk/RoadJunction");
   addMaterial("road", "gk/Road");
}

void WorldMaterials::addMaterial(const string &tag, const string &materialName)
{
   _materials[tag].push_back(
      static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(materialName)).get());
}

bool WorldMaterials::getMaterialUV(Material* mat, Real &u, Real &v)
{
   Technique* t = mat->getTechnique(0);
   string id = mat->getName();
   unsigned short numOfPasses = static_cast<unsigned short>(t->getNumPasses());
   if(numOfPasses != 0) 
   {
      for(short i=(numOfPasses-1); i>=0; i--)
      {
         Pass* p = t->getPass(i);
         for(unsigned short j=0; j<p->getNumTextureUnitStates(); j++)
         {
            TextureUnitState* tu = p->getTextureUnitState("DiffuseMap");
            if(tu != 0 && (id.find("Normal") != string::npos || id.find("Relief") != string::npos))
            {
               u = tu->getTextureUScale();
               v = tu->getTextureVScale();
               return true;
            }
         }
      }
   }
   return false;
}

template<> WorldMaterials* Ogre::Singleton<WorldMaterials>::ms_Singleton = 0;
WorldMaterials* WorldMaterials::getSingletonPtr(void)
{
   return ms_Singleton;
}
WorldMaterials& WorldMaterials::getSingleton(void)
{
   assert(ms_Singleton);
   return (*ms_Singleton);
}
