#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "stdafx.h"
#include "WorldObject.h"
#include "MoveableText.h"
#include "CityCell.h"

class WorldCell : public WorldObject
{

private:

protected:
	ManualObject* mManualObject;

public:
	WorldCell(SceneManager* creator, const CityCell& cell, const String& name, const Vector3& pos)
	{
		//
		mSceneNode = creator->getRootSceneNode()->createChildSceneNode(name); 


		ManualObject* mManualObject = new ManualObject(name); 
		mManualObject->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
		Vector3 offset(0,3,0);

		//better create some roads for these cells.
		RoadIterator ri, rend;
		for(boost::tie(ri, rend) = cell.mRoadGraph.getRoads(); ri != rend; ++ri)
		{
			const Ogre::Vector2& source2(cell.mRoadGraph.getNodePosition(cell.mRoadGraph.getRoadSource(*ri)));
			const Ogre::Vector2& target2(cell.mRoadGraph.getNodePosition(cell.mRoadGraph.getRoadTarget(*ri)));
			Ogre::Vector3 source3, target3;

			if(plotPointOnTerrain(source2, source3) && plotPointOnTerrain(target2, target3))
			{
				mManualObject->position(source3+offset); 
				mManualObject->position(target3+offset); 
			}
		}
		mManualObject->end(); 

		node->attachObject(mManualObject);

		CityCell* cellPtr = &cell;

		mCellSceneMap[cellPtr] = node;
	}


	}

	virtual ~WorldCELL()
	{
		SceneManager* destroyer = mSceneCELL->getCreator();
		destroyer->destroyEntity(mMesh);
		destroyer->destroyMovableObject(mLabel);
		destroyer->destroySceneCELL(mSceneCELL->getName());
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
		mSceneCELL->showBoundingBox(show);
	}

};

#endif
