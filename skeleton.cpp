#include "stdafx.h"
#include <math.h>
#include <assert.h>
#include "prim.h"
#include "skeleton.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>
#include <queue>
#include <list>
#include <stack>
using namespace std;
using namespace Felkel;


/******************************************************************************************************************************\
**                                             Operace pro vystup                                                             **
\******************************************************************************************************************************/

inline ostream &operator << (ostream &os, const Vertex &v)
{
  os << v.point.x << ' ' << v.point.y << ' ';
  if (v.advancingSkeletonLine) os << v.advancingSkeletonLine -> ID;
                        else os << -1;
  os << endl;
  return os;
}

inline ostream &operator << (ostream &os, const VertexList &v)
{
  VertexList :: const_iterator i = v.begin ();
  os << v.size () << endl;
  while (i != v.end ()) { os << *i; i++; }
  return os;
}

inline ostream &operator << (ostream &os, const SkeletonLine &sl)
{
  os << sl.lower.vertexID () << ' ' << sl.higher.vertexID () << ' '
     << sl.lower.leftID ()   << ' ' << sl.lower.rightID ()   << ' '
     << sl.higher.leftID ()  << ' ' << sl.higher.rightID ()  << endl;
  return os;
}

inline ostream &operator << (ostream &os, const Skeleton &s)
{
  os << s.size () << endl;
  for (Skeleton :: const_iterator si = s.begin (); si != s.end (); si++)
       os << *si;
  return os;
}

/*****************************************************************************************************************************\
**                                                      Funkce algoritmu                                                     **
\*****************************************************************************************************************************/

Vertex :: Vertex (const Ogre::Vector2 &p, Vertex &left, Vertex &right)   // tvorba vrcholu na miste pruseciku (p)
        : point (p), done (false), higher (NULL), ID (-1), leftSkeletonLine (NULL), rightSkeletonLine (NULL),
          advancingSkeletonLine (NULL)
{
  this -> leftLine = left.leftLine;    // hrany puvodni kontury, jejichz osou bude vytvareny bisektor vedouci z tohoto vrcholu
  this -> rightLine = right.rightLine;
  this -> leftVertex = &left;
  this -> rightVertex = &right;

  assert (equ(dist (point, leftLine),dist (point, rightLine))); // vytvareny vchol musi byt stejne daleko od obou hran
  Ogre::Vector2 i = intersection (leftLine, rightLine);                       // pro urceni smeru bisektoru je potreba znat souradnice
  if (equ(i.x, INFINITY))            // prusecik neni smerem dopredu     // jeste jednoho bodu, vhodnym adeptem je prusecik nositelek
     {                                                                // hran puvodni kontury
       assert(equ(i.y, INFINITY));
       i = intersectionAnywhere(leftLine, rightLine);                // Anywhere => prusecik primek a ne poloprimek
       if (equ(i.x, INFINITY))       // rovnobezne hrany
          {
            assert(equ(i.y, INFINITY));
            axis = Ray (point, leftLine.angle);                       // tvorba bisektoru pri rovnobeznych hranach => || s nimi
          }
       else                       // prusecik smerem dozadu
          {
            axis = Ray (point, i);
            axis.angle = axis.angle + Ogre::Math::PI;                                       // tvorba bisektoru
            normalizeAngle (axis.angle);
          }
     }
  else                            // prusecik smerem dopredu
     {
       axis = Ray (point, i);                                         // tvorba bisektoru
     }
}

bool intersectionFromLeft(const Ray &l, const Vertex &v) // pouze pro ladeni
{
  if (!equ(intersection (l, v.axis), INFINITEPOINT)) return false;
  if (v.rightVertex == &v) return false;
  if (!equ(intersection (l, v.rightVertex -> axis), INFINITEPOINT)) return true;
  return intersectionFromLeft (l, *v.rightVertex);
}

bool intersectionFromRight (const Ray &l, const Vertex &v) // pouze pro ladeni
{
  if (!equ(intersection(l, v.axis), INFINITEPOINT)) return false;
  if (v.leftVertex == &v) return false;
  if (!equ(intersection(l, v.leftVertex -> axis), INFINITEPOINT)) return true;
  return intersectionFromRight(l, *v.leftVertex);
}

