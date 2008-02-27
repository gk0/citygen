#include "stdafx.h"
#include "RoadGraph.h"
#include "NodeInterface.h"
#include "RoadInterface.h"
#include "Geometry.h"
#include "WorldNode.h"
#include "SimpleNode.h"

#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreLogManager.h>

using namespace std;
using namespace boost;
using namespace Ogre;

bool RoadGraph::addRoad(const NodeId nd1, const NodeId nd2, RoadId& rd)
{
	assert(nd1 != nd2);
	bool inserted = false;
	tie(rd, inserted) = add_edge(nd1, nd2, (RoadInterface*)0, _graph);
	return inserted;
}

bool RoadGraph::addRoad(const NodeId nd1, const NodeId nd2, RoadInterface* r,
		RoadId& rd)
{
	bool inserted = false;
	tie(rd, inserted) = add_edge(nd1, nd2, r, _graph);
	if (inserted)
	{
		getNode(nd1)->onAddRoad();
		getNode(nd2)->onAddRoad();
	}
	return inserted;
}

bool RoadGraph::findRoad(const NodeId nd1, const NodeId nd2, RoadId& rd) const
{
	RoadId r;
	bool found = false;
	tie(r, found) = edge(nd1, nd2, _graph);
	if (found)
		rd = r;
	return found;
}

bool RoadGraph::testRoad(const NodeId nd1, const NodeId nd2) const
{
	RoadId r;
	bool found = false;
	tie(r, found) = edge(nd1, nd2, _graph);
	return found;
}

void RoadGraph::removeRoad(const NodeId nd1, const NodeId nd2)
{
	RoadId rd;
	bool found = false;
	tie(rd, found) = edge(nd1, nd2, _graph);
	if (found)
	{
		remove_edge(rd, _graph);
		_graph[nd1]->onRemoveRoad();
		_graph[nd2]->onRemoveRoad();
	}
}

void RoadGraph::removeRoad(const RoadId rd)
{
	NodeInterface *n1, *n2;
	n1 = getSrcNode(rd);
	n2 = getDstNode(rd);
	remove_edge(rd, _graph);
	n1->onRemoveRoad();
	n2->onRemoveRoad();
}

bool RoadGraph::sortVertex(const NodeId& v0, const NodeId& v1) const
{
	Real x0 = _graph[v0]->getPosition2D().x;
	Real x1 = _graph[v1]->getPosition2D().x;
	return (x0 < x1);
}

void RoadGraph::extractPrimitives(vector< vector<NodeInterface*> > &filaments,
		vector< vector<NodeInterface*> > &nodeCycles)
{
	// create a copy of our road graph to work on
	Graph g(_graph);

	std::ostringstream oss;
	oss << " ";

	//init roads
	BOOST_FOREACH(RoadId rd, edges(g)) g[rd]->setRoadCycle(false);

	//set<NodeId> heap = vertices; 
	//gk: at the moment I'm using a vertex to store vertices so they are already sorted
	graph_traits<Graph>::vertex_iterator i, end;
	tie(i, end) = vertices(g);
	//set<NodeId> heap(i, end);

	//SET needs sort
	//sort(heap.begin(), heap.end(), comp());
	//it would be neater o use a comparator but our comparison isn't standalone
	std::list< NodeId > heap;

	//insert one
	heap.push_back(*i++);

	for (; i != end; ++i)
	{
		for (std::list<NodeId>::iterator tit = heap.begin(); true; tit++)
		{
			if (tit == heap.end())
			{
				heap.push_back(*i);
				break;
			}
			else if (sortVertex(*i, *tit))
			{
				heap.insert(tit, *i);
				break;
			}
		}
	}
	//oss << "Begin x-cords" << endl;
	//// heap sorted lets print the node ids alongside their xloc
	//for(std::list<NodeId>::iterator heapIt = heap.begin(); heapIt != heap.end(); heapIt++)
	//{
	//	oss << "(" << *heapIt << "," << mGraph[*heapIt]->getPosition2D().x << ") ";
	//}	
	//LogManager::getSingleton().logMessage(oss.str());

	try
	{

		//while (heap is not empty) do
		while(heap.size() != 0)
		{
			//Vertex v0 = heap.GetMin();
			NodeId v0 = *(heap.begin());

			switch(out_degree(v0, g))
			{
				case 0:
				remove_vertex(v0, g);
				removeFromHeap(v0, heap);
				break;
				case 1:
				extractFilament(v0, getFirstAdjacent(v0, g), g, heap, filaments);
				//oss<<"Filament: "<<graph[v0].getName()<<endl;

				//DEBUG
				//heap.erase(v0);
				break;
				default:
				extractPrimitive(v0, g, heap, filaments, nodeCycles); // filament or minimal cycle
				//oss<<"Cycle or Filament: "<<mGraph[v0].getName()<<endl;

				//DEBUG
				//heap.erase(v0);
				break;
			}
			//heap.erase(v0);
		}

	}
	catch(Exception e)
	{
		//LogManager::getSingleton().logMessage(e.getDescription());

		//DANGER - setting partially decomposed graph to real graph
		//mGraph = g;
	}
}

void RoadGraph::extractFilament(NodeId v0, NodeId v1, Graph &g,
		list<NodeId>& heap, vector< vector<NodeInterface*> > &filaments)
{
	//assert(out_degree(v0, g) != 2); // trouble
	if (out_degree(v0, g) == 2)
	{
		//	throw Exception(Exception::ERR_ITEM_NOT_FOUND, "ERROR: incorrect filament degree", "RoadGraph::extractFilament");
	}

	if (g[edge(v0, v1, g).first]->isRoadCycle())
	{
		if (out_degree(v0, g) >= 3)
		{
			remove_edge(v0, v1, g);
			v0 = v1;
			if (out_degree(v0, g) == 1)
			{
				v1 = getFirstAdjacent(v0, g);
			}

		}
		while (out_degree(v0, g) == 1)
		{
			v1 = getFirstAdjacent(v0, g);

			if (g[edge(v0, v1, g).first]->isRoadCycle())
			{
				//heap.erase(v0);
				removeFromHeap(v0, heap);
				remove_edge(v0, v1, g);
				remove_vertex(v0, g);
				v0 = v1;
			}
			else
			{
				break;
			}
		}
		if (out_degree(v0, g) == 0)
		{
			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_vertex(v0, g);
		}
	}
	else
	{
		vector<NodeInterface*> filament;

		if (out_degree(v0, g) >= 3)
		{
			filament.push_back(g[v0]);
			remove_edge(v0, v1, g);
			v0 = v1;
			if (out_degree(v0, g) == 1)
			{
				v1 = getFirstAdjacent(v0, g);
			}
		}

		while (out_degree(v0, g) == 1)
		{
			v1 = getFirstAdjacent(v0, g);
			filament.push_back(g[v0]);

			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_edge(v0, v1, g);
			remove_vertex(v0, g);
			v0 = v1;
		}

		filament.push_back(g[v0]);

		if (out_degree(v0, g) == 0)
		{
			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_vertex(v0, g);
		}

		filaments.push_back(filament);
	}
}

