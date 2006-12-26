#include "stdafx.h"
#include "RoadGraph.h"

using namespace boost;

RoadGraph::RoadGraph()
{
}

NodeDescriptor RoadGraph::createNode(const Vector2& pos)
{
	RoadNode rn;
	rn.mPosition = pos;
	rn.mSelected = false;
	return add_vertex(rn, mGraph);
}

NodeDescriptor RoadGraph::createNode(const Vector2& pos, void* ptrData)
{
	RoadNode rn;
	rn.mPosition = pos;
	rn.mSelected = false;
	rn.mPtrData1 = ptrData;
	return add_vertex(rn, mGraph);
}

void RoadGraph::moveNode(const NodeDescriptor nd, const Vector2& pos)
{
	mGraph[nd].mPosition = pos;
}

void RoadGraph::removeNode(const NodeDescriptor nd)
{
	remove_vertex(nd, mGraph);
}

RoadDescriptor RoadGraph::createRoad(const NodeDescriptor nd1, const NodeDescriptor nd2, void* ptrData)
{
	RoadDescriptor e;
	bool inserted = false;
	tie(e, inserted) = add_edge(nd1, nd2, mGraph);
	if(inserted)
		mGraph[e].mPtrData1 = ptrData;
	return e;
}

RoadDescriptor RoadGraph::createRoad(const NodeDescriptor nd1, const NodeDescriptor nd2)
{
	RoadDescriptor rd;
	bool inserted = false;
	tie(rd, inserted) = add_edge(nd1, nd2, mGraph);
	return rd;
}

bool RoadGraph::findRoad(const NodeDescriptor nd1, const NodeDescriptor nd2, RoadDescriptor& rd)
{
	RoadDescriptor r;
	bool found = false;
	tie(r, found) = edge(nd1, nd2, mGraph);
	if(found) rd = r;
	return found;
}

void RoadGraph::removeRoad(const NodeDescriptor nd1, const NodeDescriptor nd2)
{
	remove_edge(nd1, nd2, mGraph);
}

void RoadGraph::loadXML(const TiXmlHandle& root)
{
	// clear current document
	mGraph.clear();

	// a translation map is used to find the nodes for edge creation
	std::map<std::string, NodeDescriptor> nodeIdTranslation;

	// an intermediate structure is used for edges since there 
	// is no guarantee the nodes have been loaded first
	std::vector< std::pair<std::string, std::string> > edgeData;

	// TODO:
	// check graph id
	// check undirected

	// scan xml
	TiXmlElement* pElem=root.FirstChild("graph").FirstChild().Element();
	for(; pElem; pElem=pElem->NextSiblingElement())
	{
		std::string key = pElem->Value();
		if(key == "node") 
		{
			Vector2 loc;
			std::string strId = pElem->Attribute("id");
			pElem->QueryFloatAttribute("x", &loc.x);
			pElem->QueryFloatAttribute("y", &loc.y);

			NodeDescriptor nd = createNode(loc);
			nodeIdTranslation.insert(std::make_pair(strId, nd));
		}
		else if(key == "edge") 
		{
			edgeData.push_back(std::make_pair(pElem->Attribute("source"), pElem->Attribute("target")));
		}
	}

	// create the edges now that all graph data has been read
	for(unsigned int i=0; i<edgeData.size(); i++)
	{
		std::map<std::string, NodeDescriptor>::iterator sourceIt, targetIt;
		sourceIt = nodeIdTranslation.find(edgeData[i].first);
		targetIt = nodeIdTranslation.find(edgeData[i].second);
		// if source node and target node can be found
		if(sourceIt != nodeIdTranslation.end() && targetIt != nodeIdTranslation.end()) 
		{
			bool inserted;
			graph_traits<Graph>::edge_descriptor e;
			tie(e, inserted) = add_edge(sourceIt->second, targetIt->second, mGraph);
		}
	}
}

void RoadGraph::setNodeData1(const NodeDescriptor nd, void* ptrData)
{
	mGraph[nd].mPtrData1 = ptrData;
}

void RoadGraph::setRoadData1(const RoadDescriptor rd, void* ptrData)
{
	mGraph[rd].mPtrData1 = ptrData;
}