Ogre::Vector2 coordinatesOfAnyIntersectionOfTypeB (const Vertex &v, const Vertex &left, const Vertex &right)
{                                         // vrati souradnice pruseciku
  Ogre::Vector2 p1 = intersectionAnywhere(v.rightLine, right.leftLine);
  Ogre::Vector2 p2 = intersectionAnywhere(v.leftLine, left.rightLine);
  Ogre::Vector2 poi;
  if (!equ(p1, INFINITEPOINT) && !equ(p2, INFINITEPOINT))
     {
       if (pointOnRay (p1, v.rightLine)) return INFINITEPOINT;
       if (pointOnRay (p2, v.leftLine))  return INFINITEPOINT;
       poi = intersectionAnywhere (angleAxis (p1, p2, v.point), v.axis);     // puleni uhlu a zjisteni pruseciku s bisektorem od V
     }
  else //if (p1 != INFINITEPOINT)                               // specialni pripad rovnobeznosti
     {
       poi = intersectionAnywhere (left.rightLine, v.axis);
       poi.x = (poi.x + v.point.x) / 2;                                      // to je pak prusecik na polovicni vzdalenosti
       poi.y = (poi.y + v.point.y) / 2;                                      // mezi vrcholem V a protilehlou hranou
     }
  return poi;
}

Ogre::Vector2 intersectionOfTypeB (const Vertex &v, const Vertex &left, const Vertex &right)
{                          // vrati souradnice pruseciku jen pokud je platny (lezi ve vyseci)
  assert (v.prevVertex == NULL || facingTowards (v.leftLine, v.prevVertex -> rightLine));
  assert (v.nextVertex == NULL || facingTowards (v.rightLine, v.nextVertex -> leftLine));
  assert (left.prevVertex == NULL || facingTowards (left.leftLine, left.prevVertex -> rightLine));
  assert (left.nextVertex == NULL || facingTowards (left.rightLine, left.nextVertex -> leftLine));
  assert (right.prevVertex == NULL || facingTowards (right.leftLine, right.prevVertex -> rightLine));
  assert (right.nextVertex == NULL || facingTowards (right.rightLine, right.nextVertex -> leftLine));

  Ogre::Vector2 pl (intersection (v.axis, left.rightLine));       // test protne-li bisektor danou hranu
  Ogre::Vector2 pr (intersection (v.axis, right.leftLine));
  if (equ(pl, INFINITEPOINT) && equ(pr, INFINITEPOINT))
      return INFINITEPOINT;                  // lezi-li hrana "za" bisektorem, inidkuje se neuspech

  Ogre::Vector2 p;
  if (!equ(pl, INFINITEPOINT)) p = pl;
  if (!equ(pr, INFINITEPOINT)) p = pr;
  assert (!equ(p, INFINITEPOINT));
  assert (equ(pl, INFINITEPOINT) || equ(pr, INFINITEPOINT) || equ(pl, pr));

  Ogre::Vector2 poi = coordinatesOfAnyIntersectionOfTypeB (v, left, right);    // zjisteni souradnic potencialniho bodu
  Ogre::Real al = left.axis.angle - left.rightLine.angle;                  // uhly urcujici vysec, kam musi bod poi padnout
  Ogre::Real ar = right.axis.angle - right.leftLine.angle;                 //

  Ogre::Real alp = Ray (left.point, poi).angle - left.rightLine.angle;     // uhly k bodu poi
  Ogre::Real arp = Ray (right.point, poi).angle - right.leftLine.angle;    //

  normalizeAngle (al); normalizeAngle (ar); normalizeAngle (alp); normalizeAngle (arp);
  assert(lte(al, 0.0));
  assert(gte(ar, 0.0) || equ(ar, -Ogre::Math::PI));

  if ((gt(alp, 0.0) || lt(alp, al)) && !ANGLE_SIMILAR (alp, 0) && !ANGLE_SIMILAR (alp, al))  // porovnani uhlu
      return INFINITEPOINT;                                             // a ignorovani bodu lezicich mimo vysec
  if ((lt(arp, 0.0) || gt(arp, ar)) && !ANGLE_SIMILAR (arp, 0) && !ANGLE_SIMILAR (arp, ar))  // porovnani uhlu
      return INFINITEPOINT;                                             // a ignorovani bodu lezicich mimo vysec
  return poi;            // doslo-li se az sem, lezi bod ve vyseci
}