void RoadGraph::extractPrimitive(NodeId v0, Graph &g, list<NodeId>& heap,
		vector< vector<NodeInterface*> > &filaments,
		vector< vector<NodeInterface*> > &nodeCycles)
{
	set<NodeId> visited;
	vector<NodeInterface*> nodeCycle;
	vector<RoadInterface*> roadCycle;

	NodeId v1;
	bool vertexFound = getClockwiseMost(v0, v1, g);
	//nodeCycle.push_back(g[v0]);
	NodeId vprev = v0;
	NodeId vcurr = v1;
	NodeId vnext;

	//while (vcurr is not nil) and (vcurr is not v0) and (vcurr is not visited) do
	while (vertexFound && vcurr != v0 && visited.find(vcurr) == visited.end())
	{
		nodeCycle.push_back(g[vcurr]);
		roadCycle.push_back(g[edge(vprev, vcurr, g).first]);
		visited.insert(vcurr);
		vertexFound = getAntiClockwiseMostFromPrev(vprev, vcurr, vnext, g);
		vprev = vcurr;
		vcurr = vnext;
	}
	if (!vertexFound)
	{
		assert(out_degree(vprev, g) == 1);

		// Filament found, not necessarily rooted at v0.
		extractFilament(vprev, getFirstAdjacent(vprev, g), g, heap, filaments);

		//Guessing madness - no i don't understand
		//setEdgeColour(v0, getFirstAdjacent(v0, g), black_color);
		//heap.erase(v0);
		//if(out_degree(v0, g) == 2 && isCycleEdge(v0,getFirstAdjacent(v0, g))) setEdgeColour(v0, getFirstAdjacent(v0, g), black_color);
		//if(out_degree(v0, g) == 2) setEdgeColour(v0, getFirstAdjacent(v0, g), black_color);
		//removeFromHeap(v0, heap);
	}
	else if (vcurr == v0)
	{
		// Minimal cycle found.

		// add the last edge
		nodeCycle.push_back(g[vcurr]);
		roadCycle.push_back(g[edge(vprev, vcurr, g).first]);

		// add a cell to our list
		nodeCycles.push_back(nodeCycle);

		// mark every edge in the cycle as a cycle edge
		BOOST_FOREACH(RoadInterface* ri, roadCycle) ri->setRoadCycle(true);

		remove_edge(v0, v1, g);

		if (out_degree(v0, g) == 1)
		{
			// Remove the filament rooted at v0.
			extractFilament(v0, getFirstAdjacent(v0, g), g, heap, filaments);
		}

		// need to check if v1 is still there
		if (out_degree(v1, g) == 1) //BUG: TODO: causes error sometimes
		{
			// Remove the filament rooted at v1.
			extractFilament(v1, getFirstAdjacent(v1, g), g, heap, filaments);
		}
	}
	else // vcurr was visited earlier
	{
		// A cycle has been found, but is not guaranteed to be a minimal
		// cycle. This implies v0 is part of a filament. Locate the
		// starting point for the filament by traversing from v0 away
		// from the initial v1.
		while (out_degree(v0, g) == 2) // maybe try < 2
		{
			if (getFirstAdjacent(v0, g) != v1)
			{
				v1 = v0;
				v0 = getFirstAdjacent(v0, g);
			}
			else
			{
				v1 = v0;
				v0 = getSecondAdjacent(v0, g);
			}
		}
		extractFilament(v0, v1, g, heap, filaments); //danger causes assertion error
	}
}

bool RoadGraph::getClockwiseMost(NodeId vcurr, NodeId& vnext, const Graph &g)
{
	bool isConvex = false, success = false;

	Vector2 dnext;
	Vector2 dcurr = Vector2(0, -1);
	Vector2 vcurr_pos(g[vcurr]->getPosition2D());

	//for each adjacent vertex vadj of vcurr do
	BOOST_FOREACH(NodeId vadj, adjacent_vertices(vcurr, g))
	{
		//NodeId vadj = target(rd, g);

		NodeInterface* ni = g[vadj];
		Vector2 dadj = ni->getPosition2D() - vcurr_pos; //error void ptr

		if(!success)
		{
			vnext = vadj;
			dnext = dadj;
			isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			success = true;
			continue;
		}

		if(isConvex)
		{
			if(Geometry::dotPerp(dcurr, dadj) < 0 || Geometry::dotPerp(dnext, dadj) < 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			}
		}
		else
		{
			if(Geometry::dotPerp(dcurr, dadj) < 0 && Geometry::dotPerp(dnext, dadj) < 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			}
		}
	}
	return success;
}

bool RoadGraph::getClockwiseMostFromPrev(NodeId vprev, NodeId vcurr,
		NodeId& vnext, const Graph& g)
{
	bool isConvex = false, success = false;

	Vector2 vcurr_pos(g[vcurr]->getPosition2D());
	Vector2 vprev_pos(g[vprev]->getPosition2D());
	Vector2 dcurr = vcurr_pos - vprev_pos;
	Vector2 dnext;

	//for each adjacent vertex vadj of vcurr do
	BOOST_FOREACH(NodeId vadj, adjacent_vertices(vcurr, g))
	{
		//NodeId vadj = target(rd, g);

		//No going back :)
		if(vadj == vprev) continue;

		Vector2 dadj = g[vadj]->getPosition2D() - vcurr_pos; //error

		if(!success)
		{
			vnext = vadj;
			dnext = dadj;
			isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			success = true;
			continue;
		}

		if(isConvex)
		{
			if(Geometry::dotPerp(dcurr, dadj) < 0 || Geometry::dotPerp(dnext, dadj) < 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			}
		}
		else
		{
			if(Geometry::dotPerp(dcurr, dadj) < 0 && Geometry::dotPerp(dnext, dadj) < 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			}
		}
	}
	return success;
}

