//#ifdef WORLDLOT_H
//#define WORLDLOT_H
#include "stdafx.h"
#include "Triangulate.h"

class WorldLot
{
private:

public:
	static bool build(const std::vector<Ogre::Vector2> &footprint, 
		const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m)
	{
		std::vector<Ogre::Vector2> result;

		// roof
		if(Triangulate::Process(footprint, result))
		{
			// sides
			size_t i, j, N = footprint.size();
			for(i = 0; i < N; i++)
			{
				j = (i + 1) % N;
				Ogre::Vector2 perp((footprint[i] - footprint[j]).perpendicular());
				Ogre::Vector3 normal(perp.x, 0, perp.y);
				normal.normalise();
				m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
				m->normal(normal);
				m->position(Ogre::Vector3(footprint[i].x, foundation, footprint[i].y));
				m->normal(normal);
				m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
				m->normal(normal);

				m->position(Ogre::Vector3(footprint[j].x, height, footprint[j].y));
				m->normal(normal);
				m->position(Ogre::Vector3(footprint[i].x, height, footprint[i].y));
				m->normal(normal);
				m->position(Ogre::Vector3(footprint[j].x, foundation, footprint[j].y));
				m->normal(normal);
			}

			for(size_t i=0; i<result.size(); i+=3)
			{
				m->position(Ogre::Vector3(result[i+2].x, height, result[i+2].y));
				m->normal(Ogre::Vector3::UNIT_Y);
				m->position(Ogre::Vector3(result[i+1].x, height, result[i+1].y));
				m->normal(Ogre::Vector3::UNIT_Y);
				m->position(Ogre::Vector3(result[i].x, height, result[i].y));
				m->normal(Ogre::Vector3::UNIT_Y);
			}
			return true;
		}
		else
			return false;
	}
};

//#endif
