// Includes
#include "stdafx.h"
#include "WorldFrame.h"
#include "SimpleNode.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldCell.h"

//tools
#include "ToolView.h"
#include "ToolNodeSelect.h"
#include "ToolNodeAdd.h"
#include "ToolNodeDelete.h"


// Namespace 
using namespace Ogre;
using namespace std;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

// Required for WX
IMPLEMENT_CLASS(WorldFrame, wxControl)

// Event Table
BEGIN_EVENT_TABLE(WorldFrame, wxControl)
	EVT_CHAR(WorldFrame::OnChar)
	EVT_ERASE_BACKGROUND(WorldFrame::OnEraseBackground) 
	EVT_KILL_FOCUS(WorldFrame::OnFocusLost)
	EVT_SET_FOCUS(WorldFrame::OnFocusSet)
	EVT_MOTION(WorldFrame::OnMouseMove)
	EVT_LEFT_DOWN(WorldFrame::OnLeftPressed)
	EVT_MOUSEWHEEL(WorldFrame::OnMouseWheel)
//	EVT_PAINT(WorldFrame::OnPaint)
	EVT_SIZE(WorldFrame::OnSize)
	EVT_TIMER(ID_RENDERTIMER, WorldFrame::OnTimer)
END_EVENT_TABLE()

/** Brief description which ends at this dot. Details follow
 *  here.
 */
WorldFrame::WorldFrame(wxFrame* parent) 
	: wxControl(parent, -1),
	  mTimer(this, ID_RENDERTIMER)
{
	mCamera = 0;
	mRenderWindow = 0;
	mSceneManager = 0;
	mViewport = 0;

	// --------------------
	// Create a new parameters list according to compiled OS
	NameValuePairList params;
	String handle;
#ifdef __WXMSW__
	handle = StringConverter::toString((size_t)((HWND)GetHandle()));
#elif defined(__WXGTK__)
	// TODO: Someone test this. you might to use "parentWindowHandle" if this
	// does not work.  Ogre 1.2 + Linux + GLX platform wants a string of the
	// format display:screen:window, which has variable types ulong:uint:ulong.
	GtkWidget* widget = GetHandle();
	gtk_widget_realize( widget );	// Mandatory. Otherwise, a segfault happens.
	stringstream handleStream;
	Display* display = GDK_WINDOW_XDISPLAY( widget->window );
	Window wid = GDK_WINDOW_XWINDOW( widget->window );	// Window is a typedef for XID, which is a typedef for unsigned int
	/* Get the right display (DisplayString() returns ":display.screen") */
	string displayStr = DisplayString( display );
	displayStr = displayStr.substr( 1, ( displayStr.find( ".", 0 ) - 1 ) );
	/* Put all together */
	handleStream << displayStr << ':' << DefaultScreen( display ) << ':' << wid;
	handle = handleStream.str();
#else
	#error Not supported on this platform.
#endif
	params["externalWindowHandle"] = handle;

	// Get wx control window size
	int width, height;
	GetSize(&width, &height);

	// Create the render window
	mRenderWindow = Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);

	// Create the SceneManager, in this case the terrainscenemanager
    mSceneManager = Root::getSingleton().createSceneManager("TerrainSceneManager");
    createCamera();
    createViewport();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Make sure assets are loaded before we create the scene
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	//createScene();
	isDocOpen = false;

	mEditMode = MainWindow::view;
	mActiveTool = MainWindow::addNode;

	mTools.push_back(new ToolView(this));
	mTools.push_back(new ToolNodeSelect(this));
	mTools.push_back(new ToolNodeAdd(this, mSceneManager, mRoadGraph, mSimpleRoadGraph));
	mTools.push_back(new ToolNodeDelete(this));
	mActiveTool = MainWindow::viewTool;

	//toggleTimerRendering(); // only really to test fps
}


// Destructor //
WorldFrame::~WorldFrame()
{
	// destroy scene objects
	if(isDocOpen)
	{
		mTools[mActiveTool]->deactivate();
		destroyScene();
	}
	destroyViewport();
	destroyCamera();
	
	// destroy scene manager
	Root::getSingleton().destroySceneManager(mSceneManager);

	// destroy render window
	Root::getSingleton().detachRenderTarget(mRenderWindow);
	delete mRenderWindow;
	mRenderWindow = 0;

	// delete tools
	vector<Tool*>::iterator tIt, tEnd;
	for(uint32 i=0; i<mTools.size(); i++) 
		delete mTools[i];
}

