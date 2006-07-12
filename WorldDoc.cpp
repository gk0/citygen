// Includes 
#include "stdafx.h"
#include "WorldDoc.h"

WorldDoc::WorldDoc()
 : mRoadNetwork(),
   mLocationNodeMap()
{
	
}

//function adds node if it is not already there
//otherwise returns false and does nothing
bool WorldDoc::addNode(const Ogre::Vector3& loc)
{
	LocationNodeMap::iterator pos; 
	bool inserted;
	Node u;

	boost::tie(pos, inserted) = mLocationNodeMap.insert(std::make_pair(loc, Node()));
	if(inserted) {
		u = add_vertex(loc, mRoadNetwork);
		pos->second = u;
	}
	return inserted;
}

bool WorldDoc::moveNode(const Ogre::Vector3& oldLoc, const Ogre::Vector3& newLoc)
{
	bool inserted;
	LocationNodeMap::iterator pos = mLocationNodeMap.find(oldLoc);
	// if node @ oldLoc can be found
	if(pos != mLocationNodeMap.end()) {
		Node u = pos->second;
		// update Graph Entry
		mRoadNetwork[pos->second] = newLoc;
		// update Map
		mLocationNodeMap.erase(pos);
		boost::tie(pos, inserted) = mLocationNodeMap.insert(std::make_pair(newLoc, u));
		return true;
	}else return false;
	return false;
}

bool WorldDoc::removeNode(const Ogre::Vector3& loc)
{
	LocationNodeMap::iterator pos = mLocationNodeMap.find(loc);
	// if node @ loc can be found
	if(pos != mLocationNodeMap.end()) {
		// remove from graph
		remove_vertex(pos->second, mRoadNetwork);
		// remove from map
		mLocationNodeMap.erase(pos);
		return true;
	}else return false;
}

void WorldDoc::printNodes() 
{
	LocationNodeMap::iterator pos = mLocationNodeMap.begin();
	for(pos;pos != mLocationNodeMap.end(); pos++) {
		std::cout<< pos->first << " -> " << pos->second<<std::endl;
	}

	boost::graph_traits<RoadNetwork>::vertex_iterator i, end;
	for (boost::tie(i, end) = vertices(mRoadNetwork); i != end; ++i) {
		std::cout<< *i << ": " << mRoadNetwork[*i]<<std::endl;
	}
	std::cout<<std::endl;
}

//fckme i need to test if the 
bool WorldDoc::addRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2)
{
	LocationNodeMap::iterator pos1, pos2;
	pos1 = mLocationNodeMap.find(loc1);
	pos2 = mLocationNodeMap.find(loc2);
	// if node @ loc1 and node @ loc2 can be found
	if(pos1 != mLocationNodeMap.end() && pos2 != mLocationNodeMap.end()) {
		bool inserted;
		boost::graph_traits<RoadNetwork>::edge_descriptor e;
		// add edge to graph
		boost::tie(e, inserted) = add_edge(pos1->second, pos2->second, mRoadNetwork);
		return inserted;
	}else return false;
}

bool WorldDoc::removeRoad(const Ogre::Vector3& loc1, const Ogre::Vector3& loc2)
{
	LocationNodeMap::iterator pos1, pos2;
	pos1 = mLocationNodeMap.find(loc1);
	pos2 = mLocationNodeMap.find(loc2);
	// if node @ loc1 and node @ loc2 can be found
	if(pos1 != mLocationNodeMap.end() && pos2 != mLocationNodeMap.end()) {
		// remove edge to graph
		remove_edge(pos1->second, pos2->second, mRoadNetwork);
		return true;
	}else return false;
}

void WorldDoc::printEdges() 
{
	boost::graph_traits<RoadNetwork>::edge_iterator i, end;
	for (boost::tie(i, end) = edges(mRoadNetwork); i != end; ++i) {
		std::cout<< source(*i,mRoadNetwork) << " - " << target(*i,mRoadNetwork)<< "  "<<mRoadNetwork[source(*i,mRoadNetwork)]<< " - " <<mRoadNetwork[target(*i,mRoadNetwork)]<<std::endl;
	}
	std::cout<<std::endl;
}


// load the named file and dump its structure to STDOUT
/*void WorldDoc::dump_to_stdout(const char* pFilename)
{
	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
	{
		printf("\n%s:\n", pFilename);
		dump_to_stdout( &doc ); // defined later in the tutorial
	}
	else
	{
		printf("Failed to load file \"%s\"\n", pFilename);
	}
}*/