bool RoadGraph::getAntiClockwiseMostFromPrev(NodeId vprev, NodeId vcurr,
		NodeId& vnext, const Graph& g)
{
	bool isConvex = false, success = false;

	Vector2 vcurr_pos(g[vcurr]->getPosition2D());
	Vector2 vprev_pos(g[vprev]->getPosition2D());
	Vector2 dcurr = vcurr_pos - vprev_pos;
	Vector2 dnext;

	//for each adjacent vertex vadj of vcurr do
	BOOST_FOREACH(NodeId vadj, adjacent_vertices(vcurr, g))
	{
		//NodeId vadj = target(rd, g);

		//No going back :)
		if(vadj == vprev) continue;

		Vector2 dadj = g[vadj]->getPosition2D() - vcurr_pos; //error

		if(!success)
		{
			vnext = vadj;
			dnext = dadj;
			isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			success = true;
			continue;
		}

		if(isConvex)
		{
			if(Geometry::dotPerp(dcurr, dadj) > 0 && Geometry::dotPerp(dnext, dadj) > 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			}
		}
		else
		{
			if(Geometry::dotPerp(dcurr, dadj) > 0 || Geometry::dotPerp(dnext, dadj) > 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (Geometry::dotPerp(dnext, dcurr) <= 0);
			}
		}
	}
	return success;
}

void RoadGraph::removeFromHeap(NodeId v0, std::list<NodeId>& heap)
{
	std::list<NodeId>::iterator hIt = find(heap.begin(), heap.end(), v0);
	if (hIt != heap.end())
		heap.erase(hIt);
}

NodeId RoadGraph::getFirstAdjacent(NodeId nd, const Graph &g)
{
	NodeIterator2 nIt, nEnd;
	tie(nIt, nEnd) = adjacent_vertices(nd, g);
	assert(nIt != nEnd);
	return *nIt;
}

NodeId RoadGraph::getSecondAdjacent(NodeId nd, const Graph &g)
{
	NodeIterator2 nIt, nEnd;
	tie(nIt, nEnd) = adjacent_vertices(nd, g);
	assert(nIt != nEnd);
	nIt++;
	return *nIt;
}

bool RoadGraph::getNodeClosest(const Ogre::Vector2 &loc, NodeId &nd,
		Ogre::Real &distance) const
{
	if (getNodeClosestSq(loc, nd, distance))
	{
		distance = Math::Sqrt(distance);
		return true;
	}
	else
		return false;
}

bool RoadGraph::getNodeClosestSq(const Ogre::Vector2 &loc, NodeId &nd,
		Ogre::Real &distance) const
{
	Ogre::Real currDist = std::numeric_limits<Ogre::Real>::max();
	NodeId currNode = 0;
	bool success = false;
	BOOST_FOREACH(NodeId nd, vertices(_graph))
	{
		Vector2 temp(_graph[nd]->getPosition2D() - loc);
		if(temp.squaredLength() < currDist)
		{
			currDist = temp.squaredLength();
			currNode = nd;
			success = true;
		}
	}
	if (success)
	{
		nd = currNode;
		distance = currDist;
	}
	return success;
}

bool RoadGraph::hasIntersection(const RoadId roadId)
{
	// prepare the ignore list used to avoid returning 
	//  intersections to properly connected roads
	std::set<RoadId, road_less_than> ignoreList;
	RoadIterator2 riIt, riEnd;

	// add roads connected to the source node
	tie(riIt, riEnd) = getRoadsFromNode(getSrc(roadId));
	ignoreList.insert(riIt, riEnd);

	// add roads connected to the destination node
	tie(riIt, riEnd) = getRoadsFromNode(getDst(roadId));
	ignoreList.insert(riIt, riEnd);

	BOOST_FOREACH(RoadId rd, edges(_graph))
	{
		// if road is not in the ignore list
		std::set<RoadId, road_less_than>::iterator ignoreIt = ignoreList.find(rd);
		if(ignoreIt == ignoreList.end())
		{
			Vector2 intersection;
			if(Geometry::lineSegmentIntersect(getNode(getSrc(roadId))->getPosition2D(), getNode(getDst(roadId))->getPosition2D(), 
				getNode(getSrc(rd))->getPosition2D(), getNode(getDst(rd))->getPosition2D(), intersection))
				return true;
		}
	}
	return false;
}

bool RoadGraph::snapToNode(const Vector2& pos, const Real& snapSzSq,
		NodeId& nodeId) const
{
	Ogre::Real closestDistSq = snapSzSq;
	bool success = false;
	BOOST_FOREACH(NodeId nd, vertices(_graph))
	{
		Vector2 nodePos(_graph[nd]->getPosition2D());
		Real currDistSq =(nodePos - pos).squaredLength();
		if(currDistSq < closestDistSq)
		{
			closestDistSq = currDistSq;
			nodeId = nd;
			success = true;
		}
	}
	return success;
}

void RoadGraph::calculateBoundingBoxFriends(const Vector2& a, const Vector2& b, 
	const Real snapSz, vector< RoadId > &possibleRoads, vector< RoadId > &possibleWorldRoads,
	vector< NodeId > &possibleNodes, vector< NodeId > &possibleNodesOnWr) const
{
	// create bounding box vectors for segment ab
	Vector2 abP, abE;
	createAABB(a, b, abP, abE);
	abE.x += snapSz;
	abE.y += snapSz;

	// maybe slow but i don't care
	set<NodeId> possibleNodesOnWrSet, possibleNodesSet;

	BOOST_FOREACH(RoadId rd, edges(_graph))
	{
		// create bounding box for cd
		Vector2 cdP, cdE;
		createAABB(getSrcNode(rd)->getPosition2D(), getDstNode(rd)->getPosition2D(), cdP, cdE);

		// check if they intersect
		Vector2 T = cdP - abP;//vector from A to B
		if(Math::Abs(T.x) <= (abE.x + cdE.x) && Math::Abs(T.y) <= (abE.y + cdE.y))
		{
			RoadInterface* ri = getRoad(rd);
			if(typeid(*ri)==typeid(WorldRoad))
			{
				possibleWorldRoads.push_back(rd);
				possibleNodesOnWrSet.insert(getSrc(rd));
				possibleNodesOnWrSet.insert(getDst(rd));
			}
			else
			{
				possibleRoads.push_back(rd);
				possibleNodesSet.insert(getSrc(rd));
				possibleNodesSet.insert(getDst(rd));
			}
		}
	}
	possibleNodes.insert(possibleNodes.end(), possibleNodesSet.begin(), possibleNodesSet.end());
	possibleNodesOnWr.insert(possibleNodesOnWr.end(), possibleNodesOnWrSet.begin(), possibleNodesOnWrSet.end());
}