Ogre::Real Skeleton::nearestIntersection(const Vertex &v, Vertex **left, Vertex **right, Ogre::Vector2 &p) // vrati nejblizsi z pruseciku typu B
{
  Ogre::Real minDist = INFINITY;                    // neplatna hodnota
  VertexList :: iterator minI = vl.end ();      // neplatny iterator
  VertexList :: iterator i;
  for (i = vl.begin (); i != vl.end (); i++)       // iterace pres vsechny LAV
      {
        if ((*i).done) continue;                                          // vynechani jiz zpracovanych vrcholu
        if ((*i).nextVertex == NULL || (*i).prevVertex == NULL) continue; // osamely vrchol na spicce - neni v LAV
        if (&*i == &v || (*i).nextVertex == &v) continue;                 // ignorovani hran vychazejicich z V
        assert ((*i).rightVertex != NULL);
        assert ((*i).leftVertex != NULL);
        Ogre::Vector2 poi = intersectionOfTypeB (v, (*i), *(*i).nextVertex);      // zjisteni souradnic potencialniho bodu B
        if(equ(poi, INFINITEPOINT)) continue;                  // vetsinou - nelezi-li bod ve spravne vyseci
        Ogre::Real d = dist(poi, v.point);                                   // zjisteni vzdalenosti od vrcholu
        if(lt(d, minDist)) { minDist = d; minI = i; }                       // a vyber nejblizsiho
      }
  if (equ(minDist, INFINITY)) return INFINITY;                               // nenalezen zadny vhodny bod B

  i = minI;
  Ogre::Vector2 poi = coordinatesOfAnyIntersectionOfTypeB (v, (*i), *(*i).nextVertex);

  Ogre::Real d = dist (poi, v.leftLine);               // zjisteni vzdalenosti vrcholu V od nositelky (vyska ve strese)
  assert(equ(d, dist (poi, v.rightLine)));
  assert(equ(d, dist (poi, (*i).rightLine)));
  assert(equ(d, dist (poi, (*i).nextVertex -> leftLine)));

  p = poi;                                         // nastaveni navracenych hodnot
  *left = (Vertex *) &*i;
  *right = (*i).nextVertex;

  return d;                                        // vysledkem je vzdalenost od nositelky
}

bool Skeleton::invalidIntersection(const Vertex &v, const Intersection &is)        //
{
  for (VertexList :: iterator i = vl.begin (); i != vl.end (); i++)
      {
        if ((*i).done) continue;
        if ((*i).nextVertex == NULL || (*i).prevVertex == NULL) continue; // osamely vrchol na spicce
        Ogre::Vector2 poi = intersection (v.axis, (*i).axis);
        if (equ(poi, INFINITEPOINT)) continue;
        if (&*i == is.leftVertex || &*i == is.rightVertex) continue;

        Ogre::Real dv = dist (poi, v.leftLine);
        Ogre::Real dvx = dist (poi, v.rightLine);
//        assert (SIMILAR (dv, dist (poi, v.rightLine)));
        assert (equ(dv, dvx));
        if (gte(dv, is.height)) continue;

        Ogre::Real di = dist (poi, (*i).leftLine);
        assert(equ(di, dist (poi, (*i).rightLine)));
        if (gt(di, dv + MIN_DIFF)) continue;
//        if (di > is.height) continue;

        return true;
      }
  return false;
}

Intersection::Intersection(Vertex &v, Skeleton &s)      // spocitani nejnizsiho pruseciku pro vrchol v
{
  assert (v.prevVertex == NULL || facingTowards (v.leftLine, v.prevVertex -> rightLine));
  assert (v.nextVertex == NULL || facingTowards (v.rightLine, v.nextVertex -> leftLine));

  Vertex &l = *v.prevVertex;       // sousedi v LAV
  Vertex &r = *v.nextVertex;

  assert (equ(v.leftLine.angle, v.leftVertex -> leftLine.angle));
  assert (equ(v.rightLine.angle, v.rightVertex -> rightLine.angle));

  Ogre::Real al = v.axis.angle - l.axis.angle;
  normalizeAngle (al);
  Ogre::Real ar = v.axis.angle - r.axis.angle;
  normalizeAngle (ar);
  Ogre::Vector2 i1 = facingTowards (v.axis, l.axis) ? INFINITEPOINT : intersection (v.axis, l.axis); // pruseciky se sousedn.
  Ogre::Vector2 i2 = facingTowards (v.axis, r.axis) ? INFINITEPOINT : intersection (v.axis, r.axis); // vrcholy - edge event
  Ogre::Real d1 = dist (v.point, i1);     // d1 - vzdalenost od nositelky pruseciku bisektoru s bisektorem leveho souseda v LAV
  Ogre::Real d2 = dist (v.point, i2);     // d2 - vzdalenost od nositelky pruseciku bisektoru s bisektorem praveho souseda v LAV

  Vertex *leftPointer, *rightPointer;
  Ogre::Vector2 p;
  Ogre::Real d3 = INFINITY;               // d3 - u nekonvexnich vrcholu vzdalenost bodu B od nositelky
  Ogre::Real av = v.leftLine.angle - v.rightLine.angle;     // test na otevrenost uhlu u vrcholu V => konvexni/nekonvexni
  normalizeAngle (av);
  if (gt(av, 0.0) &&
     (equ(intersection (v.leftLine, v.rightLine), v.point) || equ(intersection (v.leftLine, v.rightLine), INFINITEPOINT)))
      d3 = s.nearestIntersection (v, &leftPointer, &rightPointer, p);   // zjisteni vzdalenosti bodu B od nositelky

  // vyber nejnizsiho bodu a nastaveni spravnych hodnot do prave vytvareneho pruseciku
       if(lte(d1, d2) && lte(d1, d3)) { leftVertex = &l; rightVertex = &v; poi = i1; type = CONVEX; height = dist (v.leftLine, i1); }
  else if(lte(d2, d1) && lte(d2, d3)) { leftVertex = &v; rightVertex = &r; poi = i2; type = CONVEX; height = dist (v.rightLine, i2); }
  else if(lte(d3, d1) && lte(d3, d2)) { poi = p; leftVertex = rightVertex = &v; type = NONCONVEX; height = d3; }

  if (equ(poi, INFINITEPOINT)) height = INFINITY;
  if (type == NONCONVEX && s.invalidIntersection (v, *this)) height = INFINITY;
}

