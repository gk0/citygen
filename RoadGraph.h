#ifndef ROADGRAPH_H
#define ROADGRAPH_H

#include "stdafx.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>

#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreException.h>


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
typedef boost::graph_traits<Graph>::adjacency_iterator NodeIterator2;

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

//private:
public:
	Graph _graph;

public:
	inline NodeId addNode(NodeInterface* n)
	{
		return add_vertex(n, _graph);
	}

	inline void removeNode(NodeId nd)
	{
		remove_vertex(nd, _graph);
	}

	inline RoadInterface* getRoad(const RoadId rd) const
	{
		return _graph[rd];
	}

	inline RoadId getRoadId(const NodeId src, const NodeId dst) const
	{
		bool success;
		RoadId rd;
		tie(rd, success) = edge(src, dst, _graph);
		if(!success) 
			throw new Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND, "Edge not found", "RoadGraph::getRoad");
		return rd;
	}

	bool roadExists(const NodeId src, const NodeId dst) const
	{
		bool success;
		RoadId rd;
		tie(rd, success) = edge(src, dst, _graph);
		return success;
	}

	void removeRoad(const NodeId nd1, const NodeId nd2);
	void removeRoad(const RoadId rd);

	inline void clear()
	{
		_graph.clear();
	}

	inline NodeInterface* getNode(const NodeId nd) const
	{
		return _graph[nd];
	}

	inline size_t getDegree(const NodeId nd) const
	{
		return out_degree(nd, _graph);
	}

	inline std::pair<const NodeIterator, const NodeIterator> getNodes() const
	{
		return vertices(_graph);
	}

	inline std::pair<const RoadIterator, const RoadIterator> getRoads() const
	{
		return edges(_graph);
	}

	inline std::pair<const RoadIterator2, const RoadIterator2> 
		getRoadsFromNode(const NodeId& nd) const
	{
		return out_edges(nd, _graph);
	}

	inline std::pair<const NodeIterator2, const NodeIterator2> 
		getAdjacentNodes(const NodeId& nd) const
	{
		return adjacent_vertices(nd, _graph);
	}

	inline NodeId getSrc(const RoadId rd) const
	{
		return source(rd, _graph);
	}

	inline NodeInterface* getSrcNode(const RoadId rd) const
	{
		return _graph[source(rd, _graph)];
	}

	inline NodeId getDst(const RoadId rd) const
	{
		return target(rd, _graph);
	}

	inline NodeInterface* getDstNode(const RoadId rd) const
	{
		return _graph[target(rd, _graph)];
	}

	inline void setRoad(const RoadId &rd, RoadInterface* r)
	{
		_graph[rd] = r;
	}

	bool addRoad(const NodeId nd1, const NodeId nd2, RoadId& rd);
	bool addRoad(const NodeId nd1, const NodeId nd2, RoadInterface* r, RoadId &rd);
	bool findRoad(const NodeId nd1, const NodeId nd2, RoadId &rd) const;
	bool testRoad(const NodeId nd1, const NodeId nd2) const;

	bool getNodeClosest(const Ogre::Vector2 &loc, NodeId &nd, Ogre::Real &distance) const;
	bool getNodeClosestSq(const Ogre::Vector2 &loc, NodeId &nd, Ogre::Real &distance) const;

	bool snapToNode(const Ogre::Vector2& pos, const Ogre::Real& snapSzSq, NodeId& nd) const;

	bool hasIntersection(const RoadId rd);

	void extractPrimitives(std::vector< std::vector<NodeInterface*> > &filaments, 
		std::vector< std::vector<NodeInterface*> > &nodeCycles);

	NodeId getFirstAdjacent(NodeId nd)
	{
		return getFirstAdjacent(nd, _graph);
	}
		
	bool getClockwiseMost(NodeId vcurr, NodeId& vnext)
	{
		 return getClockwiseMost(vcurr, vnext, _graph);
	}

	bool getAntiClockwiseMostFromPrev(NodeId prev, NodeId vcurr, NodeId& vnext)
	{
		return getAntiClockwiseMostFromPrev(prev, vcurr, vnext, _graph);
	}

	int snapInfo(const NodeId aNode, const Ogre::Vector2& b, const Ogre::Real snapSz, 
		Ogre::Vector3& pos, NodeId &nd, RoadId& rd) const;

	int slowSnapInfo(const NodeId aNode, const Ogre::Vector2& b, const Ogre::Real snapSz, 
		Ogre::Vector3& pos, NodeId &nd, RoadId& rd) const;

	inline bool findClosestIntscn(const NodeId aNode, const Ogre::Vector2& b, const Ogre::Real snapSz, 
		Ogre::Vector2& pos, RoadId& rd) const
	{
		std::vector<NodeId> ignore(1);
		ignore[0] = aNode;
		return findClosestIntersection(ignore, b, snapSz, pos, rd);
	}

	bool findClosestIntscnConnected(const NodeId aNode, const NodeId bNode, const Ogre::Real snapSz, 
		Ogre::Vector2& pos, RoadId& rd) const;

	bool findClosestRoad(const NodeId aNode, const Ogre::Real snapSz, 
							  Ogre::Vector2& pos, RoadId& rd) const;

	inline static void createAABB(const Ogre::Vector2 &a, const Ogre::Vector2 &b, Ogre::Vector2 &P, Ogre::Vector2 &E)
	{
		Ogre::Vector2 halfVec((b-a)/2);
		P = a + halfVec;
		E.x = Ogre::Math::Abs(halfVec.x);
		E.y = Ogre::Math::Abs(halfVec.y);
	}

   std::list<NodeId> calculateBoundaryCycle(NodeId firstNode, Graph& g) const;

