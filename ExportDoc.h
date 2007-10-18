#ifndef EXPORTDOC_H
#define EXPORTDOC_H

#include "stdafx.h"


class ExportDoc
{

public:
	virtual ~ExportDoc() {}
	virtual void addMesh(Ogre::MeshPtr mesh) = 0;
	virtual void addMesh(Ogre::SceneNode* sn, Ogre::MeshPtr mesh) = 0;
	virtual void addCamera(Ogre::Camera *cam, Ogre::SceneNode *camNode) = 0;
	virtual void addLight(Ogre::Light *l) = 0;
};

#endif