void Skeleton::applyNonconvexIntersection (Intersection &i)  // byl-li nejnizsi prusecik ve fronte nekonvexniho typu ...
{
  assert (i.leftVertex == i.rightVertex);          // nekonvexni typ pruseciku ukazuje jen na jeden vrchol, z nehoz byl vytvoren

  Vertex *leftPointer, *rightPointer;              // dva konce protilehle hrany
  Ogre::Vector2 p;                                         // souradnice pruseciku
  Ogre::Real d3 = INFINITY;                                                     // opetovne nalezeni pruseciku a koncovych bodu
  d3 = nearestIntersection (*i.leftVertex, &leftPointer, &rightPointer, p); // protilehle hrany - nutne kvuli moznosti vicekrat
  if(equ(d3, INFINITY)) return;                                               // delene jedne hrany.
                                                   // pri soubehu vice "ramen" kostry muze nastat situace, kdy je puvodni
  if(!equ(p, i.poi)) return;                          // protilehla hrana jiz zpracovana, pak se novy prusecik nalezne jinde,
                                                   // a puvodni tedy jiz nema vyznam => return.
  Vertex v1(p, *rightPointer, *i.rightVertex);    // zalozeni dvou novych vrcholu na miste pruseciku + spocitani bisektoru
  Vertex v2(p, *i.leftVertex, *leftPointer);

  assert(!equ(v1.point, INFINITEPOINT));
  assert(!equ(v2.point, INFINITEPOINT));

  i.leftVertex -> done = true;                     // oznaceni puvodniho vrcholu za zpracovany
//  i.rightVertex -> done = true;

  Vertex *newNext1 = i.rightVertex -> nextVertex;  // zapojeni prvniho vrcholu do LAV
  Vertex *newPrev1 = leftPointer -> highest ();
  v1.prevVertex = newPrev1;
  v1.nextVertex = newNext1;
  vl.push_back (v1);
  Vertex *v1Pointer = &vl.back ();
  newPrev1 -> nextVertex = v1Pointer;
  newNext1 -> prevVertex = v1Pointer;
  i.rightVertex -> higher = v1Pointer;

  Vertex *newNext2 = rightPointer -> highest ();   // zapojeni druheho vrcholu do LAV
  Vertex *newPrev2 = i.leftVertex -> prevVertex;
  v2.prevVertex = newPrev2;
  v2.nextVertex = newNext2;
  vl.push_back (v2);
  Vertex *v2Pointer = &vl.back ();
  newPrev2 -> nextVertex = v2Pointer;
  newNext2 -> prevVertex = v2Pointer;
  i.leftVertex -> higher = v2Pointer;

  push_back (SkeletonLine (*i.rightVertex, *v1Pointer)); // tvorba kostry - okridlene hrany
  SkeletonLine *linePtr = &back ();
  push_back (SkeletonLine (*v1Pointer, *v2Pointer));     // nulova delka - pomocna hrana
  SkeletonLine *auxLine1Ptr = &back ();
  push_back (SkeletonLine (*v2Pointer, *v1Pointer));     // nulova delka - pomocna hrana
  SkeletonLine *auxLine2Ptr = &back ();

  linePtr -> lower.right = i.leftVertex -> leftSkeletonLine;      // navazani okridlenych hran do kostry
  linePtr -> lower.left = i.leftVertex -> rightSkeletonLine;

  v1Pointer -> rightSkeletonLine = v2Pointer -> leftSkeletonLine = linePtr;
  v1Pointer -> leftSkeletonLine = auxLine1Ptr;
  v2Pointer -> rightSkeletonLine = auxLine2Ptr;

  auxLine1Ptr -> lower.right = auxLine2Ptr;
  auxLine2Ptr -> lower.left = auxLine1Ptr;

  if (i.leftVertex -> leftSkeletonLine) i.leftVertex -> leftSkeletonLine -> higher.left = linePtr;
  if (i.leftVertex -> rightSkeletonLine) i.leftVertex -> rightSkeletonLine -> higher.right = linePtr;
  i.leftVertex -> advancingSkeletonLine = linePtr;

  if (newNext1 == newPrev1)                                 // specialni pripad, kdy by novy LAV obsahoval pouze 2 vrcholy
     {
       v1Pointer -> done = true;                            // oba oznacit jako zpracovane
       newNext1 -> done = true;
       push_back (SkeletonLine (*v1Pointer, *newNext1)); // a doplnit kostru o jejich spojnici
       SkeletonLine *linePtr = &back ();
       linePtr -> lower.right  = v1Pointer -> leftSkeletonLine;
       linePtr -> lower.left   = v1Pointer -> rightSkeletonLine;
       linePtr -> higher.right = newNext1 -> leftSkeletonLine;
       linePtr -> higher.left  = newNext1 -> rightSkeletonLine;

       if (v1Pointer -> leftSkeletonLine)  v1Pointer -> leftSkeletonLine  -> higher.left  = linePtr;
       if (v1Pointer -> rightSkeletonLine) v1Pointer -> rightSkeletonLine -> higher.right = linePtr;
       if (newNext1 -> leftSkeletonLine)   newNext1  -> leftSkeletonLine  -> higher.left  = linePtr;
       if (newNext1 -> rightSkeletonLine)  newNext1  -> rightSkeletonLine -> higher.right = linePtr;
     }
  else
     {
       Intersection i1(*v1Pointer, *this);                        // tvorba noveho pruseciku pro bisektor vedouci z 1. noveho vrcholu
       if (!equ(i1.height, INFINITY)) iq.push (i1);             // a ulozeni do fronty
     }
  if (newNext2 == newPrev2)                                 // specialni pripad, kdy by novy LAV obsahoval pouze 2 vrcholy
     {
       v2Pointer -> done = true;                            // oba oznacit jako zpracovane
       newNext2 -> done = true;
       push_back (SkeletonLine (*v2Pointer, *newNext2)); // a doplnit kostru o jejich spojnici
       SkeletonLine *linePtr = &back ();
       linePtr -> lower.right  = v2Pointer -> leftSkeletonLine;
       linePtr -> lower.left   = v2Pointer -> rightSkeletonLine;
       linePtr -> higher.right = newNext2 -> leftSkeletonLine;
       linePtr -> higher.left  = newNext2 -> rightSkeletonLine;

       if (v2Pointer -> leftSkeletonLine)  v2Pointer -> leftSkeletonLine  -> higher.left  = linePtr;
       if (v2Pointer -> rightSkeletonLine) v2Pointer -> rightSkeletonLine -> higher.right = linePtr;
       if (newNext2 -> leftSkeletonLine)   newNext2  -> leftSkeletonLine  -> higher.left  = linePtr;
       if (newNext2 -> rightSkeletonLine)  newNext2  -> rightSkeletonLine -> higher.right = linePtr;
     }
  else
     {
       Intersection i2 (*v2Pointer, *this);                        // tvorba noveho pruseciku pro bisektor vedouci z 2. noveho vrcholu
       if(!equ(i2.height, INFINITY)) iq.push (i2);             // a ulozeni do fronty
     }
}

