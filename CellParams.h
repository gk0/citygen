#ifndef CELLGENPARAMS_H
#define CELLGENPARAMS_H

#include <OgrePrerequisites.h>

struct CellParams
{
	unsigned int	_type;
	int				_seed;
	Ogre::Real		_segmentSize;
	Ogre::Real		_segmentDeviance;
	unsigned int	_degree;
	Ogre::Real		_degreeDeviance;
	Ogre::Real		_aspect;
	Ogre::Real		_snapSize;
	Ogre::Real		_snapDeviance;
	Ogre::Real		_buildingHeight;
	Ogre::Real		_buildingDeviance;
	Ogre::Real		_roadWidth;
	size_t			_roadLimit;
	Ogre::Real		_connectivity;
	Ogre::Real		_footpathWidth;
	Ogre::Real		_footpathHeight;
	Ogre::Real		_lotWidth;
	Ogre::Real		_lotDepth;
	Ogre::Real		_lotDeviance;
	bool			_debug;
	bool			_mcbDebug;

	CellParams(unsigned int type, int seed, Ogre::Real segmentSize, 
		Ogre::Real segmentDeviance, unsigned int degree, Ogre::Real degreeDeviance, Ogre::Real aspect, 
		Ogre::Real snapSize, Ogre::Real snapDeviance, Ogre::Real buildingHeight, 
		Ogre::Real buildingDeviance, Ogre::Real roadWidth, size_t roadLimit, 
		Ogre::Real connectivity, Ogre::Real	footpathWidth, Ogre::Real footpathHeight,
		Ogre::Real lotWidth, Ogre::Real lotDepth, Ogre::Real lotDeviance, bool debug, bool mcbDebug = false)
	{
		_type = type;
		_seed = seed;
		_segmentSize = segmentSize;
		_segmentDeviance = segmentDeviance;
		_degree = degree;
		_degreeDeviance = degreeDeviance;
		_aspect = aspect;
		_snapSize = snapSize;
		_snapDeviance = snapDeviance;
		_buildingHeight = buildingHeight;
		_buildingDeviance = buildingDeviance;
		_roadWidth = roadWidth;
		_roadLimit = roadLimit;
		_connectivity = connectivity;
		_footpathWidth = footpathWidth;
		_footpathHeight = footpathHeight;
		_lotWidth = lotWidth;
		_lotDepth = lotDepth;
		_lotDeviance = lotDeviance;
		_debug = debug;
		_mcbDebug = mcbDebug;
	}

	CellParams()
	{
		*this = MANHATTAN;
		//TODO: this is a short term hack, Manhattan may not be initialised by the time it is used. DANGER
		_seed = 1;
	}

	static const CellParams MANHATTAN;
	static const CellParams SUBURBIA;
	static const CellParams INDUSTRIAL;
};



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
