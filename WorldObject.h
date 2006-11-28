#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include "stdafx.h"

using namespace Ogre;

class WorldObject
{
public:
	void setPosition(const Vector3& pos) { mSceneNode->setPosition(pos); }
	void setPosition(const Real& x, const Real& y, const Real& z) { mSceneNode->setPosition(x, y, z); }
	void setVisible(const bool vis) { mSceneNode->setVisible(vis); }

	const Vector3& getPosition() const { return mSceneNode->getPosition(); }
	SceneNode* getSceneNode() const { return mSceneNode; }
	const String& getName() { return mSceneNode->getName(); }

protected:
	SceneNode* mSceneNode;
};

#endif