void Skeleton::applyConvexIntersection(const Intersection &i)        // byl-li nejnizsi prusecik ve fronte konvexniho typu ...
{
  Vertex vtx (i.poi, *i.leftVertex, *i.rightVertex);        // vytvoreni noveho vrcholu na miste pruseciku + spocitani bisektoru
  assert(!equ(vtx.point, INFINITEPOINT));

  Vertex *newNext = i.rightVertex -> nextVertex;            // zapojeni vrcholu do LAV
  Vertex *newPrev = i.leftVertex -> prevVertex;
  vtx.prevVertex = newPrev;
  vtx.nextVertex = newNext;
  vl.push_back (vtx);
  Vertex *vtxPointer = &vl.back ();
  newPrev -> nextVertex = vtxPointer;
  newNext -> prevVertex = vtxPointer;
  i.leftVertex  -> higher = vtxPointer;
  i.rightVertex -> higher = vtxPointer;

  i.leftVertex  -> done = true;                             // oznaceni starych vrcholu za zpracovane
  i.rightVertex -> done = true;

  Intersection newI (*vtxPointer, *this);                          // spocitani nejnizsiho pruseciku pro bisektor vedouci v noveho vrcholu
  if (!equ(newI.height, INFINITY)) iq.push (newI);

  push_back (SkeletonLine (*i.leftVertex, *vtxPointer));  // ulozeni dvou car do kostry
  SkeletonLine *lLinePtr = &back ();
  push_back (SkeletonLine (*i.rightVertex, *vtxPointer));
  SkeletonLine *rLinePtr = &back ();

  lLinePtr -> lower.right = i.leftVertex -> leftSkeletonLine;      // a pospojovani okridlenych hran kostry
  lLinePtr -> lower.left = i.leftVertex -> rightSkeletonLine;
  lLinePtr -> higher.right = rLinePtr;
  rLinePtr -> lower.right = i.rightVertex -> leftSkeletonLine;
  rLinePtr -> lower.left = i.rightVertex -> rightSkeletonLine;
  rLinePtr -> higher.left = lLinePtr;

  if (i.leftVertex -> leftSkeletonLine) i.leftVertex -> leftSkeletonLine -> higher.left = lLinePtr;
  if (i.leftVertex -> rightSkeletonLine) i.leftVertex -> rightSkeletonLine -> higher.right = lLinePtr;

  if (i.rightVertex -> leftSkeletonLine) i.rightVertex -> leftSkeletonLine -> higher.left = rLinePtr;
  if (i.rightVertex -> rightSkeletonLine) i.rightVertex -> rightSkeletonLine -> higher.right = rLinePtr;

  vtxPointer -> leftSkeletonLine = lLinePtr;
  vtxPointer -> rightSkeletonLine = rLinePtr;

  i.leftVertex -> advancingSkeletonLine = lLinePtr;
  i.rightVertex -> advancingSkeletonLine = rLinePtr;
}

