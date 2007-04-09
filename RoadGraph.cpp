#include "stdafx.h"
#include "RoadGraph.h"
#include "NodeInterface.h"
#include "RoadInterface.h"
#include "Geometry.h"

using namespace std;
using namespace boost;
using namespace Ogre;

RoadGraph::RoadGraph()
{
}


bool RoadGraph::addRoad(const NodeId nd1, const NodeId nd2, RoadId& rd)
{
	assert(nd1 != nd2);
	bool inserted = false;
	tie(rd, inserted) = add_edge(nd1, nd2, mGraph);
	return inserted;
}

bool RoadGraph::addRoad(const NodeId nd1, const NodeId nd2, RoadInterface* r, RoadId& rd)
{
	bool inserted = false;
	tie(rd, inserted) = add_edge(nd1, nd2, r, mGraph);
	return inserted;
}

bool RoadGraph::findRoad(const NodeId nd1, const NodeId nd2, RoadId& rd)
{
	RoadId r;
	bool found = false;
	tie(r, found) = edge(nd1, nd2, mGraph);
	if(found) rd = r;
	return found;
}

bool RoadGraph::sortVertex(const NodeId& v0, const NodeId& v1)
{
	Real x0 = mGraph[v0]->getPosition2D().x;
	Real x1 = mGraph[v1]->getPosition2D().x;
	return (x0 < x1);
}
    
