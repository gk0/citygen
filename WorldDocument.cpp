// Includes 
#include "stdafx.h"
#include "WorldDocument.h"

IMPLEMENT_DYNAMIC_CLASS(WorldDocument, wxDocument)

WorldDocument::WorldDocument()
 : mRoadGraph(),
   mRoadVertexMap()
{

}

//function adds node if it is not already there
//otherwise returns false and does nothing
bool WorldDocument::addNode(const Ogre::Vector3& loc)
{
	RoadVertexMap::iterator pos; 
	bool inserted;
	RoadVertex u;

	boost::tie(pos, inserted) = mRoadVertexMap.insert(std::make_pair(loc, RoadVertex()));
	if(inserted) {
		u = add_vertex(loc, mRoadGraph);
		pos->second = u;
	}
	return inserted;
}

bool WorldDocument::moveNode(const Ogre::Vector3& oldLoc, const Ogre::Vector3& newLoc)
{
	bool inserted;
	RoadVertexMap::iterator pos = mRoadVertexMap.find(oldLoc);
	// if node @ oldLoc can be found
	if(pos != mRoadVertexMap.end()) {
		RoadVertex u = pos->second;
		// update Graph Entry
		mRoadGraph[pos->second] = newLoc;
		// update Map
		mRoadVertexMap.erase(pos);
		boost::tie(pos, inserted) = mRoadVertexMap.insert(std::make_pair(newLoc, u));
		return true;
	}else return false;
	return false;
}

bool WorldDocument::removeNode(const Ogre::Vector3& loc)
{
	RoadVertexMap::iterator pos = mRoadVertexMap.find(loc);
	// if node @ loc can be found
	if(pos != mRoadVertexMap.end()) {
		// remove from graph
		remove_vertex(pos->second, mRoadGraph);
		// remove from map
		mRoadVertexMap.erase(pos);
		return true;
	}else return false;
}

//fckme i need to test if the 
bool WorldDocument::addRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2, Road &road)
{
	RoadVertexMap::iterator pos1, pos2;
	pos1 = mRoadVertexMap.find(loc1);
	pos2 = mRoadVertexMap.find(loc2);
	// if node @ loc1 and node @ loc2 can be found
	if(pos1 != mRoadVertexMap.end() && pos2 != mRoadVertexMap.end()) {
		bool inserted;
		// add edge to graph
		boost::tie(road, inserted) = add_edge(pos1->second, pos2->second, mRoadGraph);
		return inserted;
	}
	return false;
}

bool WorldDocument::removeRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2)
{
	RoadVertexMap::iterator pos1, pos2;
	pos1 = mRoadVertexMap.find(loc1);
	pos2 = mRoadVertexMap.find(loc2);
	// if node @ loc1 and node @ loc2 can be found
	if(pos1 != mRoadVertexMap.end() && pos2 != mRoadVertexMap.end()) {
		// remove edge to graph
		remove_edge(pos1->second, pos2->second, mRoadGraph);
		return true;
	}else return false;
}

void WorldDocument::printRoads() 
{	
	boost::graph_traits<RoadGraph>::edge_iterator i, end;
	for (boost::tie(i, end) = edges(mRoadGraph); i != end; ++i) 
	{
		RoadVertex u,v;
		u = source(*i,mRoadGraph);
		v = target(*i,mRoadGraph);
		std::stringstream oss;
		oss << "WorldDocument:edge: " << u << "-" << v << "  "<< mRoadGraph[u] << "-" << mRoadGraph[v];
		Ogre::LogManager::getSingleton().logMessage(oss.str());
	}
}

void WorldDocument::printNodes() 
{
	RoadVertexMap::iterator pos = mRoadVertexMap.begin();
	for(pos;pos != mRoadVertexMap.end(); pos++) {
		std::stringstream oss;
		oss << "WorldDocument:nodeMap: " << pos->first << " -> " << pos->second;
		Ogre::LogManager::getSingleton().logMessage(oss.str());
	}

	boost::graph_traits<RoadGraph>::vertex_iterator i, end;
	for (boost::tie(i, end) = vertices(mRoadGraph); i != end; ++i) {
		std::stringstream oss;
		oss << "WorldDocument:node: " << *i << ": " << mRoadGraph[*i];
		Ogre::LogManager::getSingleton().logMessage(oss.str());
	}
}

std::string WorldDocument::roadVertexToString(RoadVertex v)
{
	std::stringstream oss;
	oss << v;
	return oss.str();
}
std::string WorldDocument::roadToString(Road r)
{
	std::stringstream oss;
	oss << r.m_property;
	return oss.str();
}

std::string WorldDocument::pointerToString(void * ptr) 
{		
	std::stringstream oss;
	oss << ptr;
	return oss.str();
}