void Skeleton::applyLast3(const Intersection &i)                            // zpracovani LAVu obsahujici 3 vrcholy => spicka strechy
{
  assert (i.leftVertex  -> nextVertex == i.rightVertex);           // overeni korektnosti propojeni LAVu
  assert (i.rightVertex -> prevVertex == i.leftVertex);
  assert (i.leftVertex  -> prevVertex -> prevVertex == i.rightVertex);
  assert (i.rightVertex -> nextVertex -> nextVertex == i.leftVertex);

  Vertex &v1 = *i.leftVertex;
  Vertex &v2 = *i.rightVertex;
  Vertex &v3 = *i.leftVertex -> prevVertex;
  v1.done = true;                                                  // oznaceni vsech tri vrcholu za zpracovane
  v2.done = true;
  v3.done = true;

  Ogre::Vector2 is1 = facingTowards (v1.axis, v2.axis) ? INFINITEPOINT : intersection (v1.axis, v2.axis);
  Ogre::Vector2 is2 = facingTowards (v2.axis, v3.axis) ? INFINITEPOINT : intersection (v2.axis, v3.axis);
  Ogre::Vector2 is3 = facingTowards (v3.axis, v1.axis) ? INFINITEPOINT : intersection (v3.axis, v1.axis);

  Ogre::Vector2 is = i.poi;                                                // souradnice spicky
  assert (equ(is, is1) || equ(is1, INFINITEPOINT));
  //assert (equ(is, is2) || equ(is2, INFINITEPOINT));
  assert (equ(is, is3) || equ(is3, INFINITEPOINT));

  Vertex v (is);                                                   // zalozeni noveho vrcholu na spicce, bisektor se nepocita

  v.done = true;                                                   // i novy vrchol jei nebude dale zpracovavan
  vl.push_back (v);
  Vertex *vtxPointer = &vl.back ();
  push_back (SkeletonLine (v1, *vtxPointer));             // ulozeni tri car do kostry
  SkeletonLine *line1Ptr = &back ();
  push_back (SkeletonLine (v2, *vtxPointer));
  SkeletonLine *line2Ptr = &back ();
  push_back (SkeletonLine (v3, *vtxPointer));
  SkeletonLine *line3Ptr = &back ();

  line1Ptr -> higher.right = line2Ptr;                             // zapojeni okridlenych hran
  line2Ptr -> higher.right = line3Ptr;
  line3Ptr -> higher.right = line1Ptr;

  line1Ptr -> higher.left = line3Ptr;
  line2Ptr -> higher.left = line1Ptr;
  line3Ptr -> higher.left = line2Ptr;

  line1Ptr -> lower.left = v1.rightSkeletonLine;
  line1Ptr -> lower.right = v1.leftSkeletonLine;

  line2Ptr -> lower.left = v2.rightSkeletonLine;
  line2Ptr -> lower.right = v2.leftSkeletonLine;

  line3Ptr -> lower.left = v3.rightSkeletonLine;
  line3Ptr -> lower.right = v3.leftSkeletonLine;

  if (v1.leftSkeletonLine) v1.leftSkeletonLine -> higher.left = line1Ptr;
  if (v1.rightSkeletonLine) v1.rightSkeletonLine -> higher.right = line1Ptr;

  if (v2.leftSkeletonLine) v2.leftSkeletonLine -> higher.left = line2Ptr;
  if (v2.rightSkeletonLine) v2.rightSkeletonLine -> higher.right = line2Ptr;

  if (v3.leftSkeletonLine) v3.leftSkeletonLine -> higher.left = line3Ptr;
  if (v3.rightSkeletonLine) v3.rightSkeletonLine -> higher.right = line3Ptr;

  v1.advancingSkeletonLine = line1Ptr;
  v2.advancingSkeletonLine = line2Ptr;
  v3.advancingSkeletonLine = line3Ptr;
}




