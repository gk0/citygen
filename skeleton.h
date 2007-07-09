#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "prim.h"

namespace Felkel {

struct SkeletonLine;

struct Vertex
{
	Vertex (void) : ID (-1) { };       // Bezparametricky konstruktor kvuli STL
	Vertex (const Ogre::Vector2 &p, const Ogre::Vector2 &prev = Ogre::Vector2 (), const Ogre::Vector2 &next = Ogre::Vector2 ())
		: point (p), axis (angleAxis (p, prev, next)), leftLine (p, prev), rightLine (p, next), higher (NULL),
		leftVertex (NULL), rightVertex (NULL), nextVertex (NULL), prevVertex (NULL), done (false), ID (-1),
		leftSkeletonLine (NULL), rightSkeletonLine (NULL), advancingSkeletonLine (NULL) { }
	Vertex (const Ogre::Vector2 &p, Vertex &left, Vertex &right);
	Vertex *highest (void) { return higher ? higher -> highest () : this; }
	bool atContour (void) const { return leftVertex == this && rightVertex == this; }
	bool operator == (const Vertex &v) const { return equ(point, v.point); }
	bool operator < (const Vertex &) const { assert (false); return false; } // kvuli STL, jinak se nepouziva

	// data
	Ogre::Vector2 point;                       // souradnice vrcholu
	Ray axis;                          // bisektor
	Ray leftLine, rightLine;           // leva a prava hranicni usecka, axis je jejich osou
	Vertex *leftVertex, *rightVertex;  // 2 vrcholy, jejich zaniknutim vzikl tento
	Vertex *nextVertex, *prevVertex;   // vrchol sousedici v LAV
	Vertex *higher;                    // vrchol vznikly pri zaniknuti tohoto
	bool done;                         // priznak aktivity
	size_t ID;                            // cislo automaticky pridelovane pri vkladani do LAV
	SkeletonLine *leftSkeletonLine, *rightSkeletonLine, *advancingSkeletonLine; // Pouzivano pri konstrukci kostry z okridlenych hran
};

struct SkeletonLine
{
	SkeletonLine (void) : ID (-1) { }
	SkeletonLine (const Vertex &l, const Vertex &h) : lower (l), higher (h), ID (-1) { };
	operator Segment (void) { 
		return Segment (lower.vertex -> point, higher.vertex -> point); 
	} // Jen pro ladeni

	struct SkeletonPoint
	{
		SkeletonPoint(const Vertex &v = Vertex (), SkeletonLine *l = NULL, SkeletonLine *r = NULL)
		: vertex (&v), left (l), right (r) { }
		const Vertex *vertex;                // ukazatel na vrchol (obsahuje souradnice)
		SkeletonLine *left, *right;          // kridla
		size_t leftID (void) const { if (!left) return -1; return left -> ID; }
		size_t rightID (void) const { if (!right) return -1; return right -> ID; }
		size_t vertexID (void) const { if (!vertex) return -1; return vertex -> ID; }
	} lower, higher;                       // dva body typu SkeletonOgre::Vector2

	bool operator == (const SkeletonLine &s) const
	{ 
		return higher.vertex -> ID == s.higher.vertex -> ID  && lower.vertex -> ID  == s.lower.vertex -> ID ; 
	}
	bool operator < (const SkeletonLine &) const { 
		assert (false); return false; 
	}     // kvuli STL, jinak se nepouziva
	size_t ID;                                // Cislo automaticky pridelovane pri vkladani do kostry
};

class Skeleton;

struct Intersection
{
	Intersection (void) { };           // Bezparametricky konstruktor kvuli STL
	Intersection(Vertex &v, Skeleton &s);          // Vypocet pruseciku pro dany vrchol
	Ogre::Vector2 poi;                         // Souradnice pruseciku
	Vertex *leftVertex, *rightVertex;  // 2 vrcholy, jejichz bisektory vytvorily tento prusecik
	Ogre::Real height;                     // vzdalenost od nositelky
	enum Type { CONVEX, NONCONVEX } type;

	bool operator < (const Intersection &i) const { return lt(height, i.height); }  // pro usporadani v prioritni fronte
	bool operator > (const Intersection &i) const { return gt(height, i.height); }  // pro usporadani v prioritni fronte
	bool operator == (const Intersection &i) const { return equ(poi, i.poi); }
};
typedef priority_queue <Intersection, deque <Intersection>, greater <Intersection> > IntersectionQueue;


class VertexList : public list <Vertex>
{
  public:
    VertexList (void) { }
    iterator prev (const iterator &i) { iterator tmp (i); if (tmp == begin ()) tmp = end (); tmp --; return tmp; } // => cyklicky
    iterator next (const iterator &i) { iterator tmp (i); tmp ++; if (tmp == end ()) tmp = begin (); return tmp; } // => cyklicky
    void push_back (const Vertex& x)
	{
		assert (x.prevVertex == NULL || facingTowards (x.leftLine, x.prevVertex -> rightLine));
		assert (x.nextVertex == NULL || facingTowards (x.rightLine, x.nextVertex -> leftLine));
		((Vertex &)x).ID = size ();       // automaticke cislovani
		list <Vertex> :: push_back (x);
	}
};

typedef vector <Ogre::Vector2>   Contour;
typedef vector <Contour> ContourVector;



class Skeleton : public list <SkeletonLine>
{
private: 
	IntersectionQueue iq;          // prioritni fronta pruseciku setridena podle vzdalenosti od nositelky (tj. podle vysky ve strese)
	VertexList        vl;          // SLAV, jednotlive cykly (LAV) jsou udrzovany pomoci ukazatelu nextVertex, prevVertex

public:
	Skeleton(Contour &points) { makeSkeleton(points); }

	void push_back (const SkeletonLine &x)
	{
		((SkeletonLine &)x).ID = size ();     // automaticke cislovani
		list <SkeletonLine> :: push_back (x);
	}

	void applyConvexIntersection(const Intersection &i);
	void applyLast3(const Intersection &i);
	void applyNonconvexIntersection(Intersection &i);
	VertexList& getVertexList() { return vl; }
	bool invalidIntersection(const Vertex &v, const Intersection &is);
	void makeSkeleton(ContourVector &contours);
	void makeSkeleton(Contour &points);
	Ogre::Real nearestIntersection(const Vertex &v, Vertex **left, Vertex **right, Ogre::Vector2 &p);
};

}

#endif