void WorldFrame::destroyCamera() 
{
	if (mCamera)
	{
		mSceneManager->destroyCamera(mCamera);
		mCamera = 0;
	}
}

void WorldFrame::destroyViewport() 
{
	if (mViewport)
	{
		mRenderWindow->removeViewport(mViewport->getZOrder());
		mViewport = 0;
	}
}

void WorldFrame::createCamera(void)
{    
	// Create the camera
    mCamera = mSceneManager->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(825,175,825);
    mCamera->setOrientation(Quaternion(-0.49f, 0.17f, 0.81f, 0.31f));

    // Look back along -Z
    //mCamera->lookAt(Vector3(0,-10,0));
	mCamera->setNearClipDistance( 1 );
    mCamera->setFarClipDistance( 1000 );
}


void WorldFrame::createScene(void) 
{	
    // Set ambient light
    mSceneManager->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    // Create a light
    Light* l = mSceneManager->createLight("MainLight");
    // Accept default settings: point light, white diffuse, just set position
    // NB I could attach the light to a SceneNode if I wanted it to move automatically with
    //  other objects, but I don't
    l->setPosition(20,180,50);

    // Fog
    // NB it's VERY important to set this before calling setWorldGeometry 
    // because the vertex program picked will be different
	ColourValue fadeColour(0.76f, 0.86f, 0.93f);
	mSceneManager->setFog( FOG_LINEAR, fadeColour, .001f, 500, 1000);
	mRenderWindow->getViewport(0)->setBackgroundColour(fadeColour);

    string terrain_cfg("terrain.cfg");
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        terrain_cfg = mResourcePath + terrain_cfg;
#endif
    mSceneManager -> setWorldGeometry( terrain_cfg );

    // Infinite far plane?
    if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
    {
        mCamera->setFarClipDistance(0);
    }
	mCamera->setNearClipDistance(0.1);

    // Define the required skyplane
    Plane plane;
    // 5000 world units from the camera
    plane.d = 5000;
    // Above the camera, facing down
    plane.normal = -Vector3::UNIT_Y;

	// Create our ray query
	mRaySceneQuery = mSceneManager->createRayQuery(Ray());
}

void WorldFrame::createViewport(void)
{
	if(mViewport) destroyViewport();
    // Create one viewport, entire window
    mViewport = mRenderWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0.5f,0.5f,0.5f));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));
}

void WorldFrame::destroyScene(void)
{	
	mRenderWindow->getViewport(0)->setBackgroundColour(ColourValue(0.5f, 0.5f, 0.5f));

	// destroy ray
	delete mRaySceneQuery;

	// Remove Cells
	set<WorldCell*>::iterator cIt, cEnd;
	for(cIt = mCells.begin(), cEnd = mCells.end(); cIt != cEnd; cIt++)
		delete (*cIt);
	mCells.clear();

	// Remove roads from the scene
	map<SceneNode*, WorldRoad*>::const_iterator ri, rend;
	for (ri = mSceneRoadMap.begin(), rend = mSceneRoadMap.end(); ri != rend; ++ri)
	{
		delete ri->second;
	}
	mSceneRoadMap.clear();

	// Remove nodes from the scene
	map<SceneNode*, WorldNode*>::const_iterator ni, nend;
	for (ni = mSceneNodeMap.begin(), nend = mSceneNodeMap.end(); ni != nend; ++ni)
	{
		delete ni->second;
	}
	mSceneNodeMap.clear();


	mSceneManager->clearScene();
}

// Moves the view
void WorldFrame::cameraMove(Real x, Real y, Real z)
{
	modify(true);
	mCamera->moveRelative(Vector3(x, y, z));
	stringstream oss;
	oss <<"Camera position: "<<mCamera->getPosition();
	LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
}