void Skeleton::makeSkeleton(ContourVector &contours)          // hlavni smycka vytvarejici kostru
{
	while (iq.size ()) iq.pop ();                               // vymazani globalnich kontajneru
	vl.clear();
	clear();

	// for each contour
	for (size_t ci = 0; ci < contours.size (); ci ++)              // pro kazdou samostatnou konturu, tj. pro obrys i diry
	{
		// extract polygon boundary cycle
		// note: will stop extracting if a point is similar/close to first
		Contour &points = contours [ci];        
		while(equ(points.front(), points.back()) && points.size () > 0)
			points.pop_back ();

		// remove any duplicate points (any two points close enough to be similar)
		Contour :: iterator first = points.begin();
		if (first == points.end ()) break;                    // prazdny seznam bodu -> prazdny seznam hran
		Contour :: iterator next = first;
		while (++next != points.end ())
		{
			if (equ(*first, *next)) points.erase (next);
			else first = next;
			next = first;
		}

		// create vertex list
		size_t s = points.size ();
		for (size_t f = 0; f < s; f++)
		{
			// vertexlist.push_back(point, prev_point, next_point);
			vl.push_back (Vertex (points [f], points [(s+f-1)%s], points [(s+f+1)%s]));  // zaroven spocita bisektory
		}
	}

	// needs at least 3 points (otherwise its a line not a polygon!)
	if (vl.size () < 3) return; 

	// provazani vrcholu - tvorba jednotlivych LAVu ve SLAV
	VertexList :: iterator i;
	size_t vn = 0, cn = 0;
	VertexList :: iterator contourBegin;
	for (i = vl.begin (); i != vl.end (); i++)
    {
		(*i).prevVertex = &*vl.prev(i);         // ukazatele obsahuji adresy vrcholu jiz ulozenych do seznamu
		(*i).nextVertex = &*vl.next(i);         // => lze je navazat az ted, kdy se jejich adresa uz nebude menit
		(*i).leftVertex = &*i;
		(*i).rightVertex = &*i;
		if (vn == 0) contourBegin = i;
		if (vn == contours[cn].size() - 1)
		{
			(*i).nextVertex = &*contourBegin;
			(*contourBegin).prevVertex = &*i;
			vn = 0;
			cn ++;
		}
		else vn ++;
	}

	// vytvoreni fronty pruseciku - pro kazdy vrchol jeden prusecik
	for (i = vl.begin (); i != vl.end (); i++)
	{
		if (!(*i).done)
		{
			Intersection is(*i, *this);
			if (!equ(is.height, INFINITY)) iq.push(is);  // ulozeni do prioritni fronty
		}
	}

  while (iq.size ())                              // hlavni cyklus - dokud je nejaky prusecik ve fronte
        {
          Intersection i = iq.top ();             // vyzvednuti nejnizsiho pruseciku
          iq.pop ();

          if (i.leftVertex -> done && i.rightVertex -> done) continue;   // pokud byly vrcholy ukazovane prusecikem zpracovany
                                                                         // v predchozich iteracich => vzit dalsi prusecik
          if (i.leftVertex -> done || i.rightVertex -> done)             // velmi ridka situace, kdy byl zpracovan pouze jeden
             {                                                           // z vrcholu
               if (!i.leftVertex -> done) iq.push (Intersection (*i.leftVertex, *this));    // pro ten nezpracovany je spocitan novy
               if (!i.rightVertex -> done) iq.push (Intersection (*i.rightVertex, *this));  // prusecik a je ulozen do fronty
               continue;                                                 // a aktualni prusecik jiz neni zpracovan
             }

          assert (i.leftVertex -> prevVertex != i.rightVertex);
          assert (i.rightVertex -> nextVertex != i.leftVertex);
          if (i.type == Intersection :: CONVEX)
              if (i.leftVertex -> prevVertex -> prevVertex == i.rightVertex ||
                  i.rightVertex -> nextVertex -> nextVertex == i.leftVertex)
                  applyLast3 (i);                                        // zpracovani pripadu, kdy v LAVu zbyvaji jen 3 vrcholy
             else applyConvexIntersection (i);                           // zpracovani EDGE EVENT
          if (i.type == Intersection :: NONCONVEX)
              applyNonconvexIntersection (i);                            // nebo zpracovani SPLIT EVENT
        }
  return;                                                       // vraci se odkaz na vytvorenou kostru
}

