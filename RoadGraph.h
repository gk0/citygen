#ifndef ROADGRAPH_H
#define ROADGRAPH_H

#include "stdafx.h"
using namespace std;
using namespace Ogre;
using namespace boost;

struct RoadNode 
{
	Vector2	mPosition;
	bool			mSelected;
	default_color_type color;	// required for algorithms in this case BFS
	void*			mPtrData1;
};

struct RoadEdge
{
	Vector2	mPosition;
	bool			mSelected;
	default_color_type color;	// required for algorithms in this case BFS
	void*			mPtrData1;
};

typedef adjacency_list<
	setS, // I care whether I have parallel edges and want to check for them as roads aren't often on top of one another.
	vecS, // listS selected for faster add/remove than vecS at the cost of some storage space (dfs doesn't work with listS)
	undirectedS, // my roads are undirected, I'm not monitoring the direction of traffic
	RoadNode, // pointer to the vertex scenenode containing location etc.
	RoadEdge // pointer to the edge scenenode
> Graph;

typedef graph_traits<Graph>::vertex_descriptor NodeDescriptor;
typedef graph_traits<Graph>::vertex_iterator NodeIterator;
typedef graph_traits<Graph>::edge_descriptor RoadDescriptor;
typedef graph_traits<Graph>::edge_iterator RoadIterator;
typedef graph_traits<Graph>::out_edge_iterator RoadIterator2;


enum PrimitiveType {
	ISOLATED_VERTEX,
	FILAMENT,
	MINIMAL_CYCLE
};

//template<typename T> 
class Primitive : public std::list<NodeDescriptor>
{
public:
	PrimitiveType type;
	Primitive() : std::list<NodeDescriptor>(), type(MINIMAL_CYCLE) {}
	Primitive(PrimitiveType pt) : std::list<NodeDescriptor>(), type(pt) {}
};
typedef std::vector< Primitive > PrimitiveVec;


class RoadGraph
{

private:
	Graph mGraph;

public:
	RoadGraph();

	inline void clear()
	{
		mGraph.clear();
	}

	inline void* getNodeData1(const NodeDescriptor nd) const
	{
		return mGraph[nd].mPtrData1;
	}

	inline std::pair<const NodeIterator, const NodeIterator> getNodes() const
	{
		return vertices(mGraph);
	}

	inline void* getRoadData1(const RoadDescriptor rd) const
	{
		return mGraph[rd].mPtrData1;
	}

	inline std::pair<const RoadIterator, const RoadIterator> getRoads() const
	{
		return edges(mGraph);
	}

	inline std::pair<const RoadIterator2, const RoadIterator2>getRoadsFromNode(const NodeDescriptor& nd) const
	{
		return out_edges(nd, mGraph);
	}

	inline Vector2 getNodePosition(const NodeDescriptor nd) const
	{
		return mGraph[nd].mPosition;
	}

	inline NodeDescriptor getRoadSource(const RoadDescriptor rd) const
	{
		return source(rd, mGraph);
	}

	inline NodeDescriptor getRoadTarget(const RoadDescriptor rd) const
	{
		return target(rd, mGraph);
	}

	NodeDescriptor createNode(const Vector2& pos);
	NodeDescriptor createNode(const Vector2& pos, void* ptrData1);
	void setNodeData1(const NodeDescriptor nd, void* ptrData);
	void moveNode(const NodeDescriptor nd, const Vector2& pos);
	void removeNode(const NodeDescriptor nd);

	RoadDescriptor createRoad(const NodeDescriptor nd1, const NodeDescriptor nd2);
	RoadDescriptor createRoad(const NodeDescriptor nd1, const NodeDescriptor nd2, void* ptrData1);
	bool findRoad(const NodeDescriptor nd1, const NodeDescriptor nd2, RoadDescriptor& rd);
	void removeRoad(const NodeDescriptor nd1, const NodeDescriptor nd2);
	void setRoadData1(const RoadDescriptor rd, void* ptrData);

	//std::ostream& SaveObject(std::ostream& stream);
	//std::istream& LoadObject(std::istream& stream);
	void loadXML(const TiXmlHandle& root);
	TiXmlElement* saveXML();

	bool getClockwiseMost(NodeDescriptor vcurr, NodeDescriptor& vnext);
	bool getCounterClockwiseMostFromPrev(NodeDescriptor prev, NodeDescriptor vcurr, NodeDescriptor& vnext);

	void ExtractPrimitives(PrimitiveVec& primitives);
	void ExtractIsolatedVertex(NodeDescriptor v0, std::list<NodeDescriptor>& heap, PrimitiveVec& primitives);
	void ExtractFilament(NodeDescriptor v0, NodeDescriptor v1, std::list<NodeDescriptor>& heap, PrimitiveVec& primitives);
	void ExtractPrimitive(NodeDescriptor v0, std::list<NodeDescriptor>& heap, PrimitiveVec& primitives);
	void removeFromHeap(NodeDescriptor v0, std::list<NodeDescriptor>& heap);
	bool IsCycleEdge(NodeDescriptor v0, NodeDescriptor v1);
	unsigned int getNonBlackDegree(const NodeDescriptor v);
	NodeDescriptor getNonBlackAdjacent(const NodeDescriptor v);
	bool setEdgeColour(NodeDescriptor v0, NodeDescriptor v1, default_color_type c);

	static Real dotPerp(const Vector2& v0, const Vector2& v1)
	{
		return (v0.x * v1.y) - (v1.x * v0.y);
	}

	bool sortVertex(const NodeDescriptor& v0, const NodeDescriptor& v1);

	void setVertexBlack(const NodeDescriptor v);

	bool pointInPolygon(const Vector2& loc, const Primitive& p);

	bool pointInPolygon2(const Vector2& loc, const Primitive& p);
	static bool pointRayCrosses(const Vector2& loc, const Vector2& pt1, const Vector2& pt2);

	Vector2 findPrimitiveCenter(const Primitive& p);

	bool lineIntersection(const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& d, Vector2& intersection) const;
	bool roadIntersection(const RoadDescriptor rd1, const RoadDescriptor rd2, Vector2& intersection) const;
	bool roadIntersection(const Vector2& a, const Vector2& b, const RoadDescriptor rd, Vector2& intersection) const;

	bool findClosestIntersection(const Vector2& a, const Vector2& b, RoadDescriptor& rd, Vector2& intersection) const;

	bool getNodeClosestToPoint(const Ogre::Vector2 &loc, NodeDescriptor &nd, Ogre::Real &distance);


private:
	std::string nodeDescriptorToString(NodeDescriptor nd);
};

#endif
