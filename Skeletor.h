#ifndef SKELETOR_H
#define SKELETOR_H

#include "stdafx.h"
#include <OgreVector2.h>
#include <OgreVector3.h>

struct InsetVertex;

struct InsetVertex
{
	Ogre::Vector3 _pos;
	Ogre::Vector2 _insetTarget;
	Ogre::Real _inset;
	InsetVertex* _left;
	InsetVertex* _right;
	bool _intersectionTested;
};

class SLAV
{
private:
	InsetVertex* _root;
	size_t	_size;

public:
	SLAV();
	SLAV(const Ogre::Real inset, const std::vector<Ogre::Vector3> &poly);
	SLAV(const std::vector<Ogre::Real> insets, const std::vector<Ogre::Vector3> &poly);
	SLAV(InsetVertex* r) { _root = r; }
	virtual ~SLAV();

	void add(InsetVertex* v);
	void remove(InsetVertex* v);
	InsetVertex* getRoot() { return _root; }
	size_t getSize() { return _size; }
};


class Skeletor
{
public:
	static bool processInset(SLAV& sv, std::vector<Ogre::Vector3> &poly);
};


#endif
