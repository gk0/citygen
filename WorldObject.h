#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include "stdafx.h"

class WorldObject
{
protected:
	bool					_valid;
	std::set<WorldObject*>	_attachments;

protected:
	
	Ogre::SceneNode*		_sceneNode;
	bool					_visible;

public:

	WorldObject()
	{
		_valid = false;
		_visible = true;
		_sceneNode = 0;
	}

	virtual ~WorldObject()
	{
	}

	bool isValid() const { return _valid; }

	void setPosition(const Ogre::Vector3& pos)
	{ 
		_sceneNode->setPosition(pos); 
	}

	void setPosition(Ogre::Real x, Ogre::Real y, Ogre::Real z) 
	{ 
		_sceneNode->setPosition(x, y, z); 
	}

	void setVisible(const bool vis)
	{
		_sceneNode->setVisible(vis);
		_visible = vis;
	}

	const Ogre::Vector3& getPosition() const 
	{ 
		return _sceneNode->getPosition(); 
	}

	Ogre::SceneNode* getSceneNode() const 
	{ 
		return _sceneNode; 
	}

	const Ogre::String& getName() 
	{ 
		return _sceneNode->getName(); 
	}

	const bool getVisible() 
	{ 
		return _visible; 
	}
	 
	void showBoundingBox(bool show) 
	{ 
		_sceneNode->showBoundingBox(show); 
	}

	virtual void invalidate()
	{
		_valid = false;
		std::set<WorldObject*>::iterator aIt, aEnd;
		for(aIt = _attachments.begin(), aEnd = _attachments.end(); aIt != aEnd; aIt++)
		{
			(*aIt)->invalidate();
		}
	}

	void attach(WorldObject* wo)
	{
		_attachments.insert(wo);
	}

	void detach(WorldObject* wo)
	{
		_attachments.erase(wo);
	}

	std::set<WorldObject*> getAllAttachments()
	{
		// add own attachments
		std::set<WorldObject*> allAttachments(_attachments);

		// add attachments' attachments
		std::set<WorldObject*>::iterator aIt, aEnd;
		for(aIt = _attachments.begin(), aEnd = _attachments.end(); aIt != aEnd; aIt++)
		{
			std::set<WorldObject*> a((*aIt)->getAllAttachments());
			allAttachments.insert(a.begin(), a.end());
		}
		return allAttachments;
	}

	void validate()
	{
		if(!_valid)
		{
			build();
			_valid = true;
		}
	}

	virtual void build() = 0; 

};

#endif
