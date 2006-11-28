// Includes 
#include "stdafx.h"
#include "WorldDocument.h"
using namespace boost;

IMPLEMENT_DYNAMIC_CLASS(WorldDocument, wxDocument)


WorldDocument::WorldDocument()
: wxDocument() {}



std::istream& WorldDocument::LoadObject(std::istream& stream)
{
	TiXmlDocument doc;
	stream >> doc;

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem=hDoc.FirstChildElement().Element();

	// should always have a valid root but handle gracefully if it does
	if (!pElem) return stream;
	//std::string name=pElem->Value();

	// save this for later
	const TiXmlHandle hRoot(pElem);

	// load road graph
	mRoadGraph.loadXML(hRoot);

    return stream;
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

	TiXmlElement * roadNetwork = mRoadGraph.saveXML();
	root->LinkEndChild(roadNetwork);

	//doc.SaveFile(filename);  //note: I shouldn't need the c_str() bit but when using /MDd I get a runtime error otherwise
	stream << doc;
    return stream;
}

void WorldDocument::divideCells()
{
	// get a set of primitives out of the roadgraph
	PrimitiveVec primitives;
	mRoadGraph.ExtractPrimitives( primitives );

	// for each primitive create a cell for closed loops
	for(PrimitiveVec::const_iterator pit = primitives.begin(); pit != primitives.end(); pit++)
	{
		const Primitive& primitive(*pit);
		if(primitive.type == MINIMAL_CYCLE)
		{
			//Create Cell
			mCityCells.push_back(CityCell(&mRoadGraph, primitive)); 
		}
	}
}


bool WorldDocument::pickCell(const Ogre::Vector2& loc, CityCell* &cell)
{
	// given a 2D point select a cell
	CityCellIterator ccIt, ccEnd;
	for(ccIt = mCityCells.begin(), ccEnd = mCityCells.end(); ccIt != ccEnd; ccIt++)
	{
		// if 
		if(mRoadGraph.pointInPolygon(loc, (*ccIt).mBoundaryPrimitive))
		{
			cell = &(*ccIt);
			return true;
		}
	}
	return false;
}

std::pair<CityCellIterator, CityCellIterator> WorldDocument::getCells()
{
	return std::make_pair(mCityCells.begin(), mCityCells.end());
}