bool RoadGraph::findClosestNodeSnap(const NodeId aNode, const Vector2& b, const Real snapSz, 
									const vector<NodeId> &nodes, Real &lowestR, NodeId &snapNode) const
{
	Vector2 a(_graph[aNode]->getPosition2D());
	Real snapSzSq = Math::Sqr(snapSz);
	Vector2 c, d;
	Vector2 ab(b - a);
	Real bxMinusAx(ab.x);
	Real byMinusAy(ab.y);
	Real Lsq = ab.squaredLength();
	Real L = Math::Sqrt(Lsq);
	Real stretchR(lowestR + snapSz/L);
	Real r, s;
	Real closestDistToBSq = snapSzSq;
	bool nodeSnapped = false;

	BOOST_FOREACH(NodeId nd, nodes)
	{
		NodeInterface* ni = _graph[nd];
		Vector2 c(ni->getPosition2D());

		// r = ((Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)) / L^2
		Ogre::Real cxMinusAx(c.x-a.x);
		Ogre::Real cyMinusAy(c.y-a.y);
		r = (cxMinusAx*bxMinusAx + cyMinusAy*byMinusAy) / Lsq;

		// s = ((Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)) / L^2
		s = (-cyMinusAy*bxMinusAx+cxMinusAx*byMinusAy) / Lsq;

		// install node data
		ni->_r = r;
		ni->_s = s;
		//Ogre::LogManager::getSingleton().logMessage(StringConverter::toString(b)+": "+StringConverter::toString(r));

		if (r >= 0)
		{
			if (r <= lowestR)
			{
				Real distance = Ogre::Math::Abs(s) * L;
				if (distance < snapSz && nd != aNode)
				{
					lowestR = r;
					snapNode = nd;
					nodeSnapped = true; //gk 0208
				}
			}
			else if (r < stretchR && lowestR == 1)
			{
				// test the region on the extension of ab
				Real distSq = (c - b).squaredLength();
				if (distSq < closestDistToBSq)
				{
					closestDistToBSq = distSq;
					snapNode = nd;
					nodeSnapped = true;
				}
			}
		}
	}
	return nodeSnapped;
}

bool RoadGraph::findClosestRoadIntersection(const NodeId aNode, const Vector2& b, const vector<RoadId> &roads, 
								 Real &lowestR, RoadId &iRoad, Vector3& iPoint) const
{
	Vector2 a(_graph[aNode]->getPosition2D());
	Vector2 c, d;
	Vector2 ab(b - a);
	Real r,s;
	Real bxMinusAx(ab.x);
	Real byMinusAy(ab.y);
	bool intersection = false;

	//size_t count = 0, execcount = 0;
	BOOST_FOREACH(RoadId rd, roads)
	{
		//tc++;
		NodeId cNd(source(rd, _graph));
		NodeId dNd(target(rd, _graph));
		NodeInterface* cNi = _graph[cNd];
		NodeInterface* dNi = _graph[dNd];

		// exclude r: outside segment ab or a(last snap point)
		//if(cNi->_r > lowestR && dNi->_r > lowestR || cNi->_r < 0 && dNi->_r < 0)
		//	continue;

		//// exclude s: on same side
		//if(cNi->_s > 1 && dNi->_s > 1 || cNi->_s < 0 && dNi->_s < 0)
		//	continue;

		// perform intersection test
		c = cNi->getPosition2D();
		d = dNi->getPosition2D();
		Real dxMinusCx(d.x - c.x);
		Real dyMinusCy(d.y - c.y);
		Ogre::Real denom = (bxMinusAx * dyMinusCy) - (byMinusAy * dxMinusCx);

		// line are parallel
		if (denom == 0)
			continue;

		Real axMinusCx(a.x - c.x);
		Real ayMinusCy(a.y - c.y);
		r = ((ayMinusCy * dxMinusCx) - (axMinusCx * dyMinusCy)) / denom;
		s = ((ayMinusCy * bxMinusAx) - (axMinusCx * byMinusAy)) / denom;

		//if r and s are 0 then the line are coincident (on top of one another)
		if (r == 0 && s == 0)
			continue;

		// if outside segment cd
		if (s< 0 || s> 1) continue ;

		if(r >= 0 && r < lowestR)
		{
			// skip connected segments
			if(cNd == aNode || dNd == aNode) continue;

			lowestR = r;
			iPoint.x = a.x + bxMinusAx * r;
			iPoint.y = _graph[cNd]->getPosition3D().y + s*(_graph[dNd]->getPosition3D().y - _graph[cNd]->getPosition3D().y);
			iPoint.z = a.y + byMinusAy * r;
			intersection = true;
			iRoad = rd;
		}
	}
	return intersection;
}

bool RoadGraph::snapToRoadNode(const Vector3 &p, const Real snapSz, const RoadId rd, NodeId &nd) const
{
	Vector2 p2D(p.x, p.z);
	Vector2 c(getSrcNode(rd)->getPosition2D()), d(getDstNode(rd)->getPosition2D());
	
	Real cDist((p2D - c).squaredLength()), dDist((p2D - d).squaredLength());
	if(cDist < dDist)
	{
		if(cDist < Math::Sqr(snapSz))
		{
			nd = getSrc(rd);
			return true;
		}
	}
	else
	{
		if(dDist < Math::Sqr(snapSz))
		{
			nd = getDst(rd);
			return true;
		}
	}
	return false;
}

bool RoadGraph::findClosestSnapRoad(const Vector2& b, const vector<RoadId> &roads, Real &closestDist, 
									RoadId &sRoad, Vector3& sPoint) const
{
	// last test
	bool roadSnapped = false;
	Vector2 c,d;
	Real L, Lsq, r,s;
	BOOST_FOREACH(RoadId rd, roads)
	{
		NodeId cNd(source(rd,_graph));
		NodeId dNd(target(rd,_graph));
		c = _graph[cNd]->getPosition2D();
		d = _graph[dNd]->getPosition2D();
		Vector2 cd(d - c);
		Lsq = cd.squaredLength();
		L = Math::Sqrt(Lsq);

		Real dxMinusCx(cd.x), dyMinusCy(cd.y);
		Real bxMinusCx(b.x-c.x), byMinusCy(b.y-c.y);
		r = (bxMinusCx*dxMinusCx + byMinusCy*dyMinusCy) / Lsq;

		if(r < 0 || r > 1) continue;

		s = (-byMinusCy*dxMinusCx+bxMinusCx*dyMinusCy) / Lsq;

		Real distance = Ogre::Math::Abs(s) * L;
		if(distance < closestDist)
		{
			closestDist = distance;
			sRoad = rd;
			sPoint.x = c.x + r*dxMinusCx;
			sPoint.y = _graph[cNd]->getPosition3D().y + r*(_graph[dNd]->getPosition3D().y - _graph[cNd]->getPosition3D().y);
			sPoint.z = c.y + r*dyMinusCy;
			roadSnapped = true;
		}
	}
	return roadSnapped;
}

