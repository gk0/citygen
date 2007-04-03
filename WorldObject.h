#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include "stdafx.h"

class WorldObject
{
private:
	bool mValid;
	std::vector<WorldObject*> mAttachments;

protected:
	bool isValid() const { return mValid; }
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
		std::vector<WorldObject*>::iterator aIt, aEnd;
		for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; aIt++)
		{
			(*aIt)->invalidate();
		}
	}

	void attach(WorldObject* wo)
	{
		mAttachments.push_back(wo);
	}

	void detach(WorldObject* wo)
	{
		std::vector<WorldObject*>::iterator aIt, aEnd;
		for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; )
		{
			if((*aIt) == wo)
			{
				aIt = mAttachments.erase(aIt); 
				aEnd = mAttachments.end();
			}
			else aIt++;
		}
	}

	std::vector<WorldObject*> getAllAttachments()
	{
		// add own attachments
		std::vector<WorldObject*> allAttachments(mAttachments);

		// add attachments' attachments
		std::vector<WorldObject*>::iterator aIt, aEnd;
		for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; aIt++)
		{
			std::vector<WorldObject*> a((*aIt)->getAllAttachments());
			allAttachments.insert(allAttachments.end(), a.begin(), a.end());
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
