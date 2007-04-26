#ifndef ROADGRAPH_H
#define ROADGRAPH_H

#include "stdafx.h"
//#include "Geometry.h"
//#include "NodeInterface.h"

class NodeInterface;
class RoadInterface;
class WorldCell;

typedef boost::adjacency_list<
	boost::setS, // I care whether I have parallel edges and want to check for them as roads aren't often on top of one another.
	boost::listS, // listS selected for faster add/remove than vecS at the cost of some storage space (dfs doesn't work with listS)
	boost::undirectedS, // my roads are undirected, I'm not monitoring the direction of traffic
	NodeInterface*, // pointer to the vertex scenenode containing location etc.
	RoadInterface* // pointer to the edge scenenode
> Graph;

typedef boost::graph_traits<Graph>::vertex_descriptor NodeId;
typedef boost::graph_traits<Graph>::vertex_iterator NodeIterator;
typedef boost::graph_traits<Graph>::edge_descriptor RoadId;

struct road_less_than :
	public std::binary_function<RoadId, RoadId, bool>
{
	bool operator()(const RoadId &rd1, const RoadId &rd2) const
	{
		return rd1.get_property() < rd2.get_property();
	}
};

typedef boost::graph_traits<Graph>::edge_iterator RoadIterator;
typedef boost::graph_traits<Graph>::out_edge_iterator RoadIterator2;


class RoadGraph
{

private:
	Graph mGraph;

public:
	RoadGraph();

	inline NodeId addNode(NodeInterface* n)
	{
		return add_vertex(n, mGraph);
	}

	inline void removeNode(NodeId nd)
	{
		remove_vertex(nd, mGraph);
	}

	inline RoadInterface* getRoad(const RoadId rd) const
	{
		return mGraph[rd];
	}

	inline RoadId getRoad(const NodeId src, const NodeId dst) const
	{
		bool success;
		RoadId rd;
		tie(rd, success) = edge(src, dst, mGraph);
		if(!success) 
			throw new Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND, "Edge not found", "RoadGraph::getRoad");
		return rd;
	}

	void removeRoad(const NodeId nd1, const NodeId nd2);
	void removeRoad(const RoadId rd);

	inline void clear()
	{
		mGraph.clear();
	}

	inline NodeInterface* getNode(const NodeId nd) const
	{
		return mGraph[nd];
	}

	inline size_t getDegree(const NodeId nd) const
	{
		return out_degree(nd, mGraph);
	}

	inline std::pair<const NodeIterator, const NodeIterator> getNodes() const
	{
		return vertices(mGraph);
	}

	inline std::pair<const RoadIterator, const RoadIterator> getRoads() const
	{
		return edges(mGraph);
	}

	inline std::pair<const RoadIterator2, const RoadIterator2> 
		getRoadsFromNode(const NodeId& nd) const
	{
		return out_edges(nd, mGraph);
	}

	inline NodeId getSrc(const RoadId rd) const
	{
		return source(rd, mGraph);
	}

	inline NodeInterface* getSrcNode(const RoadId rd) const
	{
		return mGraph[source(rd, mGraph)];
	}

	inline NodeId getDst(const RoadId rd) const
	{
		return target(rd, mGraph);
	}

	inline NodeInterface* getDstNode(const RoadId rd) const
	{
		return mGraph[target(rd, mGraph)];
	}

	inline void setRoad(const RoadId &rd, RoadInterface* r)
	{
		mGraph[rd] = r;
	}

	bool addRoad(const NodeId nd1, const NodeId nd2, RoadId& rd);
	bool addRoad(const NodeId nd1, const NodeId nd2, RoadInterface* r, RoadId &rd);
	bool findRoad(const NodeId nd1, const NodeId nd2, RoadId &rd) const;
	bool testRoad(const NodeId nd1, const NodeId nd2) const;

	bool roadIntersection(const RoadId rd1, const RoadId rd2, Ogre::Vector2& intersection) const;
	bool roadIntersection(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const RoadId rd, Ogre::Vector2& intersection) const;

	bool findClosestIntersection(const Ogre::Vector2& a, 
		const Ogre::Vector2& b, RoadId& rd, Ogre::Vector2& intersection) const;
	bool findClosestIntersection3(const Ogre::Vector2& a, const Ogre::Vector2& b, 
					const NodeId& ignoreNode, RoadId& rd, Ogre::Vector2& intersection) const;

	bool getNodeClosest(const Ogre::Vector2 &loc, NodeId &nd, Ogre::Real &distance) const;
	bool getNodeClosestSq(const Ogre::Vector2 &loc, NodeId &nd, Ogre::Real &distance) const;

	//bool testValidRoad(const NodeId &nd1, const Ogre::Vector2 &nd2Loc);

	/**
	 * Find the closest intersection or resulting intersection from snap to node
	 *
	 * @param sourcePos the position of the source node of the proposed road.
	 * @param destPos the proposed destination of the road.
	 * @param snapSzSquared the snap size value squared, used to snap the 
	 *  proposed destination on to an existing node or road
	 * @param nd a node descriptor reference to store a node match
	 * @param pos a Ogre::Vector2 position variable to store a placed position
	 * @return a int value indicate whether an intersection was not found, was found or snapped to node
	 */
	int findClosestSnappedIntersection(const Ogre::Vector2& srcPos, const Ogre::Vector2& dstPos, 
											const Ogre::Real& snapSzSquared, NodeId& nd,
											RoadId& rd, Ogre::Vector2& intersection) const;

	bool findClosestIntersection2(const std::vector<RoadId>& roadSegments, 
		RoadId& rd, Ogre::Vector2& intersection) const;

	bool snapToRoadNode(const Ogre::Vector2& pos, const RoadId& rd, const Ogre::Real& snapSzSq, NodeId& nd) const;
	bool snapToNode(const Ogre::Vector2& pos, const Ogre::Real& snapSzSq, NodeId& nd) const;

	bool hasIntersection(const Ogre::Vector2& a, const Ogre::Vector2& b, Ogre::Vector2& pos) const;
	bool hasIntersection(const RoadId rd);

	void extractPrimitives(std::vector<RoadInterface*> &filaments, 
		std::vector< std::vector<NodeInterface*> > &nodeCycles, 
		std::vector< std::vector<RoadInterface*> > &roadCycles);

	int snapInfo(NodeId srcNd, const Ogre::Vector2 &dstPoint, Ogre::Real snapSzSquared,
						NodeId& nd, RoadId& rd, Ogre::Vector2& pos) const;


		
	bool getClockwiseMost(NodeId vcurr, NodeId& vnext)
	{
		 return getClockwiseMost(vcurr, vnext, mGraph);
	}

	bool getCounterClockwiseMostFromPrev(NodeId prev, NodeId vcurr, NodeId& vnext)
	{
		return getCounterClockwiseMostFromPrev(prev, vcurr, vnext, mGraph);
	}

	bool findClosestIntersection(const NodeId srcNd, const NodeId dstNd,
		RoadId& rd, Ogre::Vector2& pos) const;

	bool snapToOtherNode(const Ogre::Vector2 pos, const std::set<NodeId> ignoreSet, 
		const Ogre::Real& snapSzSq, NodeId& otherNd) const;

	Ogre::Vector2 getRoadBounaryIntersection(const RoadId leftR, const RoadId rightR);


