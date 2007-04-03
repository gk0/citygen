#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "stdafx.h"

class Cell;

class Geometry 
{
public:

	/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successfull
	 */
	static Ogre::Real dotPerp(const Ogre::Vector2& v0, const Ogre::Vector2& v1);

	/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successfull
	 */
	static bool pointInCell(const Ogre::Vector2& loc, const Cell& c);

		/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successfull
	 */
	static bool rayCross(const Ogre::Vector2& loc, const Ogre::Vector2& pt1, const Ogre::Vector2& pt2);


    /** Checks if the two line segments a,b & c,d intersect
		@remarks
			Algorithm defined at 
			http://www.faqs.org/faqs/graphics/algorithms-faq/
			OR
			Computational Geometry in C, Joeseph O'Rourke Pg.221 
        @param
            a, a Vector2 defines the start point of segment 1
        @param
            b, a Vector2 defines the end point of segment 1
        @param
            c, a Vector2 defines the start point of segment 2
        @param
            d, a Vector2 defines the end point of segment 2
		@param
			intersection a Vector2 reference store the resulting
			point of intersection if it occurs.
        @returns
            A bool indicating if the line segments intersect. 
    */
	static inline bool lineSegmentIntersect(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const Ogre::Vector2& c, const Ogre::Vector2& d, Ogre::Vector2& intersection)
	{
		Ogre::Vector2 BminusA(b - a);
		Ogre::Vector2 DminusC(d - c);
		Ogre::Real denom = (BminusA.x * DminusC.y) - (BminusA.y * DminusC.x);

		// line are parallel
		if(denom == 0) return false;

		Ogre::Vector2  AminusC(a - c);
		
		Ogre::Real r = ((AminusC.y * DminusC.x) - (AminusC.x * DminusC.y)) / denom;
		if(r < 0 || r > 1) return false;

		Ogre::Real s = ((AminusC.y * BminusA.x) - (AminusC.x * BminusA.y)) / denom;
		if(s < 0 || s > 1) return false;

		//if r and s are 0 then the line are coincident (on top of one another)
	        
		// Px=Ax+r(Bx-Ax)
		// Py=Ay+r(By-Ay)
		intersection.x = a.x + r * (BminusA.x);
		intersection.y = a.y + r * (BminusA.y);

		return true;
	}

	/** Checks if the two lines with points a,b & c,d intersect
		@remarks
			Algorithm defined at 
			http://www.faqs.org/faqs/graphics/algorithms-faq/
			OR
			Computational Geometry in C, Joeseph O'Rourke Pg.221 
        @param
            a, a Vector2 defines the start point of segment 1
        @param
            b, a Vector2 defines the end point of segment 1
        @param
            c, a Vector2 defines the start point of segment 2
        @param
            d, a Vector2 defines the end point of segment 2
		@param
			intersection a Vector2 reference store the resulting
			point of intersection if it occurs.
        @returns
            A bool indicating if the line segments intersect. 
    */
	static inline bool lineIntersect(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const Ogre::Vector2& c, const Ogre::Vector2& d, Ogre::Vector2& intersection)
	{
		Ogre::Vector2 BminusA(b - a);
		Ogre::Vector2 DminusC(d - c);
		Ogre::Real denom = (BminusA.x * DminusC.y) - (BminusA.y * DminusC.x);

		// line are parallel
		if(denom == 0) return false;

		Ogre::Vector2  AminusC(a - c);
		
		Ogre::Real r = ((AminusC.y * DminusC.x) - (AminusC.x * DminusC.y)) / denom;
		Ogre::Real s = ((AminusC.y * BminusA.x) - (AminusC.x * BminusA.y)) / denom;

		//if r and s are 0 then the line are coincident (on top of one another)
		if(r == 0 && s == 0) return false;
	        
		// Px=Ax+r(Bx-Ax)
		// Py=Ay+r(By-Ay)
		intersection.x = a.x + r * (BminusA.x);
		intersection.y = a.y + r * (BminusA.y);

		return true;
	}


    /** Rotates a vector around the origin
        @param
            vec provides a reference to the vector in question to be rotated
        @param
            angle defines the amount in Radians for the vector to be rotated
    */
	static inline void rotate(Ogre::Vector2& vec, const Ogre::Radian &angle)
	{
		vec = (Ogre::Math::Cos(angle) * vec) + (Ogre::Math::Sin(angle) * vec.perpendicular());
	}


    /** Calculates area of a polygon
		@remarks
			Algorithm defined at 
			http://local.wasp.uwa.edu.au/~pbourke/geometry/polyarea
        @warning
            This function can return a negative value as an area,
			wrap in Math::abs for a guaranteed positive value.
        @param
            polyPoints a std::vector that defines the polygon in the
			form of ordered points.
        @returns
            A Real representing the area of the polygon.
    */
	static Ogre::Real polygonArea(const std::vector<Ogre::Vector2> &polyPoints);


    /** Calculates the centre of mass for any given polygon
		@remarks
			This algorithm returns the centre of mass or centre of gravity
			for any polygon assuming it is of even mass.
			Algorithm defined at 
			http://local.wasp.uwa.edu.au/~pbourke/geometry/polyarea
        @param
            polyPoints a std::vector that defines the polygon in the
			form of ordered points.
        @returns
			A vector which defines the location of the centre of the 
			polygon mass.
    */
	static Ogre::Vector2 centerOfMass(const std::vector<Ogre::Vector2> &polyPoints);


	/** Insets a polygon
		@remarks
			This function returns a polygon that is inset
        @param 
			inset a Ogre::Real that specifies the amount to inset the poly
        @param
            polyPoints a std::vector that defines the polygon in the
			form of ordered points.

        @returns
			true if the inset is successfull, false if it is not successful for
			example in the event that the poly is too small to be inset
	*/
	static bool polygonInset(Ogre::Real inset, std::vector<Ogre::Vector2> &polyPoints);



	static bool isInside(const Ogre::Vector2 &loc, const std::vector<Ogre::Vector2> &polyPoints);

};



#endif
