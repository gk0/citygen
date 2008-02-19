#include "stdafx.h"
#include "Skeletor.h"
#include "Geometry.h"

using namespace Ogre;
using namespace std;

SLAV::SLAV()
{
	_root = 0;
	_size = 0;
}

SLAV::SLAV(const Real inset, const vector<Vector3> &poly)
{
	_root = 0;
	_size = 0;

	size_t i,j,N = poly.size();
	assert(N >= 3);
	size_t lastI = N-1;
	for(i=0; i<N; i++)
	{
		j = (i+1) % N;
		InsetVertex *iv = new InsetVertex();
		iv->_pos = poly[i];
		iv->_insetTarget = Geometry::calcInsetTarget(poly[lastI], poly[i], 
				poly[j], inset, inset);
		iv->_inset = inset;
		iv->_intersectionTested = false;
		add(iv);
		lastI = i;
	}
}

SLAV::SLAV(const vector<Real> insets, const vector<Vector3> &poly)
{
	_root = 0;
	_size = 0;

	size_t i,j,N = poly.size();
	assert(N >= 3);
	size_t lastI = N-1;
	for(i=0; i<N; i++)
	{
		j = (i+1) % N;
		InsetVertex *iv = new InsetVertex();
		iv->_pos = poly[i];
		iv->_insetTarget = Geometry::calcInsetTarget(poly[lastI], poly[i], 
				poly[j], insets[lastI], insets[i]);
		iv->_inset = insets[i];
		iv->_intersectionTested = false;
		add(iv);
		lastI = i;
	}
}

SLAV::~SLAV()
{
	for(InsetVertex* iv = _root->_right->_right; iv != _root; iv = iv->_right) delete iv->_left;
	delete _root;
}

void SLAV::add(InsetVertex* v)
{
	if(_root == 0)
	{
		_root = v;
		_root->_left = _root;
		_root->_right = _root;
	}
	else
	{
		v->_left = _root->_left;
		v->_right = _root;
		_root->_left->_right = v;
		_root->_left = v;
	}
	_size++;
}

void SLAV::remove(InsetVertex* v)
{
	if(v == _root) _root = v->_right;
	v->_left->_right = v->_right;
	v->_right->_left = v->_left;
	delete v;
	_size--;
}

bool Skeletor::processInset(SLAV &sv, vector<Vector3> &poly)
{
	//TODO: Use a slav and a queue instead of the list
	//queue<InsetVertex*> ivQueue;
	//BOOST_FOREACH(InsetVertex& iv, ivList) ivQueue.push(&iv);

	while(true)
	{
		///////////////////////////////////////////////////////////////////
		// 1. Intersection test for all inset vectors
		///////////////////////////////////////////////////////////////////
		
		// find the earliest intersection
		Real intersectionLocation = 1;
		Vector2 intersection(Vector2::ZERO);
		InsetVertex *iv,*firstOffender=0;

		iv = sv.getRoot();
		do
		{
			if(!iv->_intersectionTested)
			{
				Vector2 ivPos2D(iv->_pos.x, iv->_pos.z);
				Vector2 nextIvPos2D(iv->_right->_pos.x, iv->_right->_pos.z);

				// check if the pair intersect and store the lowest
				Real r;
				Vector2 tmpInscn;
				if(Geometry::lineIntersect(ivPos2D, iv->_insetTarget, 
						nextIvPos2D, iv->_right->_insetTarget, tmpInscn, r) &&
						r >= 0 && r <= 1)
				{
					// TODO: tolerance value could be used here
					if(r < intersectionLocation) 
					{
						intersectionLocation = r;
						firstOffender = iv;
						intersection = tmpInscn;
					}
				}
				else
					iv->_intersectionTested = true;
			}
			iv = iv->_right;
		}
		while(iv != sv.getRoot());
		
		///////////////////////////////////////////////////////////////////
		// 2. Process Bisector Intersection
		///////////////////////////////////////////////////////////////////
		
		// find the closest intersection
		if(intersectionLocation != 1.0)
		{
			// remove the first offender
			InsetVertex *secondOffender=firstOffender->_right;
			sv.remove(firstOffender);

			// if there is a valid polygon remaining
			if(sv.getSize() >= 3)
			{
				// update the pos and inset of the remaining vertices
				iv = sv.getRoot();
				do
				{
					iv->_pos.x = iv->_pos.x + intersectionLocation * (iv->_insetTarget.x - iv->_pos.x);
					iv->_pos.z = iv->_pos.z + intersectionLocation * (iv->_insetTarget.y - iv->_pos.z);
					iv->_inset = iv->_inset * (1 - intersectionLocation);
					iv = iv->_right;
				}
				while(iv != sv.getRoot());
				
				// update the second offender
				secondOffender->_pos.x = intersection.x;
				secondOffender->_pos.z = intersection.y;
				secondOffender->_insetTarget = Geometry::calcInsetTarget(secondOffender->_left->_pos, secondOffender->_pos, 
						secondOffender->_right->_pos, secondOffender->_left->_inset, secondOffender->_inset);

				secondOffender->_intersectionTested = false;
				secondOffender->_left->_intersectionTested = false;
			}
			else
			{
				//LogManager::getSingleton().logMessage("Less than 3 vertices after collapse.");
				return false;
			}
		}
		else
		{
			//LogManager::getSingleton().logMessage("Valid.");
			poly.reserve(sv.getSize());
			//poly.push_back(Vector3(sv.getRoot()->_insetTarget.x, sv.getRoot()->_pos.y, sv.getRoot()->_insetTarget.y));
			iv = sv.getRoot();
			do
			{
				poly.push_back(Vector3(iv->_insetTarget.x, iv->_pos.y, iv->_insetTarget.y));
				iv = iv->_right;
			}
			while(iv != sv.getRoot());
			break;
		}
	}
	return true;
}