#define MIN_SNAP_SZ 5

int RoadGraph::snapInfo(const NodeId aNode, const Vector2& b,
		const Real snapSz, Vector3& pos, NodeId &nodeId, RoadId& roadId) const
{

	Vector2 a(_graph[aNode]->getPosition2D());

	// calculate a list of bounding box matches to reduce testing
	vector<RoadId> possibleRoads, possibleWorldRoads;
	possibleRoads.reserve(256);
	possibleWorldRoads.reserve(256);
	vector<NodeId> possibleNodesOnWr, possibleNodes;
	calculateBoundingBoxFriends(a, b, snapSz, possibleRoads, possibleWorldRoads, possibleNodes, possibleNodesOnWr);

	Real lowestR(1.0f);
	NodeId snapNode;
	bool nodeSnapped = false;
	nodeSnapped = findClosestNodeSnap(aNode,b,snapSz,possibleNodes,lowestR,snapNode);
	if(nodeSnapped) // crazy prioritisation of node snaps
	{
		nodeId = snapNode;
		return 2;
	}
	//nodeSnapped |= findClosestNodeSnap(aNode,b,MIN_SNAP_SZ,possibleNodesOnWr,lowestR,snapNode);

	RoadId iRoad;
	Vector3 iPoint;
	bool intersection = false;
	intersection = findClosestRoadIntersection(aNode,b,possibleRoads,lowestR,iRoad,iPoint);
	intersection |= findClosestRoadIntersection(aNode,b,possibleWorldRoads,lowestR,iRoad,iPoint);

	if(intersection)
	{
		RoadInterface* ri = getRoad(iRoad);
		if(typeid(*ri) == typeid(WorldRoad))
			nodeSnapped = snapToRoadNode(iPoint,MIN_SNAP_SZ,iRoad,snapNode);
		else
			nodeSnapped = snapToRoadNode(iPoint,snapSz,iRoad,snapNode);

		if(!nodeSnapped)
		{
			pos = iPoint;
			roadId = iRoad;
			return 1;
		}
	}
	
	if(nodeSnapped)
	{
		nodeId = snapNode;
		return 2;
	}

	Real closestDistance = snapSz;
	bool roadSnapped = findClosestSnapRoad(b, possibleRoads, closestDistance, iRoad, iPoint);
	roadSnapped |= findClosestSnapRoad(b, possibleWorldRoads, closestDistance, iRoad, iPoint);

	if(closestDistance < snapSz)
	{
		RoadInterface* ri = getRoad(iRoad);
		if(typeid(*ri) == typeid(WorldRoad))
			nodeSnapped = snapToRoadNode(iPoint,MIN_SNAP_SZ,iRoad,snapNode);
		else
			nodeSnapped = snapToRoadNode(iPoint,snapSz,iRoad,snapNode);

		if(!nodeSnapped)
		{
			roadId = iRoad;
			pos = iPoint;
			return 1;
		}
		else
		{
			nodeId = snapNode;
			return 2;
		}
	}
	else if(findClosestNodeSnap(aNode,b,snapSz,possibleNodesOnWr,lowestR,snapNode))
	{
		nodeId = snapNode;
		return 2;
	}
	return 0;
}

