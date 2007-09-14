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
	 * @return true if pick is successful
	 */
	static Ogre::Real dotPerp(const Ogre::Vector2& v0, const Ogre::Vector2& v1);

	/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successful
	 */
	static bool pointInCell(const Ogre::Vector2& loc, const Cell& c);

		/**
	 * Pick a node from the scene using a mouse event with coords
	 * @param e an wxMouseEvent.
	 * @param snapSq the amount of distance squared that can be snapped to.
	 * @param wn a WorldNode pointer reference that can be used to store the picked node.
	 * @return true if pick is successful
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
		Ogre::Real r, s;
		bool intersects = lineIntersect(a, b, c, d, intersection, r, s);

		if(intersects)
		{
			// intersection must occur of segment no extension
			if(r < 0 || r > 1) return false;
			if(s < 0 || s > 1) return false;
			return true;
		}
		else return false;
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
		Ogre::Real r, s;
		return lineIntersect(a, b, c, d, intersection, r, s);
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
		@param
			r, a Ogre::Real reference stores the type of 
			intersection for line a-->b
			 - if r>1, P is located on extension of ab
			 - if r<0, P is located on extension of ba
		@param
			s, a Ogre::Real reference stores the type of 
			intersection for line c-->d
			 - if s>1, P is located on extension of cd
			 - if s<0, P is located on extension of dc
        @returns
            A bool indicating if the line segments intersect. 
    */
	static inline bool lineIntersect(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const Ogre::Vector2& c, const Ogre::Vector2& d, Ogre::Vector2& intersection, 
		Ogre::Real& r, Ogre::Real &s)
	{
		Ogre::Vector2 BminusA(b - a);
		Ogre::Vector2 DminusC(d - c);
		Ogre::Real denom = (BminusA.x * DminusC.y) - (BminusA.y * DminusC.x);

		// line are parallel
		if(denom == 0) return false;

		Ogre::Vector2  AminusC(a - c);
		
		r = ((AminusC.y * DminusC.x) - (AminusC.x * DminusC.y)) / denom;
		s = ((AminusC.y * BminusA.x) - (AminusC.x * BminusA.y)) / denom;

		//if r and s are 0 then the line are coincident (on top of one another)
		if(r == 0 && s == 0) return false;
	        
		// Px=Ax+r(Bx-Ax)
		// Py=Ay+r(By-Ay)
		intersection.x = a.x + r * (BminusA.x);
		intersection.y = a.y + r * (BminusA.y);

		return true;
	}


	/** Find the distance from a point, p to a line ab
		@remarks
			Algorithm defined at 
			http://www.faqs.org/faqs/graphics/algorithms-faq/
			1.02: How do I find the distance from a point to a line?
        @param
            a, a Vector2 defines the start point of segment
        @param
            b, a Vector2 defines the end point of segment
		@param
            c, a Vector2 defines the point
		@param
            p, a Vector2 defines the point on ab that forms a
			perpendicular to c
		@param
			r, a Ogre::Real reference that indicates the pos
			of point p along line ab
			 - r=0      p = a
			 - r=1      p = b
			 - r<0      p is on the backward extension of ab
			 - r>1      p is on the forward extension of ab
			 - 0<r<1    p is interior to ab
		@param
			s, a Ogre::Real reference that indicates the pos
			of point p along line cp
			 - s<0      c is left of ab
			 - s>0      c is right of ab
			 - s=0      c is on ab
		 @returns
			dist, a Ogre::Real value for the distance 
			from c to the line ab
    */
	static inline Ogre::Real distanceToLine(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const Ogre::Vector2& c, Ogre::Vector2& p, Ogre::Real& r, Ogre::Real& s)
	{
		Ogre::Vector2 ab(b - a);
		Ogre::Real Lsq = ab.squaredLength();


		//     (Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)
        // r = -------------------------------
        //                   L^2
		Ogre::Real bxMinusAx(b.x-a.x);
		Ogre::Real byMinusAy(b.y-a.y);
		Ogre::Real cxMinusAx(c.x-a.x);
		Ogre::Real cyMinusAy(c.y-a.y);
		r = (cxMinusAx*bxMinusAx + cyMinusAy*byMinusAy) / Lsq;

		// set p
		p.x = a.x + r*bxMinusAx;
        p.y = a.y + r*byMinusAy;

		//     (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
        // s = -----------------------------
        //                  L^2
		s = (-cyMinusAy*bxMinusAx+cxMinusAx*byMinusAy) / Lsq;

		// distance from c to p = |s|*L
		return Ogre::Math::Abs(s) * Ogre::Math::Sqrt(Lsq);
	}


	/** Find the distance from a point, p to a line segment ab
		@remarks
			this function uses the Geometry::distanceToLine but
			excludes points outside the line segment and instead
			uses the distance to the end points
        @param
            a, a Vector2 defines the start point of segment
        @param
            b, a Vector2 defines the end point of segment
		@param
            c, a Vector2 defines the point
		@param
            p, a Vector2 defines the closest point on segment ab
			to c
		@param
			dist, a Ogre::Real reference stores the distance 
			to from c to the line segment ab
    */
	static inline Ogre::Real distanceToLineSegment(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const Ogre::Vector2& c, Ogre::Vector2& p)
	{
		Ogre::Real dist, r, s;
		dist = distanceToLine(a, b, c, p, r, s);

		// if p is outside of the line segment, find the distance 
		// from the end points and set p to the nearest end point
		if(r<0)
		{
			dist = (c-a).length();
			p = a;
		}
		else if(r>1)
		{
			dist = (c-b).length();
			p = b;
		}
		return dist;
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
			inset a Ogre::Real that specifies the amount to inset the polygon
        @param
            polyPoints a std::vector that defines the polygon in the
			form of ordered points.

        @returns
			true if the inset is successful, false if it is not successful for
			example in the event that the polygon is too small to be inset
	*/
	static bool polygonInset(Ogre::Real inset, std::vector<Ogre::Vector2> &polyPoints);

	/** Insets a polygon
	@remarks
	This function returns a polygon that is inset
	@param 
	inset a Ogre::Real that specifies the amount to inset the polygon
	@param
	polyPoints a std::vector that defines the polygon in the
	form of ordered points.

	@returns
	true if the inset is successful, false if it is not successful for
	example in the event that the polygon is too small to be inset
	*/
	static bool polygonInsetFast(Ogre::Real inset, std::vector<Ogre::Vector2> &polyPoints);

	/** Insets a polygon
	@remarks
	This function returns a polygon that is inset
	@param 
	inset a Ogre::Real that specifies the amount to inset the polygon
	@param
	polyPoints a std::vector that defines the polygon in the
	form of ordered points.

	@returns
	true if the inset is successful, false if it is not successful for
	example in the event that the polygon is too small to be inset
	*/
	static bool polygonInsetFast(Ogre::Real inset, std::vector<Ogre::Vector3> &polyPoints);

	/** Insets a line
		@remarks
			This function returns a line that is inset
        @param 
			inset a Ogre::Real that specifies the amount to inset the polygon
        @param
            linePoints a std::vector that defines the polygon in the
			form of ordered points.

        @returns
			true if the inset is successful, false if it is not successful for
			example in the event that the poly is too small to be inset
	*/
	static bool lineInset(Ogre::Real inset, std::vector<Ogre::Vector2> &linePoints);

	static bool unionPolyAndLine(std::vector<Ogre::Vector2> &polyPoints, std::vector<Ogre::Vector2> &linePoints);

	static bool isInside(const Ogre::Vector2 &loc, const std::vector<Ogre::Vector2> &polyPoints);




	inline static Ogre::Vector2 calcNormVec2D(const Ogre::Vector2& p1, const Ogre::Vector2& p2)
	{
		return (p2 - p1).normalisedCopy();
	}

	inline static Ogre::Vector2 calcNormVec2D(const Ogre::Vector3& p1, const Ogre::Vector3& p2)
	{
		Ogre::Vector2 tmp(p2.x - p1.x, p2.y - p1.y);
		tmp.normalise();
		return tmp;
	}

	static void polygonInset(const std::vector<Ogre::Real>& insets, std::vector<Ogre::Vector3> &poly);


	inline static Ogre::Vector2 calcInsetVector(const  Ogre::Vector2& a, 
		const Ogre::Vector2& b, const  Ogre::Real& inset)
	{
		Ogre::Vector2 bisectorVector2(a + b);
		bisectorVector2.normalise();
		// look daddy, no sin or cos
		return bisectorVector2 * (inset / bisectorVector2.dotProduct(b));
	}
};



#endif
