#ifndef MESHBUILDER_H
#define MESHBUILDER_H

#include "stdafx.h"
#include <OgreString.h>
#include <OgreVector3.h>

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

	inline static void addVData3(std::vector<Ogre::Real> &vertexData, const Ogre::Vector3& v)
	{
		vertexData.push_back(v.x);
		vertexData.push_back(v.y);
		vertexData.push_back(v.z);
	}

	inline static void addVData3(std::vector<Ogre::Real> &vertexData, 
		const Ogre::Real &x, const Ogre::Real &y, const Ogre::Real &z)
	{
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
	}

	inline static void addVData2(std::vector<Ogre::Real> &vertexData, 
		const Ogre::Real &x, const Ogre::Real &y)
	{
		vertexData.push_back(x);
		vertexData.push_back(y);
	}

	inline static void addIData3(std::vector<Ogre::uint16> &indexData, 
		const Ogre::uint16 &a, const Ogre::uint16 &b, const Ogre::uint16 &c)
	{
		indexData.push_back(a);
		indexData.push_back(b);
		indexData.push_back(c);
	}
};

#endif