// Rotates the view
void WorldFrame::cameraRotate(Real yaw, Real pitch)
{
	modify(true);
	mCamera->yaw(yaw * (mCamera->getFOVy() / mCamera->getAspectRatio() / 320.0f));
	mCamera->pitch(pitch * (mCamera->getFOVy() / 240.0f));

	stringstream oss;
	oss <<"Camera orientation: " << mCamera->getOrientation();
	LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
}

bool WorldFrame::highlightNodeFromLoc(const Vector2 &loc)
{
	//TODO: 
	#define HIGHLIGHTNODELOCSNAPSQ 16
	NodeId nd;
	if(mSimpleRoadGraph.snapToNode(loc, HIGHLIGHTNODELOCSNAPSQ, nd))
	{
		highlightNode(static_cast<WorldNode*>(mSimpleRoadGraph.getNode(nd)));
		return true;
	}
	else
		return false;
}

void WorldFrame::OnChar(wxKeyEvent& e)
{
	if(!isDocOpen) return;
	mTools[mActiveTool]->OnChar(e);
}

void WorldFrame::OnEraseBackground(wxEraseEvent &e)
{
	update();
}

void WorldFrame::OnFocusSet(wxFocusEvent& e)
{
}

void WorldFrame::OnFocusLost(wxFocusEvent& e)
{
}


void WorldFrame::OnLeftPressed(wxMouseEvent &e)
{
	if(!isDocOpen) return;
	//If you click on me get back focus
	//focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();

	mTools[mActiveTool]->OnLeftPressed(e);
}


void WorldFrame::OnMouseMove(wxMouseEvent &e)
{
	if(!isDocOpen) return;
	mTools[mActiveTool]->OnMouseMove(e);
}

void WorldFrame::OnMouseWheel(wxMouseEvent &e)
{
	if(!isDocOpen) return;
	mTools[mActiveTool]->OnMouseWheel(e);
}

void WorldFrame::OnPaint(wxPaintEvent &WXUNUSED(e))
{
	wxPaintDC dc(this);
	update();
}

void WorldFrame::OnSize(wxSizeEvent &e)
{
	// Setting new size;
	int width;
	int height;
	GetSize(&width, &height);
	if(!mRenderWindow) return;
	mRenderWindow->resize( width, height );
	// Letting Ogre know the window has been resized;
	mRenderWindow->windowMovedOrResized();
	// Set the aspect ratio for the new size;
	if (mCamera)
		mCamera->setAspectRatio(Real(width) / Real(height));

	update();
}


void WorldFrame::OnTimer(wxTimerEvent &e)
{
	update();
}

void WorldFrame::toggleTimerRendering()
{
	// Toggle Start/Stop
	if (mTimer.IsRunning())
		mTimer.Stop();
	mTimer.Start(10);
}

void WorldFrame::update()
{
	//wxControl::update();

	// render nodes
	map< Ogre::SceneNode*, WorldNode* >::iterator nIt, nEnd;
	for(nIt = mSceneNodeMap.begin(), nEnd = mSceneNodeMap.end(); nIt != nEnd; nIt++)
		nIt->second->validate();

	// render roads
	map< Ogre::SceneNode*, WorldRoad* >::iterator rIt, rEnd;
	for(rIt = mSceneRoadMap.begin(), rEnd = mSceneRoadMap.end(); rIt != rEnd; rIt++)
		rIt->second->validate();

	// render cells
	//map< Ogre::SceneNode*, WorldCell* >::iterator cIt, cEnd;
	set<WorldCell*>::iterator cIt, cEnd;
	for(cIt = mCells.begin(), cEnd = mCells.end(); cIt != cEnd; cIt++)
		(*cIt)->validate();

	if(mCamera) Root::getSingleton().renderOneFrame();
}