private:

	static bool getClockwiseMost(NodeId vcurr, NodeId& vnext, const Graph &g);
	static bool getCounterClockwiseMostFromPrev(NodeId prev, NodeId vcurr, NodeId& vnext, const Graph &g);

	static void extractFilament(NodeId v0, NodeId v1, Graph &g, std::list<NodeId>& heap, 
		std::vector<RoadInterface*>& filaments);

	static void extractPrimitive(NodeId v0, Graph &g, std::list<NodeId>& heap, 
		std::vector<RoadInterface*>& filaments, 
		std::vector< std::vector<NodeInterface*> > &nodeCycles, 
		std::vector< std::vector<RoadInterface*> > &roadCycles);

	static void removeFromHeap(NodeId v0, std::list<NodeId>& heap);
	static NodeId getFirstAdjacent(NodeId nd, const Graph &g);
	static RoadId getFirstRoad(NodeId nd, const Graph &g);

	static NodeId getSecondAdjacent(NodeId nd, const Graph &g);
	static RoadId getSecondRoad(NodeId nd, const Graph &g);

	bool sortVertex(const NodeId& v0, const NodeId& v1);

	inline bool findClosestIntersection(NodeId srcNd, const Ogre::Vector2 &srcPos,
							 const Ogre::Vector2 &dstPos, RoadId& rd, Ogre::Vector2& pos) const;
/*
	inline bool findClosestIntersection(NodeId srcNd, 
							 const Ogre::Vector2 &srcPos,
							 const Ogre::Vector2 &dstPos,
							 RoadId& rd, Ogre::Vector2& pos) const
	{
		bool hasIntersection = false;
		Ogre::Real currentDistance, closestDistance;
		Ogre::Vector2 currentIntersection;
		RoadIterator rIt, rEnd;
		for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
		{
			// test for an intersection
			if(Geometry::lineSegmentIntersect(srcPos, dstPos, getNode(getSrc(*rIt))->getPosition2D(), 
				getNode(getDst(*rIt))->getPosition2D(), currentIntersection))
			{
				// exclude roads connected to the srcNode
				if(getSrc(*rIt) == srcNd || getDst(*rIt) == srcNd)
					continue;

				// if no previous intersection
				if(!hasIntersection)
				{
					// set the initial closest distance and params
					hasIntersection = true;
					closestDistance = (srcPos - currentIntersection).squaredLength();
					pos = currentIntersection;
					rd = *rIt;
				}
				else
				{				
					// test is it the closest
					currentDistance = (srcPos - currentIntersection).squaredLength();
					if(currentDistance < closestDistance)
					{
						// set the new closest distance and params
						closestDistance = currentDistance;
						pos = currentIntersection;
						rd = *rIt;
					}
				}
			}
		}
		return hasIntersection;	
	}
*/



private:
	std::string NodeIdToString(NodeId nd);
};

#endif