/*
 * GraphML
 * http://graphml.graphdrawing.org/primer/graphml-primer.html
 *
 */
TiXmlElement* RoadGraph::saveXML()
{
	//<graph id="roadgraph" edgedefault="undirected">
	TiXmlElement *roadNetwork = new TiXmlElement("graph"); 
	roadNetwork->SetAttribute("id", "roadgraph");
	roadNetwork->SetAttribute("edgedefault", "undirected");
	//root->LinkEndChild( roadNetwork );

	graph_traits<Graph>::vertex_iterator vi, vend;
	for (tie(vi, vend) = vertices(mGraph); vi != vend; ++vi) 
	{
		Vector2& loc(mGraph[*vi].mPosition);
		//std::string id(pointerToString(*vi));
		//std::string id
	
		TiXmlElement * node;
		node = new TiXmlElement( "node" );  
		node->SetAttribute("id", (int) *vi);
		node->SetDoubleAttribute("x", loc.x);
		node->SetDoubleAttribute("y", loc.y);
		roadNetwork->LinkEndChild( node );
	}

	//Edges
	graph_traits<Graph>::edge_iterator i, end;
	for (tie(i, end) = edges(mGraph); i != end; ++i)
	{
		TiXmlElement * edge;
		edge = new TiXmlElement( "edge" );  
		edge->SetAttribute("source", nodeDescriptorToString(source(*i, mGraph)));
		edge->SetAttribute("target", nodeDescriptorToString(target(*i, mGraph)));
		roadNetwork->LinkEndChild( edge );  
	}
	return roadNetwork;
}

std::string RoadGraph::nodeDescriptorToString(NodeDescriptor nd) 
{		
	std::stringstream oss;
	oss << nd;
	return oss.str();
}



bool RoadGraph::getClockwiseMost(NodeDescriptor vcurr, NodeDescriptor& vnext) 
{
	bool isConvex = false, success = false;

	Vector2 dnext;
	Vector2 dcurr = Vector2(0, -1);
	Vector2 vcurr_pos(mGraph[vcurr].mPosition);

	//for each adjacent vertex vadj of vcurr do
	graph_traits<Graph>::out_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = out_edges(vcurr, mGraph); ei != ei_end; ++ei)
	{
		//Exclude edges marked black to indicated deletion
		if(mGraph[*ei].color == black_color) continue;

		NodeDescriptor vadj = target(*ei, mGraph);
		Vector2 dadj = mGraph[vadj].mPosition - vcurr_pos;
		
		if(!success)
		{
			vnext = vadj;
			dnext = dadj;
			isConvex = (dotPerp(dnext, dcurr) <= 0);
			success = true;
			continue;
		}

		if(isConvex)
		{
			if (dotPerp(dcurr, dadj) < 0 || dotPerp(dnext, dadj) < 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (dotPerp(dnext, dcurr) <= 0);
			}
		}
		else
		{
			if (dotPerp(dcurr, dadj) < 0 && dotPerp(dnext, dadj) < 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (dotPerp(dnext, dcurr) <= 0);
			}
		}
	}
	return success;
}


bool RoadGraph::getCounterClockwiseMostFromPrev(NodeDescriptor vprev, NodeDescriptor vcurr, NodeDescriptor& vnext) 
{
	bool isConvex = false, success = false;

	Vector2 vcurr_pos(mGraph[vcurr].mPosition);
	Vector2 vprev_pos(mGraph[vprev].mPosition);
	Vector2 dcurr = vcurr_pos - vprev_pos;
	Vector2 dnext;

	//for each adjacent vertex vadj of vcurr do
	graph_traits<Graph>::out_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = out_edges(vcurr, mGraph); ei != ei_end; ++ei)
	{
		//Exclude edges marked black to indicated deletion
		if(mGraph[*ei].color == black_color) continue;

		//No going back :)
		if(target(*ei, mGraph) == vprev) continue;

		NodeDescriptor vadj = target(*ei, mGraph);
		Vector2 dadj = mGraph[vadj].mPosition - vcurr_pos;
		
		if(!success)
		{
			vnext = vadj;
			dnext = dadj;
			isConvex = (dotPerp(dnext, dcurr) <= 0);
			success = true;
			continue;
		}

		if(isConvex)
		{
			if (dotPerp(dcurr, dadj) > 0 && dotPerp(dnext, dadj) > 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (dotPerp(dnext, dcurr) <= 0);
			}
		}
		else
		{
			if (dotPerp(dcurr, dadj) > 0 || dotPerp(dnext, dadj) > 0)
			{
				vnext = vadj;
				dnext = dadj;
				isConvex = (dotPerp(dnext, dcurr) <= 0);
			}
		}
	}
	return success;
}