bool WorldFrame::loadXML(const TiXmlHandle& worldRoot)
{
	//
	onCloseDoc();
	onNewDoc();

	// Load Camera
	{
		TiXmlElement *cameraChild = worldRoot.FirstChild("camera").FirstChild().Element();

		for(; cameraChild; cameraChild=cameraChild->NextSiblingElement())
		{
			string key = cameraChild->Value();
			if(key == "position")
			{
				Real x, y, z;
				cameraChild->QueryFloatAttribute("x", &x);
				cameraChild->QueryFloatAttribute("y", &y);
				cameraChild->QueryFloatAttribute("z", &z);
				mCamera->setPosition(x, y, z);
			}
			else if(key == "direction")
			{
				Real x, y, z;
				cameraChild->QueryFloatAttribute("x", &x);
				cameraChild->QueryFloatAttribute("y", &y);
				cameraChild->QueryFloatAttribute("z", &z);
				mCamera->setDirection(x, y, z);
			}
		}
	}


	// a translation map is used to find the nodes for edge creation
	map<string, WorldNode*> nodeIdTranslation;

	// an intermediate structure is used for edges since there 
	// is no guarantee the nodes have been loaded first
	vector< pair<string, string> > edgeData;

	// TODO:
	// check graph id
	// check undirected

	// scan xml
	TiXmlElement* pElem=worldRoot.FirstChild("graph").FirstChild().Element();
	for(; pElem; pElem=pElem->NextSiblingElement())
	{
		string key = pElem->Value();
		if(key == "node") 
		{
			Real x,y;
			string strId = pElem->Attribute("id");
			pElem->QueryFloatAttribute("x", &x);
			pElem->QueryFloatAttribute("y", &y);

			WorldNode* wn = createNode();
			wn->setPosition2D(x, y);
			nodeIdTranslation.insert(make_pair(strId, wn));
		}
		else if(key == "edge") 
		{
			edgeData.push_back(make_pair(pElem->Attribute("source"), pElem->Attribute("target")));
		}
	}

	// create the edges now that all graph data has been read
	for(unsigned int i=0; i<edgeData.size(); i++)
	{
		map<string, WorldNode*>::iterator sourceIt, targetIt;
		sourceIt = nodeIdTranslation.find(edgeData[i].first);
		targetIt = nodeIdTranslation.find(edgeData[i].second);
		// if source node and target node can be found
		if(sourceIt != nodeIdTranslation.end() && targetIt != nodeIdTranslation.end()) 
		{
			WorldRoad* wr = createRoad(sourceIt->second, targetIt->second);
			assert(wr != 0);
		}
	}

	update();
	return true;
}

TiXmlElement* WorldFrame::saveXML()
{
	TiXmlElement * root = new TiXmlElement( "WorldDocument" );  

	// Save Camera
	{
		TiXmlElement *camera = new TiXmlElement("camera"); 
		camera->SetAttribute("id", "1");
		root->LinkEndChild(camera);
		
		const Vector3 &camPos(mCamera->getPosition());
		TiXmlElement *position = new TiXmlElement("position");
		position->SetDoubleAttribute("x", camPos.x);
		position->SetDoubleAttribute("y", camPos.y);
		position->SetDoubleAttribute("z", camPos.z);
		camera->LinkEndChild(position);

		const Vector3 &camDir(mCamera->getDirection());
		TiXmlElement *direction = new TiXmlElement("direction");
		direction->SetDoubleAttribute("x", camDir.x);
		direction->SetDoubleAttribute("y", camDir.y);
		direction->SetDoubleAttribute("z", camDir.z);
		camera->LinkEndChild(direction);
	}

	//<graph id="roadgraph" edgedefault="undirected">
	TiXmlElement *roadNetwork = new TiXmlElement("graph"); 
	roadNetwork->SetAttribute("id", "roadgraph");
	roadNetwork->SetAttribute("edgedefault", "undirected");
	root->LinkEndChild( roadNetwork );

	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mSimpleRoadGraph.getNodes(); nIt != nEnd; nIt++) 
	{
		Vector2 loc(mSimpleRoadGraph.getNode(*nIt)->getPosition2D());
		//string id(pointerToString(*vi));
		//string id
	
		TiXmlElement * node;
		node = new TiXmlElement( "node" );  
		node->SetAttribute("id", (int) *nIt);
		node->SetDoubleAttribute("x", loc.x);
		node->SetDoubleAttribute("y", loc.y);
		roadNetwork->LinkEndChild( node );
	}

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = mSimpleRoadGraph.getRoads(); rIt != rEnd; rIt++) 
	{
		TiXmlElement * edge;
		edge = new TiXmlElement( "edge" );  
		edge->SetAttribute("source", (int) mSimpleRoadGraph.getSrc(*rIt));
		edge->SetAttribute("target", (int) mSimpleRoadGraph.getDst(*rIt));
		roadNetwork->LinkEndChild( edge ); 
	}
	
	return root;
}

