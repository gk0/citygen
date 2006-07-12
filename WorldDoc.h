#ifndef WorldDoc_H
#define WorldDoc_H

#include "stdafx.h"

/*
 * The world is basically a graph that store the road network
 * -roads are the edges
 * -nodes are the vertices
 */

// This comparison function is necessary for Vector3 since the 
// existing comparison would result in (0,1)<(1,0) and (1,0)<(0,1)
// both resolving to false. This means that a tree based structure 
// like a map then cannot work.
struct compare_Vector3
{
  bool operator()(const Ogre::Vector3& s1, const Ogre::Vector3& s2) const
  {
	if(s1.x < s2.x) return true;
	else if(s1.x == s2.x) { 
		if(s1.y < s2.y) return true;
		else if(s1.y == s2.y){
		  if(s1.z < s2.z) return true;
		}
	}
	return false;
  }
};

typedef boost::adjacency_list<
	boost::setS, // I care whether I have parallel edges and want to check for them as roads aren't often on top of one another.
	boost::listS, // selected for faster add/remove than vecS at the cost of some storage space
	boost::undirectedS, // my roads are undirected, I'm not monitoring the direction of traffic
	Ogre::Vector3 // set a ogre::Vector3 to store the location for each node
> RoadNetwork;

typedef boost::graph_traits<RoadNetwork>::vertex_descriptor Node;
typedef std::map<Ogre::Vector3, Node, compare_Vector3> LocationNodeMap;


class WorldDoc {

private:
	RoadNetwork mRoadNetwork;
	LocationNodeMap mLocationNodeMap;


public:
	//Default Constructor
	WorldDoc();

	bool addNode(const Ogre::Vector3& loc);
	bool removeNode(const Ogre::Vector3& loc);
	bool moveNode(const Ogre::Vector3& oldLoc, const Ogre::Vector3& newLoc);

	bool addRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2);
	bool removeRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2);

	void printNodes();
	void printEdges();
	void saveXML(const std::string& filename);
	void loadXML(const std::string& filename);

private:
	static std::string pointerToString(void * ptr);

};

#endif