void WorldDoc::saveXML(const std::string& filename) 
{
	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "yes" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * root = new TiXmlElement( "WorldDoc" );  
	doc.LinkEndChild( root );  

	TiXmlComment * comment = new TiXmlComment();
	comment->SetValue("Citygen generated CityDoc" );  
	root->LinkEndChild( comment );

	TiXmlComment * comment2 = new TiXmlComment();
	comment2->SetValue("kak file format version 1 : 11/07/2006" );  
	root->LinkEndChild( comment2 );  

	TiXmlElement * roadNetwork = new TiXmlElement( "RoadNetwork" );  
	root->LinkEndChild( roadNetwork );  
 
	TiXmlElement * nodes = new TiXmlElement( "Nodes" );  
	roadNetwork->LinkEndChild( nodes );  

	boost::graph_traits<RoadNetwork>::vertex_iterator vi, vend;
	for (boost::tie(vi, vend) = vertices(mRoadNetwork); vi != vend; ++vi) 
	{
		Ogre::Vector3 loc(mRoadNetwork[*vi]);
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

	boost::graph_traits<RoadNetwork>::edge_iterator i, end;
	for (boost::tie(i, end) = boost::edges(mRoadNetwork); i != end; ++i)
	{
		std::string nodeSource(pointerToString(source(*i, mRoadNetwork)));
		std::string nodeTarget(pointerToString(target(*i, mRoadNetwork)));
		TiXmlElement * edge;
		edge = new TiXmlElement( "Edge" );  
		edges->LinkEndChild( edge );  
		edge->SetAttribute("source", nodeSource);
		edge->SetAttribute("target", nodeTarget);
	}

	doc.SaveFile(filename);  //note: I shouldn't need the c_str() bit but when using /MDd I get a runtime error otherwise

}


void WorldDoc::loadXML(const std::string& filename) 
{
	TiXmlDocument doc(filename);
	if(!doc.LoadFile()) {
		//error handling
		return;
	}

	//Yes loading XML does mean trashing our current document
	mRoadNetwork.clear();
	mLocationNodeMap.clear();
	std::map<std::string, Node> nodeIdTranslation;


	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	// block: root node 'WorldDoc'
	{
		pElem=hDoc.FirstChildElement().Element();
		// should always have a valid root but handle gracefully if it does
		if (!pElem) return;
		std::string name=pElem->Value();

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}

	// block: 'Nodes'
	{
		pElem=hRoot.FirstChild("RoadNetwork").FirstChild("Nodes").FirstChild().Element();
		for( pElem; pElem; pElem=pElem->NextSiblingElement())
		{
			std::string key = pElem->Value();
			if(key == "Node") {
				Ogre::Vector3 loc;
				std::string id = pElem->Attribute("id");
				pElem->QueryFloatAttribute("x", &loc.x);
				pElem->QueryFloatAttribute("y", &loc.y);
				pElem->QueryFloatAttribute("z", &loc.z);

				LocationNodeMap::iterator pos; 
				bool inserted;
				Node u;

				boost::tie(pos, inserted) = mLocationNodeMap.insert(std::make_pair(loc, Node()));
				if(inserted) {
					u = add_vertex(loc, mRoadNetwork);
					pos->second = u;
					nodeIdTranslation.insert(std::make_pair(id,u));
				}
			}
		}
	}

	// block: 'Edges'
	{
		pElem=hRoot.FirstChild("RoadNetwork").FirstChild("Edges").FirstChild().Element();
		for( pElem; pElem; pElem=pElem->NextSiblingElement())
		{
			std::string key = pElem->Value();
			if(key == "Edges") {
				Ogre::Vector3 loc;
				std::string source, target;
				source = pElem->Attribute("source");
				target = pElem->Attribute("target");

				std::map<std::string, Node>::iterator pos1, pos2;
				pos1 = nodeIdTranslation.find(source);
				pos2 = nodeIdTranslation.find(target);
				// if node @ loc1 and node @ loc2 can be found
				if(pos1 != nodeIdTranslation.end() && pos2 != nodeIdTranslation.end()) {
					bool inserted;
					boost::graph_traits<RoadNetwork>::edge_descriptor e;
					// add edge to graph
					boost::tie(e, inserted) = add_edge(pos1->second, pos2->second, mRoadNetwork);
				}
			}
		}
	}
}

std::string WorldDoc::pointerToString(void * ptr) 
{		
		std::stringstream oss;
		oss << ptr;
		return oss.str();
}