void Skeleton::makeSkeleton(Contour &points)  // zkratka pokud je tvorena kostra pro jedinou konturu bez der
{
  ContourVector vv;
  vv.push_back (points);
  return makeSkeleton (vv);
}

/*****************************************************************************************************************************\
**                                                 To test the implementation ...                                            **
\*****************************************************************************************************************************/


int buildContourFromStream (istream &stream, Contour &result) // nacteni jedne kontury ze streamu, vraci pocet bodu
{
  result.erase (result.begin (), result.end ());
  int count;
  if (stream.eof ()) return EOF;
  stream >> count;                                                     // nejdriv pocet vrcholu
  if (stream.eof ()) return EOF;
  for (int f = 0; f < count; f++)
      {
        Ogre::Real x, y;
        stream >> x;                                                   // pote x a y souradnice jednotlivych bodu
        if (stream.eof ()) return EOF;
        stream >> y;
        if (stream.eof ()) return EOF;
        result.push_back (Ogre::Vector2 (x, y));
      }
  return count;
}

int buildContourVectorFromStream (istream &stream, ContourVector &result) // nacteni seznamu kontur ze streamu - i diry
{
  int total = 0;
  result.erase (result.begin (), result.end ());
  int count = 0;
  if (stream.eof ()) return EOF;
  stream >> count;                                                     // nejdriv pocet kontur
  if (stream.eof ()) return EOF;
  for (int f = 0; f < count; f++)
        {
          result.push_back (Contour ());                               // pak jednotlive kontury
          total += buildContourFromStream (stream, result [f]);
        }
  return total;
}

void outFace (Vertex &v, ostream &stream)
{
  SkeletonLine *l = v.advancingSkeletonLine;
  if (l == NULL) { cerr << "Integrity error !!!\n"; return; }
  SkeletonLine *next = l -> higher.right;
  Ogre::Vector2 last = l -> lower.vertex -> point;

  stream << last.x << ',' << last.y << ' ';
  do {
       if(!equ(last, l -> lower.vertex -> point))
          {
            last = l -> lower.vertex -> point;
            stream << last.x << ',' << last.y << ' ';
          }
       else if(!equ(last, l -> higher.vertex -> point))
          {
            last = l -> higher.vertex -> point;
            stream << last.x << ',' << last.y << ' ';
          }
       if (next == NULL) break;
            if (next -> lower .left == l) { l = next; next = next -> higher.right; }
       else if (next -> higher.left == l) { l = next; next = next -> lower.right; }
       else assert (false);
     } while (l);
  stream << endl;
}


/*
int main (int argc, const char ** argv)
{
  if (argc < 2) return 0;

  ifstream ifs (argv [1]);
  ContourVector v;
  buildContourVectorFromStream (ifs, v);
  ifs.close ();
  Skeleton s;
  s.makeSkeleton(v);


//  cout << "Skeleton:\n" << s;
  VertexList& vl(s.getVertexList());
  cout << "Skeleton:\n" << s << "\nVertices:\n" << vl;

  cout << "\nFaces:\n" << endl;
  for (VertexList :: iterator vi = vl.begin (); vi != vl.end (); vi++)
      {
        if ((*vi).atContour ()) outFace (*vi, cout);
      }
  system("PAUSE");
  return 0;
}
*/

