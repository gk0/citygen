#include "stdafx.h"
#include "prim.h"
#include <math.h>


Ray :: Ray (const Ogre::Vector2 &p, const Ogre::Vector2 &q)
          : origin (p)
{
  angle = Ogre::Real (atan2 (q.y - p.y, q.x - p.x));
  normalizeAngle (angle);
}

Ray angleAxis (const Ogre::Vector2 &b, const Ogre::Vector2 &a, const Ogre::Vector2 &c) // vrati osu uhlu abc prochazejici bodem b
{
  Ray ba (b, a);
  Ray bc (b, c);
  if(gt(ba.angle, bc.angle)) ba.angle = ba.angle - 2*Ogre::Math::PI;
  return Ray (b, (ba.angle + bc.angle) / 2);
}

Ogre::Vector2 intersection (const Ray& a, const Ray &b)
{
  if (equ(a.origin, b.origin)) return a.origin;
  if (pointOnRay (b.origin, a) && pointOnRay (a.origin, b))
      return Ogre::Vector2 ((a.origin.x + b.origin.x)/2, (a.origin.y + b.origin.y)/2);
  if (pointOnRay (b.origin, a)) return b.origin;
  if (pointOnRay (a.origin, b)) return a.origin;
  if (colinear (a, b)) return INFINITEPOINT;

  Ogre::Real sa = sinf(a.angle);
  Ogre::Real sb = sinf(b.angle);
  Ogre::Real ca = cosf(a.angle);
  Ogre::Real cb = cosf(b.angle);
  Ogre::Real x = sb*ca - sa*cb;
  if (equ(x, 0.0)) return INFINITEPOINT;
  Ogre::Real u = (cb*(a.origin.y - b.origin.y) - sb*(a.origin.x - b.origin.x))/x;
  if (lt(u,0.0)) return INFINITEPOINT;
  if (gt(((ca*(b.origin.y - a.origin.y) - sa*(b.origin.x - a.origin.x))/x), 0)) return INFINITEPOINT;
  return Ogre::Vector2 (a.origin.x + u*ca, a.origin.y + u*sa);
};

Ogre::Vector2 intersectionAnywhere (const Ray& a, const Ray &b)
{
  if (equ(a.origin, b.origin)) return a.origin;
  if (pointOnRay (b.origin, a) && pointOnRay (a.origin, b))
      return Ogre::Vector2 ((a.origin.x + b.origin.x)/2, (a.origin.y + b.origin.y)/2);
  if (pointOnRay (b.origin, a)) return b.origin;
  if (pointOnRay (a.origin, b)) return a.origin;
  if (pointOnRay (b.origin, a.opaque ()) && pointOnRay (a.origin, b.opaque ()))
      return Ogre::Vector2 ((a.origin.x + b.origin.x)/2, (a.origin.y + b.origin.y)/2);

  if (colinear (a, b)) return INFINITEPOINT;
  Ogre::Real sa = sin (a.angle);
  Ogre::Real sb = sin (b.angle);
  Ogre::Real ca = cos (a.angle);
  Ogre::Real cb = cos (b.angle);
  Ogre::Real x = sb*ca - sa*cb;
  if (equ(x, 0.0)) return INFINITEPOINT;
  Ogre::Real u = (cb*(a.origin.y - b.origin.y) - sb*(a.origin.x - b.origin.x))/x;
  return Ogre::Vector2 (a.origin.x + u*ca, a.origin.y + u*sa);
};

Ogre::Real dist (const Ogre::Vector2 &p, const Ogre::Vector2 &q)
{
  return sqrt ((p.x-q.x)*(p.x-q.x) + (p.y - q.y)*(p.y - q.y));
}

Ogre::Real dist (const Ogre::Vector2 &p, const Ray &l)
{
  Ogre::Real a = l.angle - Ray (l.origin, p).angle;
  Ogre::Real d = sin (a) * dist (l.origin, p);
  if (lt(d, 0.0)) return -d;
  return d;
}

Ogre::Real dist (const Ogre::Vector2 &p, const Segment &l)
{
  Ray hl1 (l.a, l.b);
  Ray hl2 (p, hl1.angle + Ogre::Math::PI/2);
  Ogre::Vector2 i = intersectionAnywhere (hl1, hl2);

  if (lt(i.x, l.a.x + MIN_DIFF) && gt(i.x, l.b.x - MIN_DIFF)) return dist (i, p);
  if (gt(i.x, l.a.x - MIN_DIFF) && lt(i.x, l.b.x + MIN_DIFF)) return dist (i, p);

  return min (dist (p, l.a), dist (p, l.b));
}

Ogre::Real &normalizeAngle (Ogre::Real &angle)
{
  if (gte(angle,  Ogre::Math::PI)) { angle = angle - 2*Ogre::Math::PI; return normalizeAngle (angle); }
  if (lt(angle, -Ogre::Math::PI)) { angle = angle + 2*Ogre::Math::PI; return normalizeAngle (angle); }
  return angle;
}

bool colinear (const Ray &a, const Ray &b)
{
  Ogre::Real aa = a.angle;
  Ogre::Real ba = b.angle;
  Ogre::Real aa2 = a.angle + Ogre::Math::PI;
  normalizeAngle (aa);
  normalizeAngle (ba);
  normalizeAngle (aa2);
  return equ(ba, aa) || equ(ba, aa2);
}