void WorldFrame::setEditMode(MainWindow::EditMode mode) 
{
	mEditMode = mode;
	switch(mode) {
	case MainWindow::view:
		endNodeMode();
		break;
	case MainWindow::node:
		beginNodeMode();
		break;
	case MainWindow::road:
		endNodeMode();
		break;
	case MainWindow::cell:
		endNodeMode();
		break;
	}
	Update();
}

void WorldFrame::endNodeMode()
{
	selectNode(0);
}

void WorldFrame::beginNodeMode()
{
	selectNode(0);
}

void WorldFrame::selectNode(WorldNode* wn)
{
	if(mSelectedNode) mSelectedNode->showSelected(false);
	mSelectedNode = wn;
	if(mSelectedNode)
	{
		mSelectedNode->showSelected(true);
	}
	//mNodePropertyPage->setWorldNode(wn);
	//mNodePropertyPage->update();
}

void WorldFrame::highlightNode(WorldNode* wn)
{
	if(mHighlightedNode) mHighlightedNode->showHighlighted(false);
	mHighlightedNode = wn;
	if(mHighlightedNode)
	{
		mHighlightedNode->showHighlighted(true);
	}
}

void WorldFrame::setActiveTool(MainWindow::ActiveTool tool) 
{
	mTools[mActiveTool]->deactivate();
	mActiveTool = tool;
	mTools[mActiveTool]->activate();
}

bool WorldFrame::pickTerrainIntersection(wxMouseEvent& e, Vector3& pos)
{
	// Setup the ray scene query
	float mouseX = float(1.0f/mViewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/mViewport->getActualHeight()) * e.GetY();
	Ray mouseRay = mCamera->getCameraToViewportRay(mouseX, mouseY);

	mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setSortByDistance(false);
	RaySceneQueryResult result = mRaySceneQuery->execute();
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++ )
	{
		if(itr->worldFragment)
		{
    		pos = itr->worldFragment->singleIntersection;
			return true;
		}
	}
	return false;
}

bool WorldFrame::pickNode(wxMouseEvent& e, WorldNode *&wn)
{
	// Setup the ray scene query
	float mouseX = float(1.0f/mViewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/mViewport->getActualHeight()) * e.GetY();
	Ray mouseRay = mCamera->getCameraToViewportRay(mouseX, mouseY);

	mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setSortByDistance(true);
	RaySceneQueryResult result = mRaySceneQuery->execute();
	for(RaySceneQueryResult::const_iterator itr = result.begin( ); itr != result.end(); itr++ )
	{
		if ( itr->movable && itr->movable->getName().substr(0, 4) == "node" )
		{
			wn = mSceneNodeMap[itr->movable->getParentSceneNode()];
    		return true;
		}
	}
	return false;
}

bool WorldFrame::pickNode(wxMouseEvent &e, Real snapSq, WorldNode *&wn)
{
	Vector3 pos3D;
	if(pickTerrainIntersection(e, pos3D)) 
	{
		Vector2 pos2D(pos3D.x, pos3D.z);
		NodeId nd;
		if(mSimpleRoadGraph.snapToNode(pos2D, snapSq, nd))
		{
			// all nodes in the simple graph are of type WorldNode
			NodeInterface *ni = mSimpleRoadGraph.getNode(nd);
			assert(typeid(*ni) == typeid(WorldNode));
			wn = static_cast<WorldNode*>(ni);
			return true;
		}
	}
	return false;
}

WorldNode* WorldFrame::createNode()
{
	WorldNode* wn = new WorldNode(mRoadGraph, mSimpleRoadGraph, mSceneManager);
	//wn->mSimpleNodeId = mSimpleRoadGraph.addNode(wn);
	//wn->mNodeId = mRoadGraph.addNode(wn);
	mSceneNodeMap[wn->getSceneNode()] = wn;

	modify(true);
	return wn;
}

