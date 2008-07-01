#ifndef WORLDMATERIALS_H
#define WORLDMATERIALS_H

#include "stdafx.h"
#include <OgreMaterial.h>
#include <OgreSingleton.h>
#include <map>
#include <vector>

class WorldMaterials : public Ogre::Singleton<WorldMaterials>
{
private:
   typedef std::map<std::string, std::vector<Ogre::Material*> > MaterialMap;
   MaterialMap _materials;

public:
   WorldMaterials();

   const std::vector<Ogre::Material*>& getMaterials(const std::string &name)
   {
      return _materials[name];
   }

   Ogre::Material* getDefaultMaterial(const std::string &name)
   {
      return (_materials[name])[0];
   }

   void addMaterial(const std::string &tag, const std::string &materialName);

   static bool getMaterialUV(Ogre::Material* mat, Ogre::Real &u, Ogre::Real &v);

   static WorldMaterials& getSingleton(void);
   static WorldMaterials* getSingletonPtr(void);
};

#endif
