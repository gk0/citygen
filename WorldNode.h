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
	Entity* mHighlight;
	Entity* mSelected;
	//SceneNode* mMeshNode;
	MovableText* mLabel;


public:
	WorldNode(SceneManager* creator, const String& name, const String& label, const Vector3& pos)
	{
		mHighlight = 0;
		mSelected = 0;

		// create our scene node
		mSceneNode = creator->getRootSceneNode()->createChildSceneNode(name, pos);
		Vector3 po = mSceneNode->getPosition();

		// create mesh
		mMesh = creator->createEntity(name+"Mesh", "node.mesh" );
		MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName("gk/Hilite/Red2");
		//MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().create("gk/Hilite/Red2");
		mat->getTechnique(0)->getPass(0)->setAmbient(0.1, 0.1, 0.1);
		mat->getTechnique(0)->getPass(0)->setDiffuse(0.5, 0, 0, 1.0);
		mat->getTechnique(0)->getPass(0)->setSpecular(0.7, 0.7, 0.7, 0.5);
		mMesh->setMaterialName("gk/Hilite/Red2");

		// create highlight mesh
		mHighlight = creator->createEntity(name+"Highlight", "flange.mesh" );
		mHighlight->setMaterialName("gk/Hilite/Yellow");
		mHighlight->setVisible(false);
		//mHighlight->setVisibilityFlags(

		// create select mesh
		mSelected = creator->createEntity(name+"Selected", "node.mesh" );
		mSelected->setMaterialName("gk/Hilite/Yellow");
		mSelected->setVisible(false);

		// create moveable text label
		mLabel = new MovableText("Label"+name, label);
		mLabel->setCharacterHeight(5);
		mLabel->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE); // Center horizontally and display above the node
		mLabel->setAdditionalHeight( 5.0f );

		// attach objects and set scale
		//mMeshNode = mSceneNode->createChildSceneNode();
		//mMeshNode->attachObject( mMesh );
		mSceneNode->attachObject( mMesh);
		mSceneNode->attachObject(mHighlight);
		mSceneNode->attachObject(mSelected);
		mSceneNode->attachObject(mLabel);
		//mSceneNode->setScale( 0.1f, 0.1f, 0.1f );

		//mSceneNode->setPosition(0,5,0);

	}

	virtual ~WorldNode()
	{
		SceneManager* destroyer = mSceneNode->getCreator();
		destroyer->destroyEntity(mMesh);
		destroyer->destroyEntity(mHighlight);
		destroyer->destroyEntity(mSelected);
		delete mLabel;
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

	void showHighlighted(bool highlighted)
	{
		mHighlight->setVisible(highlighted);
		//mMesh->setVisible(!highlighted & !(mSelected->getVisible()));
	}

	void showSelected(bool selected)
	{
		mSelected->setVisible(selected);
		//mMesh->setVisible(!selected);
	}

/*
	void setNodeProperties(const double& x, const double& y, const double& z)
	{
		if(mSelectedWorldNode)
		{
			SceneNode* sn = mSelectedWorldNode->getSceneNode();
			mSceneNodeMap[sn].worldNode->setLabel(l);

			//mCurrentNode->
			Vector3 pos(static_cast<Real>(x), static_cast<Real>(y), static_cast<Real>(z));
			if(plotPointOnTerrain(convert3DPointTo2D(pos), pos))
				moveNode(sn, pos);
			else
				// no move has taken place because the new location could not be plotted on the terrain
				// -write back the last correct values to the property inspector
				selectNode(mCurrentWorldNode);
			Update();
		}
	}
*/


};

#endif
