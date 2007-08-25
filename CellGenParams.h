#ifndef CELLGENPARAMS_H
#define CELLGENPARAMS_H

#include <OgrePrerequisites.h>

typedef struct
{
	unsigned int	_type;
	int				_seed;
	Ogre::Real		_segmentSize;
	Ogre::Real		_segmentDeviance;
	unsigned int	_degree;
	Ogre::Real		_degreeDeviance;
	Ogre::Real		_snapSize;
	Ogre::Real		_snapDeviance;
	Ogre::Real		_buildingHeight;
	Ogre::Real		_buildingDeviance;
	Ogre::Real		_roadWidth;
	size_t			_roadLimit;
	Ogre::Real		_connectivity;
	Ogre::Real		_lotSize;
	Ogre::Real		_lotDeviance;
	bool			_debug;
}
CellGenParams;


// TODO: should have a number of random generators defined and maybe a 
// simple parameter to try different distributions

// includes for rand functions
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

// typedefs for random number generator.
typedef boost::minstd_rand base_generator_type;
typedef boost::variate_generator<base_generator_type&, boost::uniform_real<> > rando;


#endif
