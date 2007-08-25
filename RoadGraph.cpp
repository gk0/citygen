#include "stdafx.h"
#include "RoadGraph.h"
#include "NodeInterface.h"
#include "RoadInterface.h"
#include "Geometry.h"
#include "WorldNode.h"
#include "SimpleNode.h"

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
	tie(rd, inserted) = add_edge(nd1, nd2, (RoadInterface*)0, _graph);
	return inserted;
}

bool RoadGraph::addRoad(const NodeId nd1, const NodeId nd2, RoadInterface* r, RoadId& rd)
{
	bool inserted = false;
	tie(rd, inserted) = add_edge(nd1, nd2, r, _graph);
	if(inserted) 
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
	if(found) rd = r;
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
	if(found)
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
		LogManager::getSingleton().logMessage(e.getDescription());

		//DANGER - setting partially decomposed graph to real graph
		//mGraph = g;
	}
}

void RoadGraph::extractFilament(NodeId v0, NodeId v1, Graph &g, list<NodeId>& heap, 
								vector< vector<NodeInterface*> > &filaments)
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
		vector<NodeInterface*> filament;

		if(out_degree(v0, g) >= 3)
		{
			filament.push_back(g[v0]);
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
			filament.push_back(g[v0]);

			//heap.erase(v0);
			removeFromHeap(v0, heap);
			remove_edge(v0, v1, g);
			remove_vertex(v0, g);
			v0 = v1;
		}

		filament.push_back(g[v0]);

		if(out_degree(v0, g) == 0)
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
		if(out_degree(v1, g) == 1) //TODO: causes error sometimes
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
		
		NodeInterface* ni = g[vadj];
		Vector2 dadj = ni->getPosition2D()  - vcurr_pos;	//error void ptr
		
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


		Vector2 dadj = g[vadj]->getPosition2D() - vcurr_pos;	//error
		
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
	for(; pos != heap.end(); pos++)
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
	if(success) 
	{
		nd = currNode;
		distance = currDist;
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

Vector2 RoadGraph::getRoadBounaryIntersection(const RoadId leftR, const RoadId rightR)
{
	Real lWidth, rWidth;
	Vector2 l1, l2, r1, r2, lOffset, rOffset;

	l1 = getSrcNode(leftR)->getPosition2D();
	l2 = getDstNode(leftR)->getPosition2D();
	r1 = getSrcNode(rightR)->getPosition2D();
	r2 = getDstNode(rightR)->getPosition2D();

	lWidth = getRoad(leftR)->getWidth();
	rWidth = getRoad(rightR)->getWidth();

	lOffset = (l2 - l1).perpendicular();
	lOffset.normalise();
	lOffset *= lWidth;

	rOffset = (r2 - r1).perpendicular();
	rOffset.normalise();
	rOffset *= rWidth;

	l1 -= lOffset;
	l2 -= lOffset;
	r1 += rOffset;
	r2 += rOffset;

	// if parallel, use l1 as pos
	Vector2 pos;
	if(Geometry::lineIntersect(l1, l2, r1, r2, pos))
		return pos;
	else
		return l1;
}

bool RoadGraph::snapToNode(const Vector2& pos, const Real& snapSzSq, NodeId& nd) const
{
	Ogre::Real closestDistSq = snapSzSq;
	bool success = false;
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = vertices(_graph); nIt != nEnd; nIt++)
	{
		Vector2 nodePos(_graph[*nIt]->getPosition2D());
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

int RoadGraph::findClosestIntscnOrNode(const NodeId aNode, const Vector2& b, const Real snapSz, 
							  Vector2& pos, NodeId &nd, RoadId& rd) const
{
	// function vars
	bool cWithinSnap, dWithinSnap, nodeSnapped = false;
	Real bR, bS, cR, cS, dR, dS, lowestR=std::numeric_limits<Real>::max();
	Vector2 bestPoint;
	NodeId bestNode;
	RoadId intersectingRoad;

	// get vars for seg AB
	Vector2 a(getNode(aNode)->getPosition2D());
	Vector2 BminusA(b - a);
	Real Labsq = BminusA.squaredLength();
	Real Lab = Math::Sqrt(Labsq);
	Real snapSzSq = Math::Sqr(snapSz);

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		NodeId cNode(getSrc(*rIt)), dNode(getDst(*rIt));
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
						Real LcpSq((p-c).squaredLength()), LdpSq((p-d).squaredLength());
						if(LcpSq < snapSzSq && LcpSq < LdpSq)
						{
							bestPoint = c;
							bestNode = cNode;
							nodeSnapped = true;
						}
						else if(LdpSq < snapSzSq)
						{
							bestPoint = d;
							bestNode = dNode;
							nodeSnapped = true;
						}
						else
						{
							bestPoint = p;
							nodeSnapped = false;
						}
						intersectingRoad = (*rIt);
						lowestR = bR;
					}
				}
			}
		}

		// IMPORTANT: perform check for b against adjacent roads
		// so I don't place roads on top of each other, but don't
		// perform the other checks as they'll always find the 
		// connection intersection
		if(cNode != aNode)
		{
			//
			// point c to line segment ab
			cR = (CminusA.x*BminusA.x + CminusA.y*BminusA.y) / Labsq;
			cS = (-CminusA.y*BminusA.x+CminusA.x*BminusA.y) / Labsq;
			if(cR >= 0 && cR <= 1)
				cWithinSnap = (Math::Abs(cS) * Lab) < snapSz;
			else
				cWithinSnap = (c - b).squaredLength() < snapSzSq;
			
			if(cWithinSnap && cR < lowestR)
			{
				lowestR = cR;
				bestPoint = c;
				bestNode = cNode;
				intersectingRoad = (*rIt);
				nodeSnapped = true;
			}
		}

		if(dNode != aNode)
		{
			//
			// point d to line segment ab
			dR = (DminusA.x*BminusA.x + DminusA.y*BminusA.y) / Labsq;
			dS = (-DminusA.y*BminusA.x+DminusA.x*BminusA.y) / Labsq;
			if(dR >= 0 && dR <= 1)
				dWithinSnap = (Math::Abs(dS) * Lab) < snapSz;
			else
				dWithinSnap = (d - b).squaredLength() < snapSzSq;

			if(dWithinSnap && dR < lowestR)
			{
				lowestR = dR;
				bestPoint = d;
				bestNode = dNode;
				intersectingRoad = (*rIt);
				nodeSnapped = true;
			}
		}

		if(cNode == aNode || dNode == aNode) continue;

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
		if(denom == 0) continue;	// parallel		
		Real r = ((-CminusA.y * DminusC.x) + (CminusA.x * DminusC.y)) / denom;
		Real s = ((-CminusA.y * BminusA.x) + (CminusA.x * BminusA.y)) / denom;
		if(r == 0 && s == 0) continue; // coincident (on top of one another)
		if(s >= 0 && s <= 1 && r >= 0 && r < lowestR && r<=1)
		{
			Vector2 p(a.x + r * (BminusA.x), a.y + r * (BminusA.y));
			Real LcpSq((p-c).squaredLength()), LdpSq((p-d).squaredLength());
			if(LcpSq < snapSzSq && LcpSq < LdpSq)
			{
				bestPoint = c;
				bestNode = cNode;
				nodeSnapped = true;
			}
			else if((p-d).squaredLength() < snapSzSq)
			{
				bestPoint = d;
				bestNode = dNode;
				nodeSnapped = true;
			}
			else
			{
				bestPoint = p;
				intersectingRoad = (*rIt);
				nodeSnapped = false;
			}
			intersectingRoad = (*rIt);
			lowestR = r;
		}
	}

	// Write back results if success
	if(lowestR < std::numeric_limits<Real>::max())
	{
		pos = bestPoint;
		rd = intersectingRoad;
		if(nodeSnapped)
		{
			nd = bestNode;
			return 2;
		}
		else
			return 1;
	}
	return 0;
}