void RoadGraph::extractPrimitives(vector<RoadInterface*> &filaments, 
								  vector< vector<NodeInterface*> > &nodeCycles, 
								  vector< vector<RoadInterface*> > &roadCycles)
{
	// create a copy of our road graph to work on
	Graph g(mGraph);

	std::ostringstream oss; 
	oss << " ";

	//init roads
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = edges(g); rIt != rEnd; rIt++)
		g[*rIt]->setRoadCycle(false);

	//set<NodeId> heap = vertices; 
	//gk: at the moment I'm using a vertex to store vertices so they are already sorted
	graph_traits<Graph>::vertex_iterator i, end;
	tie(i, end) = vertices(g);
	//set<NodeId> heap(i, end);
	
	//SET needs sort
	//sort(heap.begin(), heap.end(), comp());
	//it would be neater o use a comparator but our comparision isn't standalone
	std::list< NodeId > heap;

	//insert one
	heap.push_back(*i++);

	for(; i != end; ++i)
	{
		for(std::list<NodeId>::iterator tit = heap.begin(); true; tit++)
		{
			if(tit == heap.end()) {
				heap.push_back(*i);
				break;
			}
			else if(sortVertex(*i, *tit))
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

	try {
	
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
			 extractPrimitive(v0, g, heap, filaments, nodeCycles, roadCycles); // filament or minimal cycle
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
		LogManager::getSingleton().logMessage(e.getDescription());

		//DANGER - setting partially decomposed graph to real graph
		//mGraph = g;
	}

}

void RoadGraph::extractFilament(NodeId v0, NodeId v1, Graph &g, list<NodeId>& heap, 
								vector<RoadInterface*> &filaments)
{
	//assert(out_degree(v0, g) != 2); // trouble
	if(out_degree(v0, g) == 2) {
		throw Exception(Exception::ERR_ITEM_NOT_FOUND, "ERROR: incorrect filament degree", "RoadGraph::extractFilament");
	}

	if(g[edge(v0, v1, g).first]->isRoadCycle())
	{
		if(out_degree(v0, g) >= 3)		
		{
			remove_edge(v0, v1, g);
			v0 = v1;
			if(out_degree(v0, g) == 1)
			{
				v1 = getFirstAdjacent(v0, g);
			}

		}
		while(out_degree(v0, g) == 1)
		{
			v1 =  getFirstAdjacent(v0, g);

			if(g[edge(v0, v1, g).first]->isRoadCycle())
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
		if(out_degree(v0, g) == 0)
		{
			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_vertex(v0, g);
		}
	}
	else
	{
		if(out_degree(v0, g) >= 3)
		{
			filaments.push_back(g[getFirstRoad(v0, g)]);
			remove_edge(v0, v1, g);
			v0 = v1;
			if(out_degree(v0, g) == 1)
			{
				v1 =  getFirstAdjacent(v0, g);
			}
		}

		while (out_degree(v0, g) == 1)
		{
			v1 =  getFirstAdjacent(v0, g);
			filaments.push_back(g[edge(v0, v1, g).first]);

			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_edge(v0, v1, g);
			remove_vertex(v0, g);
			v0 = v1;
		}

		if(out_degree(v0, g) == 0)
		{
			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_vertex(v0, g);
		}
	}
}

void RoadGraph::extractPrimitive(NodeId v0, Graph &g, list<NodeId>& heap, 
								 vector<RoadInterface*> &filaments, 
								 vector< vector<NodeInterface*> > &nodeCycles, 
								 vector< vector<RoadInterface*> > &roadCycles)
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
	while(vertexFound && vcurr != v0 && visited.find(vcurr) == visited.end())
	{
		nodeCycle.push_back(g[vcurr]);
		roadCycle.push_back(g[edge(vprev, vcurr, g).first]);
		visited.insert(vcurr);
		vertexFound = getCounterClockwiseMostFromPrev(vprev, vcurr, vnext, g);
		vprev = vcurr;
		vcurr = vnext;
	}
	if(!vertexFound)
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
	else if(vcurr == v0)
	{
		// Minimal cycle found.

		// add the last edge
		nodeCycle.push_back(g[vcurr]);
		roadCycle.push_back(g[edge(vprev, vcurr, g).first]);

		// add a cell to our list
		nodeCycles.push_back(nodeCycle);
		roadCycles.push_back(roadCycle);

		// mark every edge in the cycle as a cycle edge
		vector< RoadInterface* >::iterator rIt, rEnd;
		for(rIt = roadCycle.begin(), rEnd = roadCycle.end(); rIt != rEnd; rIt++)
			(*rIt)->setRoadCycle(true);
		
		remove_edge(v0, v1, g);

		if(out_degree(v0, g) == 1)
		{
			// Remove the filament rooted at v0.
			extractFilament(v0,  getFirstAdjacent(v0, g), g, heap, filaments);
		}
		if(out_degree(v1, g) == 1)
		{
			// Remove the filament rooted at v1.
			extractFilament(v1,  getFirstAdjacent(v1, g), g, heap, filaments);
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
			if(getFirstAdjacent(v0, g) != v1)
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
	graph_traits<Graph>::out_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = out_edges(vcurr, g); ei != ei_end; ++ei)
	{
		NodeId vadj = target(*ei, g);
		
		Vector2 dadj = g[vadj]->getPosition2D()  - vcurr_pos;
		
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


bool RoadGraph::getCounterClockwiseMostFromPrev(NodeId vprev, NodeId vcurr, NodeId& vnext, const Graph& g) 
{
	bool isConvex = false, success = false;

	Vector2 vcurr_pos(g[vcurr]->getPosition2D());
	Vector2 vprev_pos(g[vprev]->getPosition2D());
	Vector2 dcurr = vcurr_pos - vprev_pos;
	Vector2 dnext;

	//for each adjacent vertex vadj of vcurr do
	graph_traits<Graph>::out_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = out_edges(vcurr, g); ei != ei_end; ++ei)
	{
		NodeId vadj = target(*ei, g);

		//No going back :)
		if(vadj == vprev) continue;


		Vector2 dadj = g[vadj]->getPosition2D() - vcurr_pos;
		
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
	std::list<NodeId>::iterator pos = heap.begin();
	for( ; pos != heap.end(); pos++)
	{
		if(*pos == v0) {
			heap.erase(pos);
			break;
		}
	}
}

RoadId RoadGraph::getFirstRoad(NodeId nd, const Graph &g)
{
	RoadIterator2 rIt, rEnd;
	tie(rIt, rEnd) = out_edges(nd, g);
	assert(rIt != rEnd);
	return *rIt;
}

NodeId RoadGraph::getFirstAdjacent(NodeId nd, const Graph &g)
{
	return target(getFirstRoad(nd, g), g);
}

RoadId RoadGraph::getSecondRoad(NodeId nd, const Graph &g)
{
	RoadIterator2 rIt, rEnd;
	tie(rIt, rEnd) = out_edges(nd, g);
	rIt++;
	assert(rIt != rEnd);
	return *rIt;
}

NodeId RoadGraph::getSecondAdjacent(NodeId nd, const Graph &g)
{
	return target(getSecondRoad(nd, g), g);
}


bool RoadGraph::roadIntersection(const Vector2& a, const Vector2& b, const RoadId rd, 
								 Vector2& intersection) const
{
	// get some values for our points
	return Geometry::lineSegmentIntersect(a, b, getNode(getSrc(rd))->getPosition2D(), 
		getNode(getDst(rd))->getPosition2D(), intersection);

}

bool RoadGraph::roadIntersection(const RoadId rd1, const RoadId rd2, Vector2& intersection) const
{
	// get some values for our points
	return Geometry::lineSegmentIntersect(getNode(getSrc(rd1))->getPosition2D(), getNode(getDst(rd1))->getPosition2D(),
		getNode(getSrc(rd2))->getPosition2D(), getNode(getDst(rd2))->getPosition2D(), intersection);

}

bool RoadGraph::findClosestIntersection(const Vector2& a, const Vector2& b, RoadId& rd, Vector2& intersection) const
{
	bool hasIntersection = false;
	Vector2 currentIntersection, closestIntersection;
	RoadId closestRoad;
	Ogre::Real currentDistance, closestDistance;
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		if(Geometry::lineSegmentIntersect(a, b, getNode(getSrc(*rIt))->getPosition2D(), 
			getNode(getDst(*rIt))->getPosition2D(), currentIntersection))
		{
			if(currentIntersection == a) continue;
			if(hasIntersection)
			{
				currentDistance = ( (a - currentIntersection).squaredLength() );
				if(currentDistance < closestDistance)
				{
					closestIntersection = currentIntersection;
					closestRoad = *rIt;
				}
			}
			else
			{
				hasIntersection = true;
				closestDistance = ( (a - currentIntersection).squaredLength() );
				closestIntersection = currentIntersection;
				closestRoad = *rIt;
			}
		}
	}

	if(hasIntersection)
	{
		intersection = closestIntersection;
		rd = closestRoad;
	}
	return hasIntersection;	
}


bool RoadGraph::findClosestIntersection3(const Vector2& a, const Vector2& b, 
					const NodeId& ignoreNode, RoadId& rd, Vector2& intersection) const
{
	bool hasIntersection = false;
	Vector2 currentIntersection, closestIntersection;
	RoadId closestRoad;
	Ogre::Real currentDistance, closestDistance;
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		if(source(*rIt, mGraph) != ignoreNode && target(*rIt, mGraph) != ignoreNode)
		{
			if(Geometry::lineSegmentIntersect(a, b, getNode(getSrc(*rIt))->getPosition2D(), 
				getNode(getDst(*rIt))->getPosition2D(), currentIntersection))
			{
				if(hasIntersection)
				{
					currentDistance = ( (a - currentIntersection).squaredLength() );
					if(currentDistance < closestDistance)
					{
						closestIntersection = currentIntersection;
						closestRoad = *rIt;
					}
				}
				else
				{
					hasIntersection = true;
					closestDistance = ( (a - currentIntersection).squaredLength() );
					closestIntersection = currentIntersection;
					closestRoad = *rIt;
				}
			}
		}
	}

	if(hasIntersection)
	{
		intersection = closestIntersection;
		rd = closestRoad;
	}
	return hasIntersection;	
}

// should definetely try to rewrite this too as it is a bit slow
// recursion is necessary as when a point is snapped we change the segment that we are testing
// and the recursion retests each new segment.
// takes a whopping 8937ms (215ms) to complete test case 2
// takes a whopping 8522ms (203ms) to complete test case 2 with the first recursion disabled
// takes a whopping 5577ms (156ms) to complete test case 2 with no recursion
int RoadGraph::findClosestSnappedIntersection(const Vector2& srcPos, const Vector2& dstPos, 
										const Ogre::Real& snapSzSquared, NodeId& nd,
										RoadId& rd, Vector2& pos) const
{
	NodeId snappedToNode;
	bool nodeSnapped = false;

	if(findClosestIntersection(srcPos, dstPos, rd, pos) && pos != srcPos)
	{
		// intersection!, try and snap to a node on the intersecting road
		if(snapToRoadNode(pos, rd, snapSzSquared, snappedToNode))
			nodeSnapped = true;
		else
		{
			if(pos != dstPos)
			{
				// Note: we have changed the proposed segment so we must recurse
				int res = findClosestSnappedIntersection(srcPos, pos, snapSzSquared, nd, rd, pos);
				if(res != 0) return res;
			}
			return 1;
		}
	}
	else
	{
		// no intersection!, try and snap to a node
		nodeSnapped = snapToNode(dstPos, snapSzSquared, snappedToNode);
	}

	if(nodeSnapped)
	{
		// check if the node is different to existing
		if(snappedToNode != nd)
		{
			// a new road section is proposed which must be considered
			nd = snappedToNode;

			// get the position of the new node & update dst node
			pos = getNode(nd)->getPosition2D();

			// recursively snap
			return findClosestSnappedIntersection(srcPos, pos, snapSzSquared, nd, rd, pos);
		}
		else
		{
			nd = snappedToNode;
			return 2;
		}
	}
	//LogManager::getSingleton().logMessage("Nadda SAl", LML_CRITICAL);
	return 0;
}

bool RoadGraph::getNodeClosest(const Ogre::Vector2 &loc, NodeId &nd, Ogre::Real &distance) const
{
	if(getNodeClosestSq(loc, nd, distance)) 
	{
		distance = Math::Sqrt(distance);
		return true;
	}
	else
		return false;
}

bool RoadGraph::getNodeClosestSq(const Ogre::Vector2 &loc, NodeId &nd, Ogre::Real &distance) const
{
	Ogre::Real currDist = std::numeric_limits<Ogre::Real>::max();
	NodeId currNode;
	NodeIterator nIt, nEnd;
	bool success = false;
	for(boost::tie(nIt, nEnd) = vertices(mGraph); nIt != nEnd; nIt++)
	{
		Vector2 temp(mGraph[*nIt]->getPosition2D() - loc);
		if(temp.squaredLength() < currDist) 
		{
			currDist = temp.squaredLength();
			currNode = *nIt;
			success = true;
		}
	}
	if(success) 
	{
		nd = currNode;
		distance = currDist;
	}
	return success;
}



bool RoadGraph::findClosestIntersection2(const std::vector<RoadId>& roadSegments, RoadId& rd, Vector2& intersection) const
{
	bool hasIntersection = false;
	Vector2 currentIntersection, closestIntersection;
	RoadId closestRoad;
	Ogre::Real currentDistance, closestDistance;

	for(unsigned int i=0; i<roadSegments.size(); i++)
	{
		Vector2 a = getSrcNode(roadSegments[i])->getPosition2D();
		Vector2 b = getDstNode(roadSegments[i])->getPosition2D();
	
		RoadIterator rIt, rEnd;
		for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
		{
			// don't check neighbouring segments
			if( ((i-1) < 0 || *rIt != roadSegments[i-1])
				&& ((i+1) >= roadSegments.size() || *rIt != roadSegments[i+1]) )
			{
				if(roadIntersection(a, b, *rIt, currentIntersection)
					&& currentIntersection != a)
				{
					if(hasIntersection)
					{
						currentDistance = ( (a - currentIntersection).squaredLength() );
						if(currentDistance < closestDistance)
						{
							closestIntersection = currentIntersection;
							closestRoad = *rIt;
						}
					}
					else
					{
						hasIntersection = true;
						closestDistance = ( (a - currentIntersection).squaredLength() );
						closestIntersection = currentIntersection;
						closestRoad = *rIt;
					}
				}
			}
		}

		if(hasIntersection)
		{
			intersection = closestIntersection;
			rd = closestRoad;
			break;
		}
	}
	return hasIntersection;	
}

bool RoadGraph::snapToRoadNode(const Vector2& pos, const RoadId& rd, const Real& snapSzSq, NodeId& nd) const
{
	// get distance between intersection and nodes
	Real srcDistSq = (getSrcNode(rd)->getPosition2D() - pos).squaredLength();
	Real dstDistSq = (getDstNode(rd)->getPosition2D() - pos).squaredLength();

	// check if intersection is with snapSz of the existing roads src node
	if(srcDistSq < snapSzSq)
	{
		if(srcDistSq <= dstDistSq)
		{
			nd = getSrc(rd);
			return true;
		}
		else
		{
			nd = getDst(rd);
			return true;
		}
	}
	// check if intersection is with snapSz of the existing roads dst node
	else if(dstDistSq < snapSzSq)
	{
		nd = getDst(rd);
		return true;
	}
	return false;
}

bool RoadGraph::snapToNode(const Vector2& pos, const Real& snapSzSq, NodeId& nd) const
{
	Ogre::Real closestDistSq = snapSzSq;
	bool success = false;
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = vertices(mGraph); nIt != nEnd; nIt++)
	{
		Vector2 nodePos(mGraph[*nIt]->getPosition2D());
		Real currDistSq =(nodePos - pos).squaredLength();
		if(currDistSq < closestDistSq) 
		{
			closestDistSq = currDistSq;
			nd = *nIt;
			success = true;
		}
	}
	return success;
}

bool RoadGraph::hasIntersection(const Vector2& a, const Vector2& b, Vector2& pos) const
{
	Vector2 currentIntersection;
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		if(roadIntersection(a, b, *rIt, currentIntersection))
		{
			pos = currentIntersection;
			return true;
		}
	}
	return false;
}