/*
int RoadGraph::snapInfo(const NodeId aNode, const Vector2& b,
		const Real snapSz, Vector3& pos, NodeId &nodeId, RoadId& roadId) const
{

	Vector2 a(_graph[aNode]->getPosition2D());

	vector<RoadId> possibleSnapRoads;
	vector<NodeId> possibleSnapNodes;
	possibleSnapRoads.reserve(256);
	possibleSnapNodes.reserve(256);

	// create bounding box vectors for segment ab
	Vector2 abP, abE;
	createAABB(a, b, abP, abE);
	abE.x += snapSz;
	abE.y += snapSz;
	//uint16 roundCount = 0;

	BOOST_FOREACH(RoadId rd, edges(_graph))
	{
		// create bounding box for cd
		Vector2 cdP, cdE;
		createAABB(getSrcNode(rd)->getPosition2D(), getDstNode(rd)->getPosition2D(), cdP, cdE);

		// check if they intersect
		Vector2 T = cdP - abP;//vector from A to B
		if(Math::Abs(T.x) <= (abE.x + cdE.x) && Math::Abs(T.y) <= (abE.y + cdE.y))
		{
			possibleSnapRoads.push_back(rd);
			//if(typeid(*(_graph[rd])) != typeid(WorldRoad))
			{
				possibleSnapNodes.push_back(getSrc(rd));
				possibleSnapNodes.push_back(getDst(rd));
			}
		}
		//roundCount++;
	}
	//LogManager::getSingleton().logMessage("BB filter: "+StringConverter::toString(possibleSnapRoads.size()) + "/" +StringConverter::toString(roundCount));

	Real snapSzSq = Math::Sqr(snapSz);
	Vector2 c, d;
	Vector2 ab(b - a);
	Real bxMinusAx(ab.x);
	Real byMinusAy(ab.y);
	Real Lsq = ab.squaredLength();
	Real L = Math::Sqrt(Lsq);
	Real lowestR(1.0f);
	Real stretchR(lowestR + snapSz/L);
	NodeId snapNode;
	Real r, s;
	RoadId intersectingRoad;
	Real closestDistToBSq = snapSzSq;
	bool nodeSnapped = false;

	BOOST_FOREACH(NodeId nd, possibleSnapNodes)
	{
		NodeInterface* ni = _graph[nd];
		Vector2 c(ni->getPosition2D());

		//TEST HACK
		//		if(c == b) 
		//			continue;

		// r = ((Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)) / L^2
		Ogre::Real cxMinusAx(c.x-a.x);
		Ogre::Real cyMinusAy(c.y-a.y);
		r = (cxMinusAx*bxMinusAx + cyMinusAy*byMinusAy) / Lsq;

		// s = ((Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)) / L^2
		s = (-cyMinusAy*bxMinusAx+cxMinusAx*byMinusAy) / Lsq;

		// install node data
		ni->_r = r;
		ni->_s = s;

		if (r >= 0)
		{
			if (r <= lowestR)
			{
				Real distance = Ogre::Math::Abs(s) * L;
				if (distance < snapSz && nd != aNode)
				{
					lowestR = r;
					snapNode = nd;
					nodeSnapped = true; //gk 0208
				}
			}
			else if (r < stretchR && lowestR == 1)
			{
				// test the region on the extension of ab
				Real distSq = (c - b).squaredLength();
				if (distSq < closestDistToBSq)
				{
					closestDistToBSq = distSq;
					snapNode = nd;
					nodeSnapped = true;
				}
			}
		}
	}

	bool intersection = false;
	Real intersectionY = 0.0f;
	//size_t count = 0, execcount = 0;
	BOOST_FOREACH(RoadId rd, possibleSnapRoads)
	{
		//tc++;
		NodeId cNd(source(rd, _graph));
		NodeId dNd(target(rd, _graph));
		NodeInterface* cNi = _graph[cNd];
		NodeInterface* dNi = _graph[dNd];

		//// exclude r: outside segment ab or a(last snap point)
		//if (cNi->_r > lowestR && dNi->_r > lowestR)
		//	continue;
		//if (cNi->_r < 0 && dNi->_r < 0)
		//	continue;

		//// exclude s: on same side
		//if (cNi->_s > 1 && dNi->_s > 1)
		//	continue;
		//if (cNi->_s < 0 && dNi->_s < 0)
		//	continue;

		//te++;

		// perform intersection test
		c = cNi->getPosition2D();
		d = dNi->getPosition2D();
		Real dxMinusCx(d.x - c.x);
		Real dyMinusCy(d.y - c.y);
		Ogre::Real denom = (bxMinusAx * dyMinusCy) - (byMinusAy * dxMinusCx);

		// line are parallel
		if (denom == 0)
			continue;

		Real axMinusCx(a.x - c.x);
		Real ayMinusCy(a.y - c.y);
		r = ((ayMinusCy * dxMinusCx) - (axMinusCx * dyMinusCy)) / denom;
		s = ((ayMinusCy * bxMinusAx) - (axMinusCx * byMinusAy)) / denom;

		//if r and s are 0 then the line are coincident (on top of one another)
		if (r == 0 && s == 0)
			continue;

		// if outside segment cd
		if (s< 0 || s> 1)continue ;

		if(r >= 0 && r <= 1)
		{
			// skip connected segments
			if(cNd == aNode || dNd == aNode) continue;

			//HACK: this basic subsidises world road intersection against 
			// node snaps, this whole function probably need rethought
			/*if(typeid(*(_graph[rd]))==typeid(WorldRoad) && nodeSnapped)
			{
				if(r < (lowestR + (stretchR - 1)))
				{
					intersection = true;
					intersectingRoad = rd;
					intersectionY = _graph[cNd]->getPosition3D().y + s*(_graph[dNd]->getPosition3D().y - _graph[cNd]->getPosition3D().y);
					lowestR = r;
				}
			}
			else 
			if(r < lowestR)
			{
				lowestR = r;

				Vector2 p(a.x + bxMinusAx * r, a.y + byMinusAy * r);

				if(s < 0.5)
				{
					if((c-p).squaredLength() < snapSzSq && typeid(*(_graph[rd]))!=typeid(WorldRoad))
					{
						intersection = false;
						snapNode = cNd;
					}
					else
					{
						intersection = true;
						intersectingRoad = rd;
						intersectionY = _graph[cNd]->getPosition3D().y + s*(_graph[dNd]->getPosition3D().y - _graph[cNd]->getPosition3D().y);
					}
				}
				else
				{
					if((d-p).squaredLength() < snapSzSq && typeid(*(_graph[rd]))!=typeid(WorldRoad))
					{
						intersection = false;
						snapNode = dNd;
					}
					else
					{
						intersection = true;
						intersectingRoad = rd;
						intersectionY = _graph[cNd]->getPosition3D().y + s*(_graph[dNd]->getPosition3D().y - _graph[cNd]->getPosition3D().y);
					}
				}
			}
		}
	}
	//LogManager::getSingleton().logMessage("Test 2: "+StringConverter::toString(tc)+":"+StringConverter::toString(te));

	if(intersection)
	{
		pos.x = a.x + lowestR * bxMinusAx; //lowestR: 0 ??
		pos.y = intersectionY;
		pos.z = a.y + lowestR * byMinusAy;
		roadId = intersectingRoad;
		return 1;
	}
	else if(lowestR < 1)
	{
		nodeId = snapNode;
		return 2;
	}

	// can i exclude this test like the other, using r

	// shit no the bounds test can exclude this


	// last test
	RoadId snapRoad;
	Vector3 snapPos;
	Real closestDistToB = Math::Sqrt(closestDistToBSq);
	BOOST_FOREACH(RoadId rd, possibleSnapRoads)
	{
		NodeId cNd(source(rd,_graph));
		NodeId dNd(target(rd,_graph));
		c = _graph[cNd]->getPosition2D();
		d = _graph[dNd]->getPosition2D();
		Vector2 cd(d - c);
		Lsq = cd.squaredLength();
		L = Math::Sqrt(Lsq);

		//TESTHACK
		//		if(c == b || d == b) 
		//			continue;

		Ogre::Real dxMinusCx(cd.x);
		Ogre::Real dyMinusCy(cd.y);
		Ogre::Real bxMinusCx(b.x-c.x);
		Ogre::Real byMinusCy(b.y-c.y);

		r = (bxMinusCx*dxMinusCx + byMinusCy*dyMinusCy) / Lsq;

		if(r < 0 || r > 1) continue;

		s = (-byMinusCy*dxMinusCx+bxMinusCx*dyMinusCy) / Lsq;

		Real distance = Ogre::Math::Abs(s) * L;
		if(distance < closestDistToB)
		{
			closestDistToB = distance;
			Vector2 p(c.x + r*dxMinusCx, c.y + r*dyMinusCy);
			if((p - c).squaredLength() < snapSzSq && typeid(*(_graph[rd]))!=typeid(WorldRoad))
			{
				nodeSnapped = true;
				snapNode = cNd;
			}
			else if((p - d).squaredLength() < snapSzSq && typeid(*(_graph[rd]))!=typeid(WorldRoad))
			{
				nodeSnapped = true;
				snapNode = dNd;
			}
			else
			{
				snapRoad = rd;
				snapPos.x = p.x;
				snapPos.y = _graph[cNd]->getPosition3D().y + r*(_graph[dNd]->getPosition3D().y - _graph[cNd]->getPosition3D().y);
				snapPos.z = p.y;
				nodeSnapped = false;
			}
		}
	}
	if(closestDistToB < snapSz)
	{
		if(!nodeSnapped)
		{
			roadId = snapRoad;
			pos = snapPos;
			return 1;
		}
		else
		{
			nodeId = snapNode;
			return 2;
		}
	}
	return 0;

}*/

