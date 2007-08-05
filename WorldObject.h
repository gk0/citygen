#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include "stdafx.h"

class WorldObject
{
protected:
	bool mValid;
	std::set<WorldObject*> mAttachments;

protected:
	
	Ogre::SceneNode* mSceneNode;
	bool mVisible;

public:

	WorldObject()
	{
		mValid = false;
		mVisible = true;
		mSceneNode = 0;
	}

	virtual ~WorldObject()
	{
	}

	bool isValid() const { return mValid; }

	void setPosition(const Ogre::Vector3& pos)
	{ 
		mSceneNode->setPosition(pos); 
	}

	void setPosition(Ogre::Real x, Ogre::Real y, Ogre::Real z) 
	{ 
		mSceneNode->setPosition(x, y, z); 
	}

	void setVisible(const bool vis)
	{
		mSceneNode->setVisible(vis);
		mVisible = vis;
	}

	const Ogre::Vector3& getPosition() const 
	{ 
		return mSceneNode->getPosition(); 
	}

	Ogre::SceneNode* getSceneNode() const 
	{ 
		return mSceneNode; 
	}

	const Ogre::String& getName() 
	{ 
		return mSceneNode->getName(); 
	}

	const bool getVisible() 
	{ 
		return mVisible; 
	}
	 
	void showBoundingBox(bool show) 
	{ 
		mSceneNode->showBoundingBox(show); 
	}

	virtual void invalidate()
	{
		mValid = false;
		std::set<WorldObject*>::iterator aIt, aEnd;
		for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; aIt++)
		{
			(*aIt)->invalidate();
		}
	}

	void attach(WorldObject* wo)
	{
		mAttachments.insert(wo);
	}

	void detach(WorldObject* wo)
	{
		mAttachments.erase(wo);
	}

	std::set<WorldObject*> getAllAttachments()
	{
		// add own attachments
		std::set<WorldObject*> allAttachments(mAttachments);

		// add attachments' attachments
		std::set<WorldObject*>::iterator aIt, aEnd;
		for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; aIt++)
		{
			std::set<WorldObject*> a((*aIt)->getAllAttachments());
			allAttachments.insert(a.begin(), a.end());
		}
		return allAttachments;
	}

	void validate()
	{
		if(!mValid)
		{
			build();
			mValid = true;
		}
	}

	virtual void build() = 0; 

};

#endif