bool RoadGraph::findClosestIntersection(const std::vector<NodeId>& ignore, const Vector2& b, const Real snapSz, 
							  Vector2& pos, RoadId& rd) const
{
	// function vars
	Real bR, bS, cR, cS, dR, dS, lowestR=std::numeric_limits<Real>::max();
	Vector2 bestPoint;
	RoadId intersectingRoad;

	// get vars for seg AB
	Vector2 a(getNode(ignore[0])->getPosition2D());
	Vector2 BminusA(b - a);
	Real Labsq = BminusA.squaredLength();
	Real Lab = Math::Sqrt(Labsq);

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		NodeId cNode(getSrc(*rIt)), dNode(getDst(*rIt));
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
						intersectingRoad = (*rIt);
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
			intersectingRoad = (*rIt);
		}

		//
		// point d to line segment ab
		dR = (DminusA.x*BminusA.x + DminusA.y*BminusA.y) / Labsq;
		dS = (-DminusA.y*BminusA.x+DminusA.x*BminusA.y) / Labsq;
		if(dR >= 0 && dR < lowestR && dR <= 1 && (Math::Abs(dS) * Lab) < snapSz)
		{
			lowestR = dR;
			bestPoint = d;
			intersectingRoad = (*rIt);
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
		if(denom == 0) continue;	// parallel		
		Real r = ((-CminusA.y * DminusC.x) + (CminusA.x * DminusC.y)) / denom;
		Real s = ((-CminusA.y * BminusA.x) + (CminusA.x * BminusA.y)) / denom;
		if(r == 0 && s == 0) continue; // coincident (on top of one another)
		if(s >= 0 && s <= 1 && r >= 0 && r < lowestR && r<=1)
		{
			Vector2 p(a.x + r * (BminusA.x), a.y + r * (BminusA.y));
			bestPoint = p;
			intersectingRoad = (*rIt);
			lowestR = r;
		}
	}

	// Write back results if success
	if(lowestR < std::numeric_limits<Real>::max())
	{
		pos = bestPoint;
		rd = intersectingRoad;
		return true;
	}
	return false;
}