bool RoadGraph::findClosestIntersection(const std::vector<NodeId>& ignore,
		const Vector2& b, const Real snapSz, Vector2& pos, RoadId& roadId) const
{
	// function vars
	Real bR, bS, cR, cS, dR, dS, lowestR=std::numeric_limits<Real>::max();
	Vector2 bestPoint(Vector2::ZERO);
	RoadId intersectingRoad;

	// get vars for seg AB
	Vector2 a(getNode(ignore[0])->getPosition2D());
	Vector2 BminusA(b - a);
	Real Labsq = BminusA.squaredLength();
	Real Lab = Math::Sqrt(Labsq);

	BOOST_FOREACH(RoadId rd, edges(_graph))
	{
		NodeId cNode(getSrc(rd)), dNode(getDst(rd));
		bool skip = false;

		// skip can lead to dangers, this func is only used by the gui so is not too bad
		BOOST_FOREACH(NodeId ignoreId, ignore) if(cNode == ignoreId || dNode == ignoreId) skip = true;
		if(skip) continue;

		Vector2 c(getNode(cNode)->getPosition2D());
		Vector2 d(getNode(dNode)->getPosition2D());
		Vector2 CminusA(c - a), DminusA(d - a);

		//// point b to line segment cd
		if(lowestR > 1)
		{
			Vector2 DminusC(d - c);
			Vector2 BminusC(b - c);
			Real Lcdsq = DminusC.squaredLength();
			bR = (BminusC.x*DminusC.x + BminusC.y*DminusC.y) / Lcdsq;
			bS = (-BminusC.y*DminusC.x+BminusC.x*DminusC.y) / Lcdsq;
			if(bR >= 0 && bR <= 1)
			{
				if((Math::Abs(bS) * Math::Sqrt(Lcdsq)) < snapSz)
				{
					Vector2 p(c.x + bR*DminusC.x, c.y + bR*DminusC.y);
					bR = (a-p).length() / Lab;
					if(bR < lowestR)
					{
						bestPoint = p;
						intersectingRoad = rd;
						lowestR = bR;
					}
				}
			}
		}

		//
		// point c to line segment ab
		cR = (CminusA.x*BminusA.x + CminusA.y*BminusA.y) / Labsq;
		cS = (-CminusA.y*BminusA.x+CminusA.x*BminusA.y) / Labsq;
		if(cR >= 0 && cR < lowestR && cR <= 1 && (Math::Abs(cS) * Lab) < snapSz)
		{
			lowestR = cR;
			bestPoint = c;
			intersectingRoad = rd;
		}

		//
		// point d to line segment ab
		dR = (DminusA.x*BminusA.x + DminusA.y*BminusA.y) / Labsq;
		dS = (-DminusA.y*BminusA.x+DminusA.x*BminusA.y) / Labsq;
		if(dR >= 0 && dR < lowestR && dR <= 1 && (Math::Abs(dS) * Lab) < snapSz)
		{
			lowestR = dR;
			bestPoint = d;
			intersectingRoad = rd;
		}

		// if c and d are on the same side of line seg AB
		//	- no chance of an intersection
		if(cS == dS) continue;

		// if c and d are too far down the line seg
		//  - no chance for intersection to be further down
		if(cR > lowestR && dR > lowestR) continue;

		//
		// line intersection
		Vector2 DminusC(d - c);
		Real denom = (BminusA.x * DminusC.y) - (BminusA.y * DminusC.x);
		if(denom == 0) continue; // parallel		
		Real r = ((-CminusA.y * DminusC.x) + (CminusA.x * DminusC.y)) / denom;
		Real s = ((-CminusA.y * BminusA.x) + (CminusA.x * BminusA.y)) / denom;
		if(r == 0 && s == 0) continue; // coincident (on top of one another)
		if(s >= 0 && s <= 1 && r >= 0 && r < lowestR && r<=1)
		{
			Vector2 p(a.x + r * (BminusA.x), a.y + r * (BminusA.y));
			bestPoint = p;
			intersectingRoad = rd;
			lowestR = r;
		}
	}

	// Write back results if success
	if (lowestR < std::numeric_limits<Real>::max())
	{
		pos = bestPoint;
		roadId = intersectingRoad;
		return true;
	}
	return false;
}

bool RoadGraph::findClosestRoad(const NodeId aNode, const Real snapSz,
		Vector2& pos, RoadId& roadId) const
{
	// function vars
	Real currentDistance, closestDistanceSq=std::numeric_limits<Real>::max();
	Vector2 bestPoint(Vector2::ZERO), a(getNode(aNode)->getPosition2D());
	RoadId intersectingRoad;
	Real snapSzSq = Math::Sqr(snapSz);

	BOOST_FOREACH(RoadId rd, edges(_graph))
	{
		NodeId cNode(getSrc(rd)), dNode(getDst(rd));
		Vector2 c(getNode(cNode)->getPosition2D());
		Vector2 d(getNode(dNode)->getPosition2D());

		//// point a to line segment cd
		Vector2 DminusC(d - c);
		Vector2 AminusC(a - c);
		Real Lcdsq = DminusC.squaredLength();
		Real aR = (AminusC.x*DminusC.x + AminusC.y*DminusC.y) / Lcdsq;
		Real aS = (-AminusC.y*DminusC.x+AminusC.x*DminusC.y) / Lcdsq;
		if(aR >= 0 && aR <= 1)
		{
			currentDistance = Math::Abs(aS) * Math::Sqrt(Lcdsq);
			if(currentDistance < snapSz)
			{
				currentDistance = Math::Sqr(currentDistance);
				if(currentDistance < closestDistanceSq)
				{
					closestDistanceSq = currentDistance;
					intersectingRoad = (rd);
					bestPoint = Vector2(c.x + aR*DminusC.x, c.y + aR*DminusC.y);
				}
			}
		}
		else
		{
			if(aR < 0)
			{
				Real LacSq(AminusC.squaredLength());
				if(LacSq < snapSzSq && LacSq < closestDistanceSq)
				{
					closestDistanceSq = LacSq;
					bestPoint = c;
					intersectingRoad = rd;
				}
			}
			else
			{
				Vector2 AminusD(a - d);
				Real LadSq(AminusD.squaredLength());
				if(LadSq < snapSzSq && LadSq < closestDistanceSq)
				{
					closestDistanceSq = LadSq;
					bestPoint = d;
					intersectingRoad = rd;
				}
			}
		}
	}
	// Write back results if success
	if (closestDistanceSq < std::numeric_limits<Real>::max())
	{
		pos = bestPoint;
		roadId = intersectingRoad;
		return true;
	}
	return false;
}

