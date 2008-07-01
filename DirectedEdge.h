#ifndef DIRECTEDEDGE_H
#define DIRECTEDEDGE_H

#include <OgreVector3.h>

class DirectedEdge
{

private:
	DirectedEdge* _next;
	DirectedEdge* _prev;
	Ogre::Vector3 _pos;


public:
	Ogre::Real _s;
	bool _exterior;

	DirectedEdge(const Ogre::Vector3 &srcPos)
	{
		_pos = srcPos;
		_prev = this;
		_next = this;
		_exterior = false;
	}

	DirectedEdge(const Ogre::Vector3 &srcPos, DirectedEdge* prev)
	{
		_pos = srcPos;
		prev->_next = this;
		_prev = prev;
		_exterior = false;
	}

	~DirectedEdge(void) {};

	DirectedEdge* next()
	{
		return _next;
	}

	DirectedEdge* prev()
	{
		return _prev;
	}

	const Ogre::Vector3& srcPos()
	{
		return _pos;
	}

	const Ogre::Vector3& dstPos()
	{
		return _next->_pos;
	}

	static void closeCycle(DirectedEdge* prev, DirectedEdge* next)
	{
		next->_prev = prev;
		prev->_next = next;
	}

	DirectedEdge* insert(const Ogre::Vector3& p)
	{
		DirectedEdge* n = next();
		DirectedEdge* d = new DirectedEdge(p, this);
		d->_exterior = _exterior;
		closeCycle(d, n);
		return d;
	}

	static void bridge(DirectedEdge* a, DirectedEdge* b)
	{
		DirectedEdge* aNext = a->_next;

		// first side
		DirectedEdge* b2 = new DirectedEdge(b->_pos);
		a->_next = b2;
		b2->_prev = a;
		b2->_next = b->_next;
		b->_next->_prev = b2;
		b2->_s = b -> _s;

		// second side
		DirectedEdge* a2 = new DirectedEdge(a->_pos);
		a2->_prev = b;
		a2->_next = aNext;
		aNext->_prev = a2;
		b->_next = a2;
		a2->_s = a -> _s;

		// do exterior
		b2->_exterior = b->_exterior;
		a2->_exterior = a->_exterior;
		a->_exterior = false;
		b->_exterior = false;
	}

	Ogre::Real getLengthSquared2D()
	{
		return Ogre::Math::Sqr(_pos.x - _next->_pos.x) + Ogre::Math::Sqr(_pos.z - _next->_pos.z);
	}
};

#endif

