#ifndef __PRIMITIVES_H_INCLUDED
#define __PRIMITIVES_H_INCLUDED

#include "assert.h"
#include <math.h>
#include <OgreVector2.h>
#include <iostream>
using namespace std;

#ifdef INFINITY
  #undef INFINITY
#endif

//#define Ogre::Math::PI            Ogre::Math::PI
#define INFINITY        std::numeric_limits<Ogre::Real>::max()
#define INFINITEPOINT	Ogre::Vector2(INFINITY, INFINITY)
#define MIN_DIFF        0.0005
#define MIN_ANGLE_DIFF  0.0000005


#define SIMILAR(a,b) ((a)-(b) < MIN_DIFF && (b)-(a) < MIN_DIFF)
#define ANGLE_SIMILAR(a,b) (normalizedAngle (a) - normalizedAngle (b) < MIN_ANGLE_DIFF && normalizedAngle (b) - normalizedAngle (a) < MIN_ANGLE_DIFF)

inline bool equ(const Ogre::Real &a, const Ogre::Real &b) { return SIMILAR(a, b); }
inline bool lt(const Ogre::Real &a, const Ogre::Real &b) { return a < b && !SIMILAR(a, b); }
inline bool gt(const Ogre::Real &a, const Ogre::Real &b) { return a > b && !SIMILAR(a,b); }
inline bool lte(const Ogre::Real &a, const Ogre::Real &b) { return a < b || SIMILAR(a,b); }
inline bool gte(const Ogre::Real &a, const Ogre::Real &b) { return a > b || SIMILAR(a,b); }

Ogre::Real &normalizeAngle (Ogre::Real &angle);
inline Ogre::Real normalizedAngle (Ogre::Real angle) { Ogre::Real temp = angle; normalizeAngle (temp); return temp; }

inline bool equ(const Ogre::Vector2 &a, const Ogre::Vector2 &b) { return equ(a.x, b.x) && equ(a.y, b.y); }


class Ray
{
  public:
    Ray (const Ogre::Vector2 &p = Ogre::Vector2 (0, 0), const Ogre::Vector2 &q = Ogre::Vector2 (0, 0));
    Ray (const Ogre::Vector2 &p, const Ogre::Real &a) : origin (p), angle (a) { normalizeAngle (angle); };
    Ray opaque (void) const { return Ray (origin, angle + Ogre::Math::PI); }

    Ogre::Vector2 origin;
    Ogre::Real angle;
};

Ray angleAxis (const Ogre::Vector2 &a, const Ogre::Vector2 &b, const Ogre::Vector2 &c); // vrati osu uhlu bac prochazejici bodem a
inline int pointOnRay (const Ogre::Vector2 &p, const Ray &r) { return equ(p, r.origin) || equ(Ray(r.origin, p).angle, r.angle); }
inline int facingTowards (const Ray &a, const Ray &b) { return pointOnRay(a.origin, b) && pointOnRay(b.origin, a) && !equ(a.origin, b.origin); }
bool colinear (const Ray &a, const Ray &b);
Ogre::Vector2 intersection (const Ray& a, const Ray &b);
Ogre::Vector2 intersectionAnywhere (const Ray& a, const Ray &b);


inline ostream &operator << (ostream &os, const Ray &l) { os << '[' << l.origin.x << "; " << l.origin.y << "; " << 180*l.angle/Ogre::Math::PI << ']'; return os; }

struct Segment
{
  Segment (const Ogre::Vector2 &p = Ogre::Vector2 (), const Ogre::Vector2 &q = Ogre::Vector2 ()) : a (p), b (q) { };
  Segment (Ogre::Real x1, Ogre::Real y1, Ogre::Real x2, Ogre::Real y2) : a (x1, y1), b (x2, y2) { }

  Ogre::Vector2 a, b;
};

Ogre::Real dist (const Ogre::Vector2 &p, const Ogre::Vector2 &q);
Ogre::Real dist (const Ogre::Vector2 &p, const Ray &l);
Ogre::Real dist (const Ogre::Vector2 &p, const Segment &l);


inline ostream &operator << (ostream &os, const Segment &l)
{
  os << "line (" << l.a << ", " << l.b << ")\n";
  return os;
}

inline Ogre::Real dist (const Ray &l, const Ogre::Vector2 &p)
{
  return dist (p, l);
}

inline Ogre::Real dist (const Segment &l, const Ogre::Vector2 &p)
{
  return dist (p, l);
}


#endif