// TODO: atm this function delete the cell by deleting the roads,
// ideally we'd like to save the cell and its lovely user specified params
void WorldFrame::insertNodeOnRoad(WorldNode* wn, WorldRoad* wr)
{	
	//NOTE: need to write an insert node function for WorldCell
	// sux
/*
	modify(true);

	// get cells attached to this road
	vector<WorldCell*> attachedCells;
	vector<WorldObject*> attachments(wr->getAllAttachments());
	vector<WorldObject*>::iterator aIt, aEnd;
	for(aIt = attachments.begin(), aEnd = attachments.end(); aIt != aEnd; aIt++)
	{
		// if attachment is a cell
		if(typeid(*(*aIt)) == typeid(WorldCell))
		{
			attachedCells.push_back(static_cast<WorldCell*>(*aIt));
		}
	}

	// get cell boundaries and params
	size_t numOfCells = attachedCells.size();
	vector< vector<RoadInterface*> > boundaries(numOfCells);
	vector<GrowthGenParams> genParams(numOfCells);
	for(size_t i=0; i<numOfCells; i++)
	{
		// get cell data 
		boundaries[i] = attachedCells[i]->getBoundary();

		// remove road from boundary
		boundaries[i].erase(static_cast<RoadInterface*>(wr));

		// set incomplete boundary
		attachedCells[i]->setBoundary(boundaries[i]);
	}

	// store nodes
	WorldNode* wn1 = static_cast<WorldNode*>(wr->getSrcNode());
	WorldNode* wn2 = static_cast<WorldNode*>(wr->getDstNode());

	// remove road
	deleteRoad(wr);

	// create replacement roads
	RoadId rd;
	mSimpleRoadGraph.addRoad(wn1->mSimpleNodeId, wn->mSimpleNodeId, rd);
	WorldRoad* wr1 = new WorldRoad(mSceneManager, wn1, wn, mRoadGraph);
	wr1->mSimpleRoadId = rd;
	mSimpleRoadGraph.setRoad(rd, wr1);
	mSceneRoadMap[wr1->getSceneNode()] = wr1;

	mSimpleRoadGraph.addRoad(wn->mSimpleNodeId, wn2->mSimpleNodeId, rd);
	WorldRoad* wr2 = new WorldRoad(mSceneManager, wn, wn2, mRoadGraph);
	wr2->mSimpleRoadId = rd;
	mSimpleRoadGraph.setRoad(rd, wr2);
	mSceneRoadMap[wr2->getSceneNode()] = wr2;

	// create replacement cells
	for(size_t i=0; i<numOfCells; i++)
	{	
		// add new roads to boundary
		boundaries[i].insert(wr1);
		boundaries[i].insert(wr2);

		// set complete boundary
		attachedCells[i]->setBoundary(boundaries[i]);
	}
*/
}

void WorldFrame::deleteNode(WorldNode* wn)
{
	modify(true);

	//delete any connected roads
	while(true)
	{
		RoadIterator2 rIt, rEnd;
		boost::tie(rIt, rEnd) = mSimpleRoadGraph.getRoadsFromNode(wn->mSimpleNodeId);
		if(rIt == rEnd) break;
		WorldRoad* wr = static_cast<WorldRoad*>(mSimpleRoadGraph.getRoad(*rIt));
		deleteRoad(wr);
	}

	// update current node if necessary
	if(mHighlightedNode == wn) mHighlightedNode = 0;
	if(mSelectedNode == wn) selectNode(0);

	//Delete the Node
	mSimpleRoadGraph.removeNode(wn->mSimpleNodeId);
	mRoadGraph.removeNode(wn->mNodeId);
	mSceneNodeMap.erase(wn->getSceneNode());

	delete wn;
}