bool RoadGraph::hasIntersection(const RoadId rd)
{
	// prepare the ignore list used to avoid returning 
	//  intersections to properly connected roads
	std::set<RoadId, road_less_than> ignoreList;
	RoadIterator2 riIt, riEnd;

	// add roads connected to the src node
	tie(riIt, riEnd) = getRoadsFromNode(getSrc(rd));
	ignoreList.insert(riIt, riEnd);

	// add roads connected to the dst node
	tie(riIt, riEnd) = getRoadsFromNode(getDst(rd));
	ignoreList.insert(riIt, riEnd);

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		// if road is not in the ignore list
		std::set<RoadId, road_less_than>::iterator ignoreIt = ignoreList.find(*rIt);
		if(ignoreIt == ignoreList.end())
		{
			Vector2 intersection;
			if(roadIntersection(rd, *rIt, intersection))
				return true;
		}
	}
	return false;
}




















































bool RoadGraph::findClosestIntersection(NodeId srcNd, 
							 const Vector2 &srcPos,
							 const Vector2 &dstPos,
							 RoadId& rd, Vector2& pos) const
{
	bool hasIntersection = false;
	Ogre::Real currentDistance, closestDistance;
	Vector2 currentIntersection;
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		// exclude roads connected to the srcNode
		if(getSrc(*rIt) == srcNd || getDst(*rIt) == srcNd)
			continue;

		// test for an intersection
		if(Geometry::lineSegmentIntersect(srcPos, dstPos, getNode(getSrc(*rIt))->getPosition2D(), 
			getNode(getDst(*rIt))->getPosition2D(), currentIntersection))
		{
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



int RoadGraph::snapInfo(NodeId srcNd, const Vector2 &dstPos, Real snapSzSquared,
						NodeId& nd, RoadId& rd, Vector2& pos) const
{
	Vector2 srcPos = getNode(srcNd)->getPosition2D();

	NodeId snappedToNode;
	bool nodeSnapped = false;

	// Find Closest Intersection
	if(findClosestIntersection(srcNd, srcPos, dstPos, rd, pos))
	{
		// Intersection!
		// Snap to Road Node
		if(snapToRoadNode(pos, rd, snapSzSquared, snappedToNode))
		{
			nodeSnapped = true;
		}
		else
		{
			// INTERSECTION
			return 1;
		}
	}
	else
	{
		// No intersection!
		// Snap To Node
		nodeSnapped = snapToNode(dstPos, snapSzSquared, snappedToNode);
	}

	// if node snapped we alter the direction of our proposed road and must retest it
	while(nodeSnapped)
	{
		// Find Closest Intersection
		if(findClosestIntersection(srcNd, srcPos, getNode(snappedToNode)->getPosition2D(), rd, pos))
		{
			// Snap to Road Node
			NodeId tmp; 
			if(snapToRoadNode(pos, rd, snapSzSquared, tmp))
			{
				if(snappedToNode != tmp) {
					snappedToNode = tmp;
					continue;
				}
			}
			else
			{
				// INTERSECTION
				return 1;
			}
		}
		// NODE SNAP
		nd = snappedToNode;
		return 2;
	}

	// NO INTERSECTION
	return 0;
}