bool RoadGraph::findClosestRoad(const NodeId aNode, const Real snapSz, 
							  Vector2& pos, RoadId& rd) const
{
	// function vars
	Real currentDistance, closestDistanceSq=std::numeric_limits<Real>::max();
	Vector2 bestPoint, a(getNode(aNode)->getPosition2D());
	RoadId intersectingRoad;
	Real snapSzSq = Math::Sqr(snapSz);

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
	{
		NodeId cNode(getSrc(*rIt)), dNode(getDst(*rIt));
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
					intersectingRoad = (*rIt);
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
					intersectingRoad = (*rIt);
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
					intersectingRoad = (*rIt);
				}
			}
		}
	}
	// Write back results if success
	if(closestDistanceSq < std::numeric_limits<Real>::max())
	{
		pos = bestPoint;
		rd = intersectingRoad;
		return true;
	}
	return false;
}

bool RoadGraph::findClosestIntscnConnected(const NodeId aNode, const NodeId bNode, const Ogre::Real snapSz, 
	Ogre::Vector2& pos, RoadId& rd) const
{
	std::vector<NodeId> ignore(2);
	ignore[0] = aNode;
	ignore[1] = bNode;
	Ogre::Vector2 bPos = getNode(bNode)->getPosition2D();
	return findClosestIntersection(ignore, bPos, snapSz, pos, rd);
}

int RoadGraph::snapInfo(const NodeId srcNd, const Vector2& dstPos, const Real snapSzSquared,
						NodeId& nd, RoadId& rd, Vector2& pos) const
{
	//Vector2 srcPos = getNode(srcNd)->getPosition2D();
	//Vector2 dstPos = getNode(dstNd)->getPosition2D();
	int state = 0;

	//while(true)
	//{
		state = findClosestIntscnOrNode(srcNd, dstPos, Math::Sqrt(snapSzSquared), pos, nd, rd);
	//	
	//	if(state < 2) break;
	//	else if()

	///	{
	//	}
	//}
	return state;
}

struct NodeInfo {
	const NodeId _id;
	const Vector2 _pos;
	NodeInfo(const NodeId id, const Vector2& pos)
			: _id(id), _pos(pos) {}
};
bool operator<(const NodeInfo &l, const NodeInfo &r)
{  return l._pos.x < r._pos.x;  }