WorldRoad* WorldFrame::createRoad(WorldNode* wn1, WorldNode* wn2)
{
	modify(true);

	if(!mSimpleRoadGraph.testRoad(wn1->mSimpleNodeId, wn2->mSimpleNodeId))
	{
		// create the road in the scene
		WorldRoad* wr = new WorldRoad(wn1, wn2, mRoadGraph, mSimpleRoadGraph, mSceneManager);
		mSceneRoadMap[wr->getSceneNode()] = wr;

		// check the road graph to get a count of the number of cycles
		vector< vector<NodeInterface*> > nodeCycles;
		vector< vector<RoadInterface*> > roadCycles;
		vector<RoadInterface*> filaments;
		mSimpleRoadGraph.extractPrimitives(filaments, nodeCycles, roadCycles);
		//mRoadGraph.extractPrimitives(filaments, nodeCycles, roadCycles);

		// if the number of cycles is greater than the number of cells
		// then we have most definitely made a new cell
		if(roadCycles.size() > mCells.size())
		{
			// find the new roadCycles
			WorldCell* alteredCell = 0;
			set<WorldCell*>::iterator cellIt, cellEnd;
			for(cellIt = mCells.begin(), cellEnd = mCells.end(); cellIt != cellEnd; cellIt++)
			{
				bool cellFound = false;

				// These should be the same size
				assert(nodeCycles.size() == roadCycles.size());

				vector< vector<NodeInterface*> >::iterator ncIt = nodeCycles.begin();
				vector< vector<RoadInterface*> >::iterator rcIt, rcEnd;

				for(rcIt = roadCycles.begin(), rcEnd = roadCycles.end(); rcIt != rcEnd; rcIt++, ncIt++)
				{
					// if cell has boundary of cycle
					if((*cellIt)->compareBoundary(*rcIt))
					{
						// remove cycle, as its not new
						roadCycles.erase(rcIt);
						nodeCycles.erase(ncIt);
						cellFound = true;
						break;
					}
				}

				// if no match was found
				if(!cellFound)
				{
					assert(alteredCell == 0);	// there should only ever be one changed cell
					alteredCell = *cellIt;
				}
			}

			// two options are possible:
			switch(roadCycles.size()) 
			{
			// 1: created a new cell
			case 1:
				mCells.insert(new WorldCell(mRoadGraph, nodeCycles[0], roadCycles[0]));
				break;
			// 2: divided an existing cell into two
			case 2:
				{
					// NOTE: maybe a copy constructor could be tidier

					// get old cell params
					GrowthGenParams g = alteredCell->getGrowthGenParams();
					// delete old cell
					mCells.erase(alteredCell);
					delete alteredCell;

					// create 2 new cells in place of old cell with old cell params
					WorldCell* wc0 = new WorldCell(mRoadGraph, nodeCycles[0], roadCycles[0]); 
					WorldCell* wc1 = new WorldCell(mRoadGraph, nodeCycles[1], roadCycles[1]);
					wc0->setGrowthGenParams(g);
					wc1->setGrowthGenParams(g);
					mCells.insert(wc0);
					mCells.insert(wc1);
				}
				break;
			default:
				new Exception(Exception::ERR_INTERNAL_ERROR, "What how many new roadCycles was that", "createRoad");
				break;
			}
		}
		else
		{
			// road is probably a filament
			set<WorldCell*>::iterator cellIt, cellEnd;
			for(cellIt = mCells.begin(), cellEnd = mCells.end(); cellIt != cellEnd; cellIt++)
			{
				if((*cellIt)->isInside(wr->getSrcNode()->getPosition2D())) 
				{
					if((*cellIt)->isInside(wr->getDstNode()->getPosition2D()) || (*cellIt)->isOnBoundary(wr->getDstNode()))
						(*cellIt)->addFilament(wr);
				}
				else if((*cellIt)->isInside(wr->getDstNode()->getPosition2D())) 
				{
					if((*cellIt)->isInside(wr->getSrcNode()->getPosition2D()) || (*cellIt)->isOnBoundary(wr->getSrcNode()))
						(*cellIt)->addFilament(wr);
				}
			}
		}
		return wr;
	}
	return 0;
}