bool RoadGraph::findClosestIntscnConnected(const NodeId aNode,
		const NodeId bNode, const Ogre::Real snapSz, Ogre::Vector2& pos,
		RoadId& rd) const
{
	std::vector<NodeId> ignore(2);
	ignore[0] = aNode;
	ignore[1] = bNode;
	Ogre::Vector2 bPos = getNode(bNode)->getPosition2D();
	return findClosestIntersection(ignore, bPos, snapSz, pos, rd);
}

struct NodeInfo
{
	const NodeId _id;
	const Vector2 _pos;
	NodeInfo(const NodeId id, const Vector2& pos) :
		_id(id), _pos(pos)
	{
	}
};
bool operator<(const NodeInfo &l, const NodeInfo &r)
{
	return l._pos.x < r._pos.x;
}

bool RoadGraph::getClockwiseCycle(NodeId v0, NodeId v1, Graph &g, std::vector<NodeId> &cycle)
{
	// find a cycle terminating when we go from v0 to v1 again.
	NodeId prevNode = v0, currNode = v1, nextNode;
	cycle.push_back(v1);
	while(true)
	{
		if(out_degree(currNode,g) == 1)
		{
			// dead end, lets turn around and go back the other side
			if(currNode == v0 && prevNode == v1) break;
			cycle.push_back(prevNode);
			std::swap(prevNode, currNode);
		}
		else
		{
			// get next node
			bool vertexFound = getClockwiseMostFromPrev(prevNode, currNode, nextNode, g);
			if(vertexFound)
			{
				if(currNode == v0 && nextNode == v1) break;
				cycle.push_back(nextNode);
				prevNode = currNode;
				currNode = nextNode;
			}
			else LogManager::getSingleton().logMessage("FUCK");
		}
	}
	return true;
}

bool RoadGraph::getAntiClockwiseCycle(NodeId v0, NodeId v1, Graph &g, std::vector<NodeId> &cycle)
{
	// find a cycle terminating when we go from v0 to v1 again.
	NodeId prevNode = v0, currNode = v1, nextNode;
	cycle.push_back(v1);
	while(true)
	{
		if(currNode == v0)
			size_t z=0;

		if(out_degree(currNode,g) == 1)
		{
			// dead end, lets turn around and go back the other side
			if(currNode == v0 && prevNode == v1) break;
			cycle.push_back(prevNode);
			std::swap(prevNode, currNode);
		}
		else
		{
			// get next node
			bool vertexFound = getAntiClockwiseMostFromPrev(prevNode, currNode, nextNode, g);
			if(vertexFound)
			{
				if(currNode == v0 && nextNode == v1) break;
				cycle.push_back(nextNode);
				prevNode = currNode;
				currNode = nextNode;
			}
			else LogManager::getSingleton().logMessage("FUCK");
		}
	}
	return true;
}

void RoadGraph::extractEnclosedRegions(std::vector< std::vector<NodeInterface*> > &polys, size_t limit)
{
	// make a copy of local graph
	Graph g(_graph);
	set<RoadId, road_less_than> visitedRoads;
	std::list<NodeId> heap;
	{
		set<NodeInfo> heap2;
		BOOST_FOREACH(NodeId nd, vertices(g))
		heap2.insert(NodeInfo(nd, g[nd]->getPosition2D()));
		//DEBUG:
		//ostringstream oss;
		//BOOST_FOREACH(NodeInfo &n, heap2) oss << n._id << ": " << n._pos << "\n";
		//LogManager::getSingleton().logMessage(oss.str());
		BOOST_FOREACH(NodeInfo n, heap2) if(out_degree(n._id, g)) heap.insert(heap.end(), n._id);
	}

	try
	{
		// add perimeter roads to visited set
		{
			// get first node
			NodeId v0 = *(heap.begin());
			
			// get next node
			NodeId v1;
			bool vertexFound = getClockwiseMost(v0, v1, g);
			if(vertexFound)
			{	
				std::vector<NodeId> cycle;
				getClockwiseCycle(v0, v1, g, cycle);
				for(size_t i=0; i<cycle.size(); i++)
				{
					size_t j = (i+1) % cycle.size();
					visitedRoads.insert(edge(cycle[i], cycle[j], g).first);
					//LogManager::getSingleton().logMessage(StringConverter::toString(g[edge(cycle[i], cycle[j], g).first]->_visitCount));
				}
			}
		}

		//while (heap is not empty) do
		size_t i,count=0;
		for(i=0; i<1000 && heap.size() != 0 && count<limit; i++)
		{
			NodeId v0 = *(heap.begin());

			if(out_degree(v0, g))
			{
				// get next node
				NodeId v1;
				bool vertexFound = getClockwiseMost(v0, v1, g);
				if(vertexFound)
				{	
					std::vector<NodeId> cycle;
					getAntiClockwiseCycle(v0, v1, g, cycle);
					vector<NodeInterface*> poly;
					BOOST_FOREACH(NodeId nd, cycle) poly.push_back(g[nd]);
					polys.push_back(poly);
					for(size_t i=0; i<cycle.size(); i++)
					{
						size_t j = (i+1) % cycle.size();
						RoadId rd;
						bool b;
						tie(rd, b) = edge(cycle[i], cycle[j], g);
						if(b)
						{
							if(visitedRoads.find(rd) != visitedRoads.end()) remove_edge(rd, g);
							else visitedRoads.insert(rd);
						}
						//LogManager::getSingleton().logMessage(StringConverter::toString(g[edge(cycle[i], cycle[j], g).first]->_visitCount));
					}
				}
				count++;
			}
			else
			{
				remove_vertex(v0, g);
				removeFromHeap(v0, heap);
			}
		}

		if(i>=1000)
		LogManager::getSingleton().logMessage("Primitive Infinitum");

	}
	catch(Exception e)
	{
		LogManager::getSingleton().logMessage(e.getDescription());
	}
}
