#ifndef WORLDNODE_H
#define WORLDNODE_H

#include "stdafx.h"
#include "WorldObject.h"
#include "NodeInterface.h"
#include "MovableText.h"
#include "RoadGraph.h"

class WorldRoad;

class WorldNode : public WorldObject, public NodeInterface
{

private:
	static int mInstanceCount;
	Ogre::Entity* mMesh;
	Ogre::Entity* mHighlight;
	Ogre::Entity* mSelected;
	Ogre::MovableText* mLabel;
	Ogre::SceneManager* mCreator;
	std::vector<WorldRoad*> mRoads;

	void init(const Ogre::String& name, const Ogre::String& label);

public:
	NodeId mSimpleNodeId;

	WorldNode(Ogre::SceneManager* creator, const Ogre::String& name);
	WorldNode(Ogre::SceneManager* creator);
	WorldNode(Ogre::SceneManager* creator, const Ogre::Vector3& pos);
	~WorldNode();

	void setLabel(const Ogre::String& label);
	const Ogre::String& getLabel() const;
	void showHighlighted(bool highlighted);
	void showSelected(bool selected);
	Ogre::Vector3 getPosition3D() const;
	void setPosition(const Ogre::Vector3 &pos);
	bool setPosition2D(Ogre::Real x, Ogre::Real z);
	void setPosition3D(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void setPosition3D(const Ogre::Vector3& pos);
	void setPosition(Ogre::Real x, Ogre::Real y, Ogre::Real z);

	Ogre::Vector2 getPosition2D() const;
	const Ogre::Vector3& getPosition() const { return WorldObject::getPosition(); }

	void attach(WorldObject* wo);
	void detach(WorldObject* wo);

	bool move(Ogre::Vector2 pos);

	void notify();
	
	bool loadXML(const TiXmlHandle& nodeRoot);
	bool hasRoadIntersection();

	void build() { return; }


}; 

#endif
