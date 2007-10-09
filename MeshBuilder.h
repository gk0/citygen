#ifndef MESHBUILDER_H
#define MESHBUILDER_H

#include "stdafx.h"


typedef std::pair< const std::vector<Ogre::Real>*, const std::vector<Ogre::uint16>* > PtrPolyData;
typedef std::map< Ogre::Material*, std::vector< PtrPolyData > > PolyDataMap;

class MeshBuilder
{
private:
	const Ogre::String _name;
	const Ogre::String _groupName;
	Ogre::ManualResourceLoader* _loader;

	PolyDataMap		_polyDataMap;

public:
	MeshBuilder(const Ogre::String &name, const Ogre::String &groupName, 
		Ogre::ManualResourceLoader *loader=0);

	void registerData(Ogre::Material* material, const std::vector<Ogre::Real> &vdata, 
		const std::vector<Ogre::uint16> &idata);

	void build();

};

#endif