void RoadGraph::extractFootprints(std::vector< std::vector<Ogre::Vector2> > &polys, Real num, ManualObject* dob)
{
	// create a copy of our road graph to work on
	Graph g(_graph);

	//init roads
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = edges(g); rIt != rEnd; rIt++)
		g[*rIt]->setRoadCycle(false);

	set<NodeInfo> heap2;
	BOOST_FOREACH(NodeId nd, vertices(g))
		heap2.insert(NodeInfo(nd, g[nd]->getPosition2D()));


	//DEBUG:
	//ostringstream oss;
	//BOOST_FOREACH(NodeInfo &n, heap2) oss << n._id << ": " << n._pos << "\n";
	//LogManager::getSingleton().logMessage(oss.str());

	std::list<NodeId> heap;
	BOOST_FOREACH(NodeInfo n, heap2) heap.insert(heap.end(), n._id);

	try {

	//while (heap is not empty) do
	size_t i;
	for(i=0; i<1000 && heap.size() != 0; i++)
	{
		//if(i<num)
		if(i>=10)
			int z=0;

		//Vertex v0 = heap.GetMin();
		NodeId v0 = *(heap.begin());

		switch(out_degree(v0, g))
		{
		case 0:
			remove_vertex(v0, g);
			removeFromHeap(v0, heap);
			break;
		case 1:
			extractFilamentF(v0, getFirstAdjacent(v0, g), g, heap);
			//oss<<"Filament: "<<graph[v0].getName()<<endl;

			//DEBUG
			//heap.erase(v0);
			break;
		default:
			 extractPrimitiveF(v0, g, heap, polys); // filament or minimal cycle
			//oss<<"Cycle or Filament: "<<mGraph[v0].getName()<<endl;

			//DEBUG
			//heap.erase(v0);
			break;
		}
		//heap.erase(v0);
	}
	
	if(i>=1000)
		LogManager::getSingleton().logMessage("Primitive Infinitum");

	}
	catch(Exception e)
	{
		LogManager::getSingleton().logMessage(e.getDescription());
	}

	if(dob==0) return;
	//RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = edges(g); rIt != rEnd; rIt++)
	{
		dob->position(g[source(*rIt, g)]->getPosition3D() + Vector3(0,0.5,0));
		dob->position(g[target(*rIt, g)]->getPosition3D() + Vector3(0,0.5,0));
	}

}

void RoadGraph::extractPrimitiveF(NodeId v0, Graph &g, list<NodeId>& heap, vector< vector<Vector2> > &polys)
{
	set<NodeId> visited;
	vector<NodeId> nodeCycle;
	vector<Vector2> poly;

	NodeId v1;
	bool vertexFound = getClockwiseMost(v0, v1, g);

	NodeId vprev = v0;
	NodeId vcurr = v1;
	NodeId vnext;

	//while (vcurr is not nil) and (vcurr is not v0) and (vcurr is not visited) do
	size_t i;
	for(i=0; i<1000; i++)
	{
		nodeCycle.push_back(vcurr);
		vertexFound = getCounterClockwiseMostFromPrev(vprev, vcurr, vnext, g);	// error

		// lets add a point here
		if(vertexFound)
		{
			// BAD KITTY: can't go backwards, esp at start, 
			// am guessin v0-v1 should always be on a boundary of sorts
			// so no start on a filament
			if(vcurr == v1 && vnext == v0) 
			{
				// bad kitty
				removeFromHeap(v0, heap);
				break;
			}

			poly.push_back(getSuperIntscn(vprev, vcurr, vnext, g));
			if(visited.find(vcurr) != visited.end()) 
			{
				//snip vprev --> vcurr
				remove_edge(vprev, vcurr, g);
			}
			else
				g[edge(vprev, vcurr, g).first]->setRoadCycle(true);

			if(vcurr == v0 && vnext == v1) 
			{
				polys.push_back(poly);
				break;
			}

			visited.insert(vcurr);
			vprev = vcurr;
			vcurr = vnext;
		}
		else
		{
			addTerminalPoints(vprev, vcurr, g, poly);
			visited.insert(vcurr);
			swap(vprev, vcurr);

			//extractFilamentF(vcurr, getFirstAdjacent(vcurr, g), g, heap, poly);
		}
	}
	if(i>=1000)
		LogManager::getSingleton().logMessage("Infinitum");
	else
	{
		// trash this bad monkey
		
		// snip it
		remove_edge(v0, v1, g);

		if(out_degree(v0, g) == 1)
		{
			// Remove the filament rooted at v0.
			extractFilamentF(v0,  getFirstAdjacent(v0, g), g, heap);
		}
		if(out_degree(v1, g) == 1) //TODO: causes error sometimes
		{
			// Remove the filament rooted at v1.
			extractFilamentF(v1,  getFirstAdjacent(v1, g), g, heap);		// error
		}
	}
}