//private:
public:
	
	static bool getClockwiseCycle(NodeId v0, NodeId v1, Graph &g, std::vector<NodeId> &cycle);
	static bool getClockwiseCycleList(NodeId v0, NodeId v1, Graph &g, std::list<NodeId> &cycle);
	static bool getAntiClockwiseCycle(NodeId v0, NodeId v1, Graph &g, std::vector<NodeId> &cycle);

	static bool getClockwiseMost(NodeId vcurr, NodeId& vnext, const Graph &g);
	static bool getClockwiseMostFromPrev(NodeId vprev, NodeId vcurr, NodeId& vnext, const Graph& g);
	static bool getAntiClockwiseMostFromPrev(NodeId prev, NodeId vcurr, NodeId& vnext, const Graph &g);

	static void extractFilament(NodeId v0, NodeId v1, Graph &g, std::list<NodeId>& heap, 
		std::vector< std::vector<NodeInterface*> > &filaments);

	static void extractPrimitive(NodeId v0, Graph &g, std::list<NodeId>& heap, 
		std::vector< std::vector<NodeInterface*> > &filaments, 
		std::vector< std::vector<NodeInterface*> > &nodeCycles);

	static void removeFromHeap(NodeId v0, std::list<NodeId>& heap);
	static NodeId getFirstAdjacent(NodeId nd, const Graph &g);
	static NodeId getSecondAdjacent(NodeId nd, const Graph &g);

	bool sortVertex(const NodeId& v0, const NodeId& v1) const;

	static Ogre::Vector2 getSuperIntscn(NodeId a, NodeId b, NodeId c, Graph& g);

	static void addTerminalPoints(NodeId a, NodeId b, Graph& g, std::vector<Ogre::Vector2> &poly);

	void extractEnclosedRegions(
      std::vector< std::vector<NodeInterface*> > &polys, Ogre::ManualObject* debugMO = 0, size_t limit = 10000);

	static void addTerminalPoints(NodeId a, NodeId b, Graph& g, std::vector<Ogre::Vector3> &poly);

	static Ogre::Vector3 getSuperIntscn2(NodeId a, NodeId b, NodeId c, Graph& g);

private:
	std::string NodeIdToString(NodeId nd);

	bool findClosestIntersection(const std::vector<NodeId>& ignore, const Ogre::Vector2& b, const Ogre::Real snapSz, 
		Ogre::Vector2& pos, RoadId& rd) const;

	void calculateBoundingBoxFriends(const Ogre::Vector2& a, const Ogre::Vector2& b, 
		const Ogre::Real snapSz, std::vector< RoadId > &possibleRoads, std::vector< RoadId > &possibleWrRoads,
		std::vector< NodeId > &possibleNodes, std::vector<NodeId> &possibleNodesOnWr) const;

	bool findClosestNodeSnap(const NodeId aNode, const Ogre::Vector2& b, const Ogre::Real snapSz, 
							const std::vector< NodeId > &nodes, Ogre::Real &lowestR, NodeId &snapNode) const;

	bool findClosestRoadIntersection(const NodeId aNode, const Ogre::Vector2& b, const std::vector<RoadId> &roads, 
								Ogre::Real &lowestR, RoadId &iRoad, Ogre::Vector3& iPoint) const;

	bool snapToRoadNode(const Ogre::Vector3 &p, const Ogre::Real snapSz, const RoadId rd, NodeId &nd) const;

	bool findClosestSnapRoad(const Ogre::Vector2& b, const std::vector<RoadId> &roads, Ogre::Real &closestDist,
									RoadId &sRoad, Ogre::Vector3& sPoint) const;
};

#endif