bool RoadGraph::sortVertex(const NodeDescriptor& v0, const NodeDescriptor& v1)
{
	Real x0 = mGraph[v0].mPosition.x;
	Real x1 = mGraph[v1].mPosition.x;
	return (x0 < x1);
}

void RoadGraph::ExtractPrimitives(PrimitiveVec& primitives)
{
	//should return this
	//mCells.clear();
	//primitives.clear();

	std::ostringstream oss; 
	oss << " ";

	//Init Graph Colors
	graph_traits<Graph>::edge_iterator ei, eend;
	for (tie(ei, eend) = edges(mGraph); ei != eend; ++ei) 
	{
		mGraph[*ei].color = white_color;
	}

	graph_traits<Graph>::vertex_iterator vi, vend;
	for (tie(vi, vend) = vertices(mGraph); vi != vend; ++vi) 
	{
		mGraph[*vi].color = white_color;
	}

	//set<NodeDescriptor> heap = vertices; 
	//gk: at the moment I'm using a vertex to store vertices so they are already sorted
	graph_traits<Graph>::vertex_iterator i, end;
	tie(i, end) = vertices(mGraph);
	//set<NodeDescriptor> heap(i, end);
	
	//SET needs sort
	//sort(heap.begin(), heap.end(), comp());
	//it would be neater o use a comparator but our comparision isn't standalone

	std::list< NodeDescriptor > heap;

	//insert one
	heap.push_back(*i++);

	for(; i != end; ++i)
	{
		for(std::list<NodeDescriptor>::iterator tit = heap.begin(); true; tit++)
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

	// heap sorted lets print the node ids alongside their xloc
	for(std::list<NodeDescriptor>::iterator heapIt = heap.begin(); heapIt != heap.end(); heapIt++)
	{
		oss << "(" << *heapIt << "," << mGraph[*heapIt].mPosition.x << ") ";
	}	
	
	//while (heap is not empty) do
	while(heap.size() != 0)
	{
		//Vertex v0 = heap.GetMin();
		NodeDescriptor v0 = *(heap.begin());

		unsigned int degree = getNonBlackDegree(v0);
		switch(degree)
		{
		case 0:
			ExtractIsolatedVertex(v0, heap, primitives);
			//oss<<"Isolated: "<<graph[v0].getName()<<endl;
			break;
		case 1:
			ExtractFilament(v0, getNonBlackAdjacent(v0), heap, primitives);
			//oss<<"Filament: "<<graph[v0].getName()<<endl;

			//DEBUG
			//heap.erase(v0);
			break;
		default:
			ExtractPrimitive(v0,heap, primitives); // filament or minimal cycle
			//oss<<"Cycle or Filament: "<<mGraph[v0].getName()<<endl;

			//DEBUG
			//heap.erase(v0);
			break;
		}
		//heap.erase(v0);
	}
	
	for(unsigned int i=0; i < primitives.size(); i++)
	{
		const Primitive& primitive(primitives[i]); 
		oss << "Primitive "<< primitive.type << ": ";
		Primitive::const_iterator pit;
		for(pit = primitive.begin(); pit != primitive.end(); pit++) 
		{
			oss << *pit << ",";
		}
		oss<<std::endl;
	}
	LogManager::getSingleton().logMessage(oss.str());
} 

void RoadGraph::removeFromHeap(NodeDescriptor v0, std::list<NodeDescriptor>& heap)
{
	std::list<NodeDescriptor>::iterator pos = heap.begin();
	for( ; pos != heap.end(); pos++)
	{
		if(*pos == v0) {
			heap.erase(pos);
			break;
		}
	}
}

void RoadGraph::ExtractIsolatedVertex(NodeDescriptor v0, std::list<NodeDescriptor>& heap, PrimitiveVec& primitives)
{
	Primitive primitive(ISOLATED_VERTEX);
	primitive.push_back(v0);
	//heap.erase(v0);
	removeFromHeap(v0, heap);
	//remove_vertex(v0, graph);
	mGraph[v0].color = black_color;
	primitives.push_back(primitive);
}

unsigned int RoadGraph::getNonBlackDegree(const NodeDescriptor v) 
{
	unsigned int degree = 0;
	graph_traits<Graph>::out_edge_iterator i, end;
	for(tie(i,end) = out_edges(v, mGraph); i != end; ++i)
	{
		if(mGraph[*i].color != black_color) {
			//&& mGraph[target(*i, mGraph)].color == black_color){
			degree++;
		}
	}
	return degree;
}

NodeDescriptor RoadGraph::getNonBlackAdjacent(const NodeDescriptor v)
{
	graph_traits<Graph>::out_edge_iterator i, end;
	for(tie(i,end) = out_edges(v, mGraph); i != end; ++i)
	{
		if(mGraph[*i].color != black_color) 
			//&& mGraph[target(*i, mGraph)].color == black_color){ 
			return target(*i, mGraph);
	}
	return (NodeDescriptor) 0;
}

void RoadGraph::setVertexBlack(const NodeDescriptor v)
{
	mGraph[v].color = black_color; //not needed really
	graph_traits<Graph>::out_edge_iterator i, end;
	for(tie(i,end) = out_edges(v, mGraph); i != end; ++i)
	{
		mGraph[*i].color = black_color;
	}
}

bool RoadGraph::setEdgeColour(NodeDescriptor v0, NodeDescriptor v1, default_color_type c)
{
	RoadDescriptor rd;
	bool success;
	tie(rd,success) = edge(v0, v1, mGraph);
	if(success) {
		mGraph[rd].color = black_color;
	}
	return success;
}

void RoadGraph::ExtractFilament(NodeDescriptor v0, NodeDescriptor v1, std::list<NodeDescriptor>& heap, PrimitiveVec& primitives)
{
	if (IsCycleEdge(v0, v1))
	{
		if(getNonBlackDegree(v0) >= 3)		
		{
			//remove_edge(v0, v1, mGraph);
			setEdgeColour(v0, v1, black_color);
			v0 = v1;
			if (getNonBlackDegree(v0) == 1)
			{
				//v1 = v0.Adjacent[0];
				v1 = getNonBlackAdjacent(v0);
			}

		}
		while(getNonBlackDegree(v0) == 1)
		{
			//v1 = v0.adjacent[0];
			v1 = getNonBlackAdjacent(v0);

			//pkV1 = pkV0->Adjacent[0];
			if (IsCycleEdge(v0, v1))
			{
				//heap.erase(v0);
				removeFromHeap(v0, heap);
				//remove_edge(v0, v1, mGraph);
				setEdgeColour(v0, v1, black_color);
				//remove_vertex(v0, mGraph);
				setVertexBlack(v0);
				v0 = v1;
			}
			else
			{
				break;
			}
		}
		if (getNonBlackDegree(v0) == 0)
		{
			//heap.erase(v0);
			removeFromHeap(v0, heap);
			//remove_vertex(v0, mGraph);
			setVertexBlack(v0);
		}
	}
	else
	{
		Primitive primitive(FILAMENT);
		if (getNonBlackDegree(v0) >= 3)
		{
			primitive.push_back(v0);
			//remove_edge(v0, v1, mGraph);
			setEdgeColour(v0, v1, black_color);
			v0 = v1;
			if (getNonBlackDegree(v0) == 1) {
				//v1 = v0.Adjacent[0];
				v1 = getNonBlackAdjacent(v0);
			}
		}

		while (getNonBlackDegree(v0) == 1)
		{
			primitive.push_back(v0);
			//v1 = v0.Adjacent[0];
			v1 = getNonBlackAdjacent(v0);

			//heap.erase(v0);
			removeFromHeap(v0, heap);
			//remove_edge(v0, v1, mGraph);
			setEdgeColour(v0, v1, black_color);
			//remove_vertex(v0, mGraph);
			setVertexBlack(v0);
			v0 = v1;
		}
		primitive.push_back(v0);

		if (getNonBlackDegree(v0) == 0)
		{
			//heap.erase(v0);
			removeFromHeap(v0, heap);
			//remove_vertex(v0, mGraph);
			setVertexBlack(v0);
		}
		primitives.push_back(primitive);
	}
}

bool RoadGraph::IsCycleEdge(NodeDescriptor v0, NodeDescriptor v1)
{
	RoadDescriptor rd;
	bool b;
	tie(rd,b) = edge(v0, v1, mGraph);
	return (mGraph[rd].color == gray_color);
}

void RoadGraph::ExtractPrimitive (NodeDescriptor v0, std::list<NodeDescriptor>& heap, PrimitiveVec& primitives)
{
	std::set<NodeDescriptor> visited;
	std::list<NodeDescriptor> sequence;
	sequence.push_back(v0);
	NodeDescriptor v1;
	getClockwiseMost(v0, v1);
	NodeDescriptor vprev = v0;
	NodeDescriptor vcurr = v1;
	NodeDescriptor vnext;

	bool vertexFound = true;

	//while (vcurr is not nil) and (vcurr is not v0) and (vcurr is not visited) do
	while(vertexFound && vcurr != v0)
	{
		std::set<NodeDescriptor>::iterator vi = visited.find(vcurr);
		if(vi != visited.end()) break;
		sequence.push_back(vcurr);
		visited.insert(vcurr);
		vertexFound = getCounterClockwiseMostFromPrev(vprev, vcurr, vnext);
		vprev = vcurr;
		vcurr = vnext;
	}
	if (!vertexFound)
	{
		// Filament found, not necessarily rooted at v0.
		ExtractFilament(v0, getNonBlackAdjacent(v0),heap, primitives);

		//Guessing madness - no i don't understand
		//setEdgeColour(v0, getNonBlackAdjacent(v0), black_color);
		//heap.erase(v0);
		//if(getNonBlackDegree(v0) == 2 && IsCycleEdge(v0,getNonBlackAdjacent(v0))) setEdgeColour(v0, getNonBlackAdjacent(v0), black_color);
		if(getNonBlackDegree(v0) == 2) setEdgeColour(v0, getNonBlackAdjacent(v0), black_color);
		//removeFromHeap(v0, heap);
	}
	else if (vcurr == v0)
	{
		// Minimal cycle found.
		Primitive primitive(MINIMAL_CYCLE);
		primitive.swap(sequence);
		primitives.push_back(primitive);
		//CityCell<NodeDescriptor> cell(primitive);
		//mCells.push_back(cell);

		//for each edge e in sequence do
		RoadDescriptor rd;
		bool success;
		Primitive::const_iterator pit = primitive.begin();
		Primitive::const_iterator pend = primitive.end();
		Primitive::const_iterator pprev = pit++;
		while(pit != pend)
		{
			tie(rd, success) = edge(*pprev, *pit, mGraph);
			if(success) {
				//EXPERIMENTAL / NONSENSE
				//if(mGraph[e].color == gray_color) mGraph[e].color = black_color;
				//else 
				if(mGraph[rd].color != black_color) mGraph[rd].color = gray_color;
			}
			pprev = pit++;
		}
		tie(rd, success) = edge(*pprev, *(primitive.begin()), mGraph);
		if(success) {
			//EXPERIMENTAL / NONSENSE
			//if(mGraph[e].color == gray_color) mGraph[e].color = black_color;
			//else
			if(mGraph[rd].color != black_color) mGraph[rd].color = gray_color;
		}
		//edges.remove(v0,v1);
		//if(getNonBlackDegree(v0) == 1 && getNonBlackDegree(v1) == 1) 
		{
			setEdgeColour(v0, v1, black_color);
		}

		if (getNonBlackDegree(v0) == 1)
		{
			// Remove the filament rooted at v0.
			ExtractFilament(v0, getNonBlackAdjacent(v0), heap, primitives);
		}
		if (getNonBlackDegree(v1) == 1)
		{
			// Remove the filament rooted at v1.
			ExtractFilament(v1, getNonBlackAdjacent(v1), heap, primitives);
		}
	}
	else // vcurr was visited earlier
	{
		// A cycle has been found, but is not guaranteed to be a minimal
		// cycle. This implies v0 is part of a filament. Locate the
		// starting point for the filament by traversing from v0 away
		// from the initial v1.
		while (getNonBlackDegree(v0) == 2)
		{
			if (getNonBlackAdjacent(v0) != v1)
			{
				v1 = v0;
				v0 = getNonBlackAdjacent(v0);
			}
			else
			{
				v1 = v0;
				bool seenOne = false;
				//v0 = v0.Adjacent[1];
				graph_traits<Graph>::out_edge_iterator i, end;
				for(tie(i,end) = out_edges(v0, mGraph); i != end; ++i)
				{
					if(mGraph[*i].color != black_color) 
					{
						if(seenOne)
						{
							v0 = target(*i, mGraph);
							break;
						}
						seenOne = true;
					}
				}
			}
		}
		ExtractFilament(v0, v1, heap, primitives);
	}
}

//http://www.acm.org/pubs/tog/editors/erich/ptinpoly/#ref1
bool RoadGraph::pointInPolygon(const Vector2& loc, const Primitive& p)
{
	bool inPolygon = false;
	// for each point in polygon
	Primitive::const_iterator pIt, pIt2, pEnd;
	for(pIt2 = p.begin(), pIt = pIt2++, pEnd = p.end(); pIt2 != pEnd; pIt = pIt2++)
	{
		if(pointRayCrosses(loc, mGraph[*pIt].mPosition, mGraph[*pIt2].mPosition))
			inPolygon = !inPolygon;
	}
	if(p.begin() != pEnd)
	{
		if(pointRayCrosses(loc, mGraph[*pIt].mPosition, mGraph[*(p.begin())].mPosition))
			inPolygon = !inPolygon;
	}
	return inPolygon;
}

bool RoadGraph::pointRayCrosses(const Vector2& loc, const Vector2& pt1, const Vector2& pt2)
{
	return ((((pt2.y <= loc.y) && (loc.y < pt1.y)) ||
			((pt1.y <= loc.y) && (loc.y < pt2.y))) &&
			(loc.x < (pt1.x - pt2.x) * (loc.y - pt2.y) / (pt1.y - pt2.y) + pt2.x));
}

bool RoadGraph::pointInPolygon2(const Vector2& loc, const Primitive& p)
{
	bool inPolygon = false;
	// for each point in polygon
	Primitive::const_iterator pIt, pIt2, pEnd;
	for(pIt2 = p.begin(), pIt = pIt2++, pEnd = p.end(); pIt2 != pEnd; pIt = pIt2++)
	{
		if(pointRayCrosses(loc, mGraph[*pIt].mPosition, mGraph[*pIt2].mPosition))
			inPolygon = !inPolygon;
	}
	if(p.begin() != pEnd)
	{
		if(pointRayCrosses(loc, mGraph[*pIt].mPosition, mGraph[*(p.begin())].mPosition))
			inPolygon = !inPolygon;
	}
	return inPolygon;
}

Vector2 RoadGraph::findPrimitiveCenter(const Primitive& p)
{
	Primitive::const_iterator pIt, pEnd;
	Vector2 loc(0,0);
	unsigned int count = 0;
	for(pIt = p.begin(), pEnd = p.end(); pIt != pEnd; pIt++, count++)
	{
		loc += mGraph[*pIt].mPosition;
	}
	// no div by zero 
	if(count) return loc / count;
	else return Vector2(0,0);
}

/*
inline bool intersect(const Ogre::Real& x1, const Ogre::Real& y1,
                     const Ogre::Real& x2, const Ogre::Real& y2,
                     const Ogre::Real& x3, const Ogre::Real& y3,
                     const Ogre::Real& x4, const Ogre::Real& y4,
                           Ogre::Real& ix,       Ogre::Real& iy)
{
  Ogre::Real ax = x2 - x1;
  Ogre::Real bx = x3 - x4;

  Ogre::Real lowerx;
  Ogre::Real upperx;
  Ogre::Real uppery;
  Ogre::Real lowery;

  if (ax < 0)
  {
     lowerx = x2;
     upperx = x1;
  }
  else
  {
     upperx = x2;
     lowerx = x1;
  }

  if (bx > 0)
  {
     if ((upperx < x4) || (x3 < lowerx))
     return false;
  }
  else if ((upperx < x3) || (x4 < lowerx))
     return false;

  Ogre::Real ay = y2 - y1;
  Ogre::Real by = y3 - y4;

  if (ay < 0)
  {
     lowery = y2;
     uppery = y1;
  }
  else
  {
     uppery = y2;
     lowery = y1;
  }

  if (by > 0)
  {
     if ((uppery < y4) || (y3 < lowery))
        return false;
  }
  else if ((uppery < y3) || (y4 < lowery))
     return false;

  Ogre::Real cx = x1 - x3;
  Ogre::Real cy = y1 - y3;
  Ogre::Real d  = (by * cx) - (bx * cy);
  Ogre::Real f  = (ay * bx) - (ax * by);

  if (f > 0)
  {
     if ((d < 0) || (d > f))
        return false;
  }
  else if ((d > 0) || (d < f))
     return false;

  Ogre::Real e = (ax * cy) - (ay * cx);

  if (f > 0)
  {
     if ((e < 0) || (e > f))
        return false;
  }
  else if ((e > 0) || (e < f))
     return false;

  Ogre::Real ratio = (ax * -by) - (ay * -bx);

  if (ratio != 0.0)
  {
     ratio = ((cy * -bx) - (cx * -by)) / ratio;
     ix    = x1 + (ratio * ax);
     iy    = y1 + (ratio * ay);
  }
  else
  {
     if ((ax * -cy)==(-cx * ay))
     {
        ix = x3;
        iy = y3;
     }
     else
     {
        ix = x4;
        iy = y4;
     }
  }
  if( ((x1 <= ix && ix <= x2) || (x2 >= ix && ix <= x1))
	  && ((y1 >= iy && iy <= y2) || (y2 >= iy && iy <= y1)))
  {
  }
  else
  {
	  return false;
  }

  return true;
}

bool RoadGraph::lineIntersection(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Vector2& i) const
{
	return intersect(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, i.x, i.y);
}
bool RoadGraph::lineIntersection(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Vector2& intersection) const
{
	Ogre::Real m1,m2,c1,c2,ix,iy;

	// line equation: y = mx + c;
	m1 = (b.y - a.y) / (b.x - a.x);
	m2 = (d.y - c.y) / (d.x - c.x);

	// c = y - mx
	c1 = a.y - (m1 * a.x);
	c2 = c.y - (m2 * c.x);

//	if(c1 = c2)

	// combine the equations to get x
	ix = (c2 - c1) / (m1 - m2);

	//check ix is within line segments
	if ( !((a.x <= ix && ix <= b.x) || (b.x >= ix && ix <= a.x)) 
		 || !((c.x <= ix && ix <= d.x) || (d.x >= ix && ix <= c.x)) )
	{
		return false;
	}

	// get y by inserting into one of the line equations
	iy = (m1 * ix) + c1;

	//check y is within line segments
	if ( !((a.y <= iy && iy <= b.y) || (b.y >= iy && iy <= a.y)) 
		 || !((c.y <= iy && iy <= d.y) || (d.y >= iy && iy <= c.y)) )
	{
		return false;
	}

	intersection.x = ix;
	intersection.y = iy;

	return true;
}


// gk: is shit.
// Based on public domain function by Darel Rex Finley, 2006
// http://alienryderflex.com/intersect/
bool RoadGraph::lineIntersection(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Vector2& intersection) const
{
	Ogre::Real  distAB, theCos, theSin, newX, ABpos ;

	//  Fail if either line is undefined.
	if (a.x==b.x && a.y==b.y || c.x==d.x && c.y==d.y) return false;

	//  (1) Translate the system so that point A is on the origin.
	b.x-=a.x; b.y-=a.y;
	c.x-=a.x; c.y-=a.y;
	d.x-=a.x; d.y-=a.y;

	//  Discover the length of segment A-B.
	distAB=sqrt(b.x*b.x+b.y*b.y);

	//  (2) Rotate the system so that point B is on the positive X a.xis.
	theCos=b.x/distAB;
	theSin=b.y/distAB;
	newX=c.x*theCos+c.y*theSin;
	c.y  =c.y*theCos-c.x*theSin; c.x=newX;
	newX=d.x*theCos+d.y*theSin;
	d.y  =d.y*theCos-d.x*theSin; d.x=newX;

	//  Fail if the lines are parallel.
	if (c.y==d.y) return false;

	//  (3) Discover the position of the intersection point along line A-B.
	ABpos=d.x+(c.x-d.x)*d.y/(d.y-c.y);

	// IMPORTANT
	// gk: this lines make the deifference between a line segment test and full line test
	//  Fail if segment C-D crosses line A-B outside of segment A-B.
	if (ABpos<0 || ABpos>distAB) return false;

	//  (4) Apply the discovered position to line A-B in the original coordinate system.
	intersection.x = a.x+ABpos*theCos;
	intersection.y = a.y+ABpos*theSin;

	//  Success.
	return true; 
}
*/



// All the lineIntersection algorithms are shit, really,
// so i must write my own, well based on these rather easy to 
// follow instructions
// http://www.faqs.org/faqs/graphics/algorithms-faq/
//
//		(Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
//	r = -----------------------------  (eqn 1)
//		(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
//
//		(Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
//	s = -----------------------------  (eqn 2)
//		(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
//
bool RoadGraph::lineIntersection(const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& d, Vector2& intersection) const
{
	Ogre::Vector2  BminusA(b - a);
	Ogre::Vector2 DminusC(d - c);
	Ogre::Real denom = (BminusA.x * DminusC.y) - (BminusA.y * DminusC.x);

	// line are parallel
	if(denom == 0) return false;

	Ogre::Vector2  AminusC(a - c);
	
	Ogre::Real r = ((AminusC.y * DminusC.x) - (AminusC.x * DminusC.y)) / denom;
	if(r < 0 || r > 1) return false;

	Ogre::Real s = ((AminusC.y * BminusA.x) - (AminusC.x * BminusA.y)) / denom;
	if(s < 0 || s > 1) return false;

	//if r and s are 0 then the line are coincident (on top of one another)
        
	// Px=Ax+r(Bx-Ax)
    // Py=Ay+r(By-Ay)
	intersection.x = a.x + r * (BminusA.x);
	intersection.y = a.y + r * (BminusA.y);

	return true;
}

bool RoadGraph::roadIntersection(const Vector2& a, const Vector2& b, const RoadDescriptor rd, Vector2& intersection) const
{
	// get some values for our points
	return lineIntersection(a, b, getNodePosition(getRoadSource(rd)), getNodePosition(getRoadTarget(rd)), intersection);

}

bool RoadGraph::roadIntersection(const RoadDescriptor rd1, const RoadDescriptor rd2, Vector2& intersection) const
{
	// get some values for our points
	return lineIntersection(getNodePosition(getRoadSource(rd1)), getNodePosition(getRoadTarget(rd1)),
							getNodePosition(getRoadSource(rd2)), getNodePosition(getRoadTarget(rd2)), intersection);

}

bool RoadGraph::findClosestIntersection(const Vector2& a, const Vector2& b, RoadDescriptor& rd, Vector2& intersection) const
{
	bool hasIntersection = false;
	Vector2 currentIntersection, closestIntersection;
	RoadDescriptor closestRoad;
	Ogre::Real currentDistance, closestDistance;
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = getRoads(); rIt != rEnd; rIt++)
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

	if(hasIntersection)
	{
		intersection = closestIntersection;
		rd = closestRoad;
	}
	return hasIntersection;	
}


bool RoadGraph::getNodeClosestToPoint(const Ogre::Vector2 &loc, NodeDescriptor &nd, Ogre::Real &distance)
{
	Ogre::Real currDist = numeric_limits<Ogre::Real>::max();
	NodeDescriptor currNode;
	NodeIterator nIt, nEnd;
	bool success = false;
	for(boost::tie(nIt, nEnd) = vertices(mGraph); nIt != nEnd; nIt++)
	{
		Vector2 temp(mGraph[*nIt].mPosition - loc);
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
		distance = Math::Sqrt(currDist);
	}
	return success;
}