void RoadGraph::extractFilamentF(NodeId v0, NodeId v1, Graph &g, list<NodeId>& heap)
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
				remove_edge(v0, v1, g);		//error
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
			remove_edge(v0, v1, g);
			v0 = v1;
			if(out_degree(v0, g) == 1)
			{
				v1 =  getFirstAdjacent(v0, g);
			}
		}

		while(out_degree(v0, g) == 1)
		{
			v1 =  getFirstAdjacent(v0, g);
			
			//heap.erase(v0)
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

Ogre::Vector2 RoadGraph::getSuperIntscn(NodeId na, NodeId nb, NodeId nc, Graph& g)
{
	Vector2 a(g[na]->getPosition2D()), b(g[nb]->getPosition2D()), c(g[nc]->getPosition2D());
	Vector2 abPerp = (b - a).perpendicular();
	Vector2 bcPerp = (c - b).perpendicular();
	abPerp.normalise();
	bcPerp.normalise();
	abPerp *= g[edge(na, nb, g).first]->getWidth();
	bcPerp *= g[edge(nb, nc, g).first]->getWidth();

	Real r,s;
	Vector2 inscn;
	if(Geometry::lineIntersect(a+abPerp, b+abPerp, b+bcPerp, c+bcPerp, inscn, r, s) && r >= 0 && s <= 1)
	{
		return inscn;
	}
	else
	{
		return (b+abPerp+b+bcPerp)/2;
	}
}

void RoadGraph::addTerminalPoints(NodeId na, NodeId nb, Graph& g, vector<Vector2> &poly)
{
	Vector2 a(g[na]->getPosition2D()), b(g[nb]->getPosition2D());
	Vector2 abPerp = (b - a).perpendicular();
	abPerp.normalise();
	abPerp *= g[edge(na, nb, g).first]->getWidth();
	poly.push_back(b+abPerp);
	poly.push_back(b-abPerp);
}


void RoadGraph::extractFootprints(std::vector< std::vector<Ogre::Vector3> > &polys, Real num, ManualObject* dob)
{
	// create a copy of our road graph to work on
	Graph g(_graph);

	//init roads
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = edges(g); rIt != rEnd; rIt++)
		g[*rIt]->setRoadCycle(false);

	set<NodeInfo> heap2;
	BOOST_FOREACH(NodeId nd, vertices(g))
		heap2.insert(NodeInfo(nd, g[nd]->getPosition2D()));


	//DEBUG:
	//ostringstream oss;
	//BOOST_FOREACH(NodeInfo &n, heap2) oss << n._id << ": " << n._pos << "\n";
	//LogManager::getSingleton().logMessage(oss.str());

	std::list<NodeId> heap;
	BOOST_FOREACH(NodeInfo n, heap2) heap.insert(heap.end(), n._id);

	try {

		//while (heap is not empty) do
		size_t i;
		for(i=0; i<1000 && heap.size() != 0; i++)
		{
			//if(i<num)
			if(i>=10)
				int z=0;

			//Vertex v0 = heap.GetMin();
			NodeId v0 = *(heap.begin());

			switch(out_degree(v0, g))
			{
			case 0:
				remove_vertex(v0, g);
				removeFromHeap(v0, heap);
				break;
			case 1:
				extractFilamentF(v0, getFirstAdjacent(v0, g), g, heap);
				//oss<<"Filament: "<<graph[v0].getName()<<endl;

				//DEBUG
				//heap.erase(v0);
				break;
			default:
				extractPrimitiveF(v0, g, heap, polys); // filament or minimal cycle
				//oss<<"Cycle or Filament: "<<mGraph[v0].getName()<<endl;

				//DEBUG
				//heap.erase(v0);
				break;
			}
			//heap.erase(v0);
		}

		if(i>=1000)
			LogManager::getSingleton().logMessage("Primitive Infinitum");

	}
	catch(Exception e)
	{
		LogManager::getSingleton().logMessage(e.getDescription());
	}

	if(dob==0) return;
	//RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = edges(g); rIt != rEnd; rIt++)
	{
		dob->position(g[source(*rIt, g)]->getPosition3D() + Vector3(0,0.5,0));
		dob->position(g[target(*rIt, g)]->getPosition3D() + Vector3(0,0.5,0));
	}

}

