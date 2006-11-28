#ifndef WORLDNODE_H
#define WORLDNODE_H

#include "stdafx.h"
#include "WorldObject.h"
#include "MoveableText.h"

class WorldNode : public WorldObject
{

private:

protected:
	Entity* mMesh;
	//SceneNode* mMeshNode;
	MovableText* mLabel;

public:
	WorldNode(SceneManager* creator, const String& name, const String& label, const Vector3& pos)
	{
		// create our scene node
		mSceneNode = creator->getRootSceneNode()->createChildSceneNode(name, pos);
		Vector3 po = mSceneNode->getPosition();

		// create mesh
		mMesh = creator->createEntity(name+"Mesh", "node.mesh" );
		

		//MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName("gk/Hilite/Red");
		//mat->set
		//mat->setAmbient(0,255,0);
		//m->setAmbient(0,255,0);

		mMesh->setMaterialName("gk/Hilite/Red");

		// create moveable text label
		mLabel = new MovableText("Label"+name, label);
		mLabel->setCharacterHeight(48);
		mLabel->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE); // Center horizontally and display above the node
		mLabel->setAdditionalHeight( 5.0f );

		// attach objects and set scale
		//mMeshNode = mSceneNode->createChildSceneNode();
		//mMeshNode->attachObject( mMesh );
		mSceneNode->attachObject( mMesh);
		mSceneNode->attachObject(mLabel);
		mSceneNode->setScale( 0.1f, 0.1f, 0.1f );
	}

	virtual ~WorldNode()
	{
		SceneManager* destroyer = mSceneNode->getCreator();
		destroyer->destroyEntity(mMesh);
		destroyer->destroyMovableObject(mLabel);
		destroyer->destroySceneNode(mSceneNode->getName());
	}

	void setLabel(const String& label)
	{
		mLabel->setCaption(label);
	}

	const String& getLabel() const
	{
		return mLabel->getCaption();
	}

	void showBoundingBox(bool show)
	{
		mSceneNode->showBoundingBox(show);
	}

};

#endif