void WorldFrame::deleteRoad(WorldRoad* wr)
{
	modify(true);

	// get cells attached to this road
	vector<WorldCell*> attachedCells;
	vector<WorldObject*> attachments(wr->getAllAttachments());
	vector<WorldObject*>::iterator aIt, aEnd;
	for(aIt = attachments.begin(), aEnd = attachments.end(); aIt != aEnd; aIt++)
	{
		// if attachment is a cell
		if(typeid(*(*aIt)) == typeid(WorldCell))
		{
			attachedCells.push_back(static_cast<WorldCell*>(*aIt));
		}
	}

	switch(attachedCells.size())
	{
	case 0:
		break;
	case 1:
		{
			// could be a boundary edge or a filament
			const vector<RoadInterface*> &boundary(attachedCells[0]->getBoundary());
			size_t i;
			for(i=0; i<boundary.size(); i++)
				if(boundary[i] == wr) break;

			if(i<boundary.size())
			{
				mCells.erase(attachedCells[0]);
				delete attachedCells[0];
			}
			else
			{
				attachedCells[0]->removeFilament(wr);
			}
		}
		break;
	case 2:
		{
			// should favour one - can do a vector swap to set preference
			// maybe prefer the cell with largest area

			// get common road
			RoadInterface* ri = 0;
			vector<RoadInterface*> roads0 = attachedCells[0]->getBoundary();
			vector<RoadInterface*> roads1 = attachedCells[1]->getBoundary();
			vector<RoadInterface*>::iterator rIt0, rIt1, rEnd0, rEnd1;
			for(rIt0 = roads0.begin(), rEnd0 = roads0.end(); rIt0 != rEnd0; rIt0++)
			{
				//rIt1 = roads1.find(*rIt0);
				for(rIt1 = roads1.begin(), rEnd1 = roads1.end(); rIt1 != rEnd1; rIt1++)
				{
					if((*rIt1) == (*rIt0)) 
						break;
				}
				//if(rIt1 != roads1.end())
				if(rIt1 != rEnd1)
				{
					ri = (*rIt1);
					break;
				}
			}

			// check common road is wr
			assert(static_cast<WorldRoad*>(ri) == wr);

			// remove wr from boundary 1
			roads1.erase(rIt1);

			// union bounary and filaments
			roads0.insert(rIt0, roads1.begin(), roads1.end());

			// remove wr from boundary 0
			roads0.erase(rIt0);
			
			// delete extraneous cell
			mCells.erase(attachedCells[1]);
			delete attachedCells[1];

			// update cell
			attachedCells[0]->setBoundary(roads0);
		}
		break;
	default:
		new Exception(Exception::ERR_INTERNAL_ERROR, "What how many new cells have you got", "deleteRoad");
		break;
	}

	WorldNode *src = static_cast<WorldNode*>(wr->getSrcNode());
	WorldNode *dst = static_cast<WorldNode*>(wr->getDstNode());
	mSimpleRoadGraph.removeRoad(src->mSimpleNodeId, dst->mSimpleNodeId);
	mSceneRoadMap.erase(wr->getSceneNode());
	delete wr;
}

void WorldFrame::onNewDoc()
{
	mHighlightedNode = 0;
	mSelectedNode = 0;
	mIntersectionPresent = false;

	
	onCloseDoc();
	createScene();
	mTools[mActiveTool]->activate();
	update();
	isDocOpen = true;
}

void WorldFrame::onCloseDoc()
{
	if(isDocOpen)
	{
		isDocOpen = false;
		mTools[mActiveTool]->deactivate();
		
		// destroy scene data
		destroyScene();

		// destroy graph data
		mSimpleRoadGraph.clear();
		mRoadGraph.clear();

		update();
	}
}

bool WorldFrame::plotPointOnTerrain(Real x, Real &y, Real z)
{
	//create scene doc using doc.x -> x, doc.y -> z  and y from ray cast
	Ray verticalRay(Vector3(x, 5000.0f, z), Vector3::NEGATIVE_UNIT_Y );
	mRaySceneQuery->setRay(verticalRay);
	RaySceneQueryResult result = mRaySceneQuery->execute();

	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++ )
	{
		if(itr->worldFragment)
		{
			y = itr->worldFragment->singleIntersection.y;
			return true;
		}
	}
	return false;
}

void WorldFrame::modify(bool b)
{
	static_cast<MainWindow*>(GetParent())->modify(b);
}

template<> WorldFrame* Ogre::Singleton<WorldFrame>::ms_Singleton = 0;
WorldFrame* WorldFrame::getSingletonPtr(void)
{
    return ms_Singleton;
}
WorldFrame& WorldFrame::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}