void RoadGraph::extractPrimitiveF(NodeId v0, Graph &g, list<NodeId>& heap, vector< vector<Vector3> > &polys)
{
	set<NodeId> visited;
	vector<NodeId> nodeCycle;
	vector<Vector3> poly;

	NodeId v1;
	bool vertexFound = getClockwiseMost(v0, v1, g);

	NodeId vprev = v0;
	NodeId vcurr = v1;
	NodeId vnext;

	//while (vcurr is not nil) and (vcurr is not v0) and (vcurr is not visited) do
	size_t i;
	for(i=0; i<1000; i++)
	{
		nodeCycle.push_back(vcurr);
		vertexFound = getCounterClockwiseMostFromPrev(vprev, vcurr, vnext, g);	// error

		// lets add a point here
		if(vertexFound)
		{
			// BAD KITTY: can't go backwards, esp at start, 
			// am guessin v0-v1 should always be on a boundary of sorts
			// so no start on a filament
			if(vcurr == v1 && vnext == v0) 
			{
				// bad kitty
				removeFromHeap(v0, heap);
				break;
			}

			poly.push_back(getSuperIntscn2(vprev, vcurr, vnext, g));
			if(visited.find(vcurr) != visited.end()) 
			{
				//snip vprev --> vcurr
				remove_edge(vprev, vcurr, g);
			}
			else
				g[edge(vprev, vcurr, g).first]->setRoadCycle(true);

			if(vcurr == v0 && vnext == v1) 
			{
				polys.push_back(poly);
				break;
			}

			visited.insert(vcurr);
			vprev = vcurr;
			vcurr = vnext;
		}
		else
		{
			addTerminalPoints(vprev, vcurr, g, poly);
			visited.insert(vcurr);
			swap(vprev, vcurr);

			//extractFilamentF(vcurr, getFirstAdjacent(vcurr, g), g, heap, poly);
		}
	}
	if(i>=1000)
		LogManager::getSingleton().logMessage("Infinitum");
	else
	{
		// trash this bad monkey

		// snip it
		remove_edge(v0, v1, g);

		if(out_degree(v0, g) == 1)
		{
			// Remove the filament rooted at v0.
			extractFilamentF(v0,  getFirstAdjacent(v0, g), g, heap);
		}
		if(out_degree(v1, g) == 1) //TODO: causes error sometimes
		{
			// Remove the filament rooted at v1.
			extractFilamentF(v1,  getFirstAdjacent(v1, g), g, heap);		// error
		}
	}
}

void RoadGraph::addTerminalPoints(NodeId na, NodeId nb, Graph& g, vector<Vector3> &poly)
{
	Vector2 a(g[na]->getPosition2D()), b(g[nb]->getPosition2D());
	Vector2 abPerp = (b - a).perpendicular();
	abPerp.normalise();
	abPerp *= g[edge(na, nb, g).first]->getWidth();
	
	poly.push_back(Vector3(b.x+abPerp.x, g[nb]->getPosition3D().y, b.y+abPerp.y));
	poly.push_back(Vector3(b.x-abPerp.x, g[nb]->getPosition3D().y, b.y-abPerp.y));
}

Ogre::Vector3 RoadGraph::getSuperIntscn2(NodeId na, NodeId nb, NodeId nc, Graph& g)
{
	Vector2 a(g[na]->getPosition2D()), b(g[nb]->getPosition2D()), c(g[nc]->getPosition2D());
	Vector2 abPerp = (b - a).perpendicular();
	Vector2 bcPerp = (c - b).perpendicular();
	abPerp.normalise();
	bcPerp.normalise();
	abPerp *= g[edge(na, nb, g).first]->getWidth();
	bcPerp *= g[edge(nb, nc, g).first]->getWidth();

	Real r,s;
	Vector2 inscn;
	if(Geometry::lineIntersect(a+abPerp, b+abPerp, b+bcPerp, c+bcPerp, inscn, r, s) && r >= 0 && s <= 1)
	{
		return Vector3(inscn.x, g[nb]->getPosition3D().y, inscn.y);
	}
	else
	{
		Vector2 tmp((b+abPerp+b+bcPerp)/2);
		return Vector3(tmp.x, g[nb]->getPosition3D().y, tmp.y);
	}
}