std::ostream& WorldDocument::SaveObject(std::ostream& stream)
{
    wxDocument::SaveObject(stream);

	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "yes" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * root = new TiXmlElement( "WorldDocument" );  
	doc.LinkEndChild( root );  

	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue("Citygen generated CityDoc" );  
	root->LinkEndChild( comment );

	TiXmlComment * comment2 = new TiXmlComment();
	comment2->SetValue("kak file format version 1 : 11/07/2006" );  
	root->LinkEndChild( comment2 );  

	TiXmlElement * roadNetwork = new TiXmlElement( "RoadGraph" );  
	root->LinkEndChild( roadNetwork );  
 
	TiXmlElement * nodes = new TiXmlElement( "Nodes" );  
	roadNetwork->LinkEndChild( nodes );  

	boost::graph_traits<RoadGraph>::vertex_iterator vi, vend;
	for (boost::tie(vi, vend) = vertices(mRoadGraph); vi != vend; ++vi) 
	{
		Ogre::Vector3 loc(mRoadGraph[*vi]);
		std::string id(pointerToString(*vi));
	
		TiXmlElement * node;
		node = new TiXmlElement( "Node" );  
		nodes->LinkEndChild( node );  
		node->SetAttribute("id", id);
		node->SetDoubleAttribute("x", loc.x);
		node->SetDoubleAttribute("y", loc.y);
		node->SetDoubleAttribute("z", loc.z);
	}

	//Edges
	TiXmlElement * edges = new TiXmlElement( "Edges" );  
	roadNetwork->LinkEndChild( edges );  

	boost::graph_traits<RoadGraph>::edge_iterator i, end;
	for (boost::tie(i, end) = boost::edges(mRoadGraph); i != end; ++i)
	{
		std::string nodeSource(pointerToString(source(*i, mRoadGraph)));
		std::string nodeTarget(pointerToString(target(*i, mRoadGraph)));
		TiXmlElement * edge;
		edge = new TiXmlElement( "Edge" );  
		edges->LinkEndChild( edge );  
		edge->SetAttribute("source", nodeSource);
		edge->SetAttribute("target", nodeTarget);
	}

	//doc.SaveFile(filename);  //note: I shouldn't need the c_str() bit but when using /MDd I get a runtime error otherwise
	stream << doc;
    return stream;
}

std::istream& WorldDocument::LoadObject(std::istream& stream)
{
    wxDocument::LoadObject(stream);

	TiXmlDocument doc;
	stream >> doc;

	//Yes loading XML does mean trashing our current document
	mRoadGraph.clear();
	mRoadVertexMap.clear();
	std::map<std::string, RoadVertex> nodeIdTranslation;


	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	// block: root node 'WorldDocument'
	{
		pElem=hDoc.FirstChildElement().Element();
		// should always have a valid root but handle gracefully if it does
		if (!pElem) return stream;
		std::string name=pElem->Value();

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}

	// block: 'Nodes'
	{
		pElem=hRoot.FirstChild("RoadGraph").FirstChild("Nodes").FirstChild().Element();
		for( pElem; pElem; pElem=pElem->NextSiblingElement())
		{
			std::string key = pElem->Value();
			if(key == "Node") {
				Ogre::Vector3 loc;
				std::string id = pElem->Attribute("id");
				pElem->QueryFloatAttribute("x", &loc.x);
				pElem->QueryFloatAttribute("y", &loc.y);
				pElem->QueryFloatAttribute("z", &loc.z);

				RoadVertexMap::iterator pos; 
				bool inserted;
				RoadVertex u;

				boost::tie(pos, inserted) = mRoadVertexMap.insert(std::make_pair(loc, RoadVertex()));
				if(inserted) {
					u = add_vertex(loc, mRoadGraph);
					pos->second = u;
					nodeIdTranslation.insert(std::make_pair(id,u));
				}
			}
		}
	}

	// block: 'Edges'
	{
		pElem=hRoot.FirstChild("RoadGraph").FirstChild("Edges").FirstChild().Element();
		for( pElem; pElem; pElem=pElem->NextSiblingElement())
		{
			std::string key = pElem->Value();
			if(key == "Edge") {
				Ogre::Vector3 loc;
				std::string source, target;
				source = pElem->Attribute("source");
				target = pElem->Attribute("target");

				std::map<std::string, RoadVertex>::iterator pos1, pos2;
				pos1 = nodeIdTranslation.find(source);
				pos2 = nodeIdTranslation.find(target);
				// if node @ loc1 and node @ loc2 can be found
				if(pos1 != nodeIdTranslation.end() && pos2 != nodeIdTranslation.end()) {
					bool inserted;
					boost::graph_traits<RoadGraph>::edge_descriptor e;
					// add edge to graph
					boost::tie(e, inserted) = add_edge(pos1->second, pos2->second, mRoadGraph);
				}
			}
		}
	}
    return stream;
}

bool WorldDocument::findRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2, Road &road)
{
	//given the location for two node find the road between them
	RoadVertexMap::iterator pos1, pos2;
	pos1 = mRoadVertexMap.find(loc1);
	pos2 = mRoadVertexMap.find(loc2);
	// if node @ loc1 and node @ loc2 can be found
	if(pos1 != mRoadVertexMap.end() && pos2 != mRoadVertexMap.end())
	{
		bool success;
		boost::tie(road, success) = boost::edge(pos1->second, pos2->second, mRoadGraph);
		return success;
	}
	return false;
}