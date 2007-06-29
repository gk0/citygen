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
#include "ToolRoadSelect.h"
#include "ToolCellSelect.h"

#ifdef __WXGTK__
#include <gdk/gdk.h>
#include <gtk/gtk.h> 
#include <gdk/gdkx.h>
#include <wx/gtk/win_gtk.h>
#include <GL/glx.h> 
#endif


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

	mSelectedNode = 0;
	mSelectedRoad = 0;
	mSelectedCell = 0;

	//init();
	//toggleTimerRendering(); // only really to test fps
}

void WorldFrame::init()
{
	// --------------------
	// Create a new parameters list according to compiled OS
	NameValuePairList params;
	String handle;
#ifdef __WXMSW__
	handle = StringConverter::toString((size_t)((HWND)GetHandle()));
#elif defined(__WXGTK__)

	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	GtkWidget* privHandle = m_wxwindow;
	// prevents flickering
	gtk_widget_set_double_buffered(privHandle, FALSE);
	// grab the window object
	GdkWindow* gdkWin = GTK_PIZZA(privHandle)->bin_window;
	Display* display = GDK_WINDOW_XDISPLAY(gdkWin);
	Window wid = GDK_WINDOW_XWINDOW(gdkWin);
	
	std::stringstream str;
	
	// display
	str << (unsigned long)display << ':';
	
	// screen (returns "display.screen")
	std::string screenStr = DisplayString(display);
	std::string::size_type dotPos = screenStr.find(".");
	screenStr = screenStr.substr(dotPos+1, screenStr.size());
	str << screenStr << ':';
	
	// XID
	str << wid << ':';
	
	// retrieve XVisualInfo
	int attrlist[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, None };
	XVisualInfo* vi = glXChooseVisual(display, DefaultScreen(display), attrlist);
	str << (unsigned long)vi;
	
	handle = str.str();
#else
	#error Not supported on this platform.
#endif
	params["externalWindowHandle"] = handle;

	// Get wx control window size
	int width, height;
	GetSize(&width, &height);

	// Create the render window
	mRenderWindow = Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);
	mRenderWindow->setActive(true);

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

	mToolsetMode = MainWindow::view;
	mActiveTool = MainWindow::addNode;

	mTools.push_back(new ToolView(this));
	mTools.push_back(new ToolNodeSelect(this));
	mTools.push_back(new ToolNodeAdd(this, mSceneManager, mRoadGraph, mSimpleRoadGraph));
	mTools.push_back(new ToolNodeDelete(this));
	mTools.push_back(new ToolRoadSelect(this));
	mTools.push_back(new ToolCellSelect(this));
	mActiveTool = MainWindow::viewTool;
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
	mCamera->setNearClipDistance(1);
    mCamera->setFarClipDistance(1000);
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
	mSceneManager->setFog(FOG_LINEAR, fadeColour, .001f, 500, 1000);
	mRenderWindow->getViewport(0)->setBackgroundColour(fadeColour);

    string terrain_cfg("terrain.cfg");
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        terrain_cfg = mResourcePath + terrain_cfg;
#endif
    mSceneManager -> setWorldGeometry(terrain_cfg);

	//mWorldTerrain.load();

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

	if(mToolsetMode == MainWindow::view)
		updateProperties();
	//stringstream oss;
	//oss <<"Camera position: "<<mCamera->getPosition();
	//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
}

// Rotates the view
void WorldFrame::cameraRotate(Real yaw, Real pitch)
{
	modify(true);
	mCamera->yaw(yaw * (mCamera->getFOVy() / mCamera->getAspectRatio() / 320.0f));
	mCamera->pitch(pitch * (mCamera->getFOVy() / 240.0f));


	if(mToolsetMode == MainWindow::view)
		updateProperties();
	//stringstream oss;
	//oss <<"Camera orientation: " << mCamera->getOrientation();
	//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
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
	if(!mRenderWindow) return;
	
	// Setting new size;
	int width;
	int height;
	GetSize(&width, &height);
	mRenderWindow->resize(width, height);
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
	vector<TiXmlElement*> edgeElements;

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
			edgeElements.push_back(pElem);
		}
		else if(key == "cells") 
			break;
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
			if(!mSimpleRoadGraph.testRoad(sourceIt->second->mSimpleNodeId, targetIt->second->mSimpleNodeId))
			{
				// create the road in the scene
				WorldRoad* wr = new WorldRoad(sourceIt->second, targetIt->second, mRoadGraph, mSimpleRoadGraph, mSceneManager);
				mSceneRoadMap[wr->getSceneNode()] = wr;
				const TiXmlHandle roadRoot(edgeElements[i]);
				wr->loadXML(roadRoot);
			}
		}
	}

	pElem=worldRoot.FirstChild("cells").FirstChild().Element();
	for(; pElem; pElem=pElem->NextSiblingElement())
	{
		string key = pElem->Value();
		if(key=="cell")
		{
			vector<NodeInterface*> nodeCycle;

			//load the cycle
			TiXmlElement* pElem2=pElem->FirstChild("cycle")->FirstChildElement();
			for(; pElem2; pElem2=pElem2->NextSiblingElement())
			{
				key = pElem2->Value();
				if(key=="node")
				{
					string strId = pElem2->Attribute("id");
					nodeCycle.push_back(nodeIdTranslation[strId]);
				}
			}

			if(nodeCycle.size() > 2)
			{
				WorldCell* wc = new WorldCell(mRoadGraph, mSimpleRoadGraph, nodeCycle);
				mSceneCellMap[wc->getSceneNode()] = wc;
				mCells.insert(wc);
				const TiXmlHandle cellRoot(pElem);
				wc->loadXML(pElem);
			}
		}
	}
		

	update();
	return true;
}

TiXmlElement* WorldFrame::saveXML()
{
	TiXmlElement * root = new TiXmlElement("WorldDocument");  

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
	root->LinkEndChild(roadNetwork);

	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mSimpleRoadGraph.getNodes(); nIt != nEnd; nIt++) 
	{
		NodeInterface* ni = mSimpleRoadGraph.getNode(*nIt);
		Vector2 loc(ni->getPosition2D());
		//string id(pointerToString(*vi));
		//string id
	
		TiXmlElement * node;
		node = new TiXmlElement("node");  
		node->SetAttribute("id", (int) ni);
		node->SetDoubleAttribute("x", loc.x);
		node->SetDoubleAttribute("y", loc.y);
		roadNetwork->LinkEndChild(node);
	}

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = mSimpleRoadGraph.getRoads(); rIt != rEnd; rIt++) 
	{
		WorldRoad* wr = static_cast<WorldRoad*>(mSimpleRoadGraph.getRoad(*rIt));
		roadNetwork->LinkEndChild(wr->saveXML());
	}

	TiXmlElement *cells = new TiXmlElement("cells"); 
	cells->SetAttribute("id", "cellset0");
	root->LinkEndChild(cells);

	set<WorldCell*>::iterator cIt,cEnd;
	for(cIt = mCells.begin(), cEnd = mCells.end(); cIt != cEnd; cIt++)
	{
		cells->LinkEndChild((*cIt)->saveXML());
	}
	return root;
}

void WorldFrame::setToolsetMode(MainWindow::ToolsetMode mode) 
{
	/*
	switch(mode) {
	case MainWindow::view:
		endNodeMode();
		break;
	case MainWindow::node:
		beginNodeMode();
		break;
	case MainWindow::road:
		endNodeMode();
		selectRoad(0);
		break;
	case MainWindow::cell:
		endNodeMode();
		selectCell(0);
		break;
	}*/
	selectNode(0);
	selectRoad(0);
	selectCell(0);
	mToolsetMode = mode;
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
	updateProperties();
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
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
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
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if (itr->movable && itr->movable->getName().substr(0, 4) == "node")
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

	modify(true);

	// get road nodes
	WorldNode* wn1 = static_cast<WorldNode*>(wr->getSrcNode());
	WorldNode* wn2 = static_cast<WorldNode*>(wr->getDstNode());

	// get cells attached to this road
	vector<WorldCell*> attachedCells;
	vector< vector<NodeInterface*> > boundaries;

	set<WorldObject*> attachments(wr->getAllAttachments());
	set<WorldObject*>::iterator aIt, aEnd;
	for(aIt = attachments.begin(), aEnd = attachments.end(); aIt != aEnd; aIt++)
	{
		// if attachment is a cell
		if(typeid(*(*aIt)) == typeid(WorldCell))
		{
			WorldCell* wc = static_cast<WorldCell*>(*aIt);

			// get cell ptr
			attachedCells.push_back(wc);

			// get cell boundary data 
			boundaries.push_back(wc->getBoundaryCycle());

			// clear cell graph, remember we are messing with its data
			wc->clear();
		}
	}

	// delete road node
	mSceneRoadMap.erase(wr->getSceneNode());
	delete wr;

	// create replacement roads
	WorldRoad* wr1 = new WorldRoad(wn1, wn, mRoadGraph, mSimpleRoadGraph, mSceneManager);
	mSceneRoadMap[wr1->getSceneNode()] = wr1;
	WorldRoad* wr2 = new WorldRoad(wn, wn2, mRoadGraph, mSimpleRoadGraph, mSceneManager);
	mSceneRoadMap[wr2->getSceneNode()] = wr2;

	// update cell boundaries
	size_t numOfCells = attachedCells.size();
	for(size_t i=0; i<numOfCells; i++)
	{
		// insert new node into boundary cycle
		size_t j,k,N = boundaries[i].size();
		for(j=0; j<N; j++)
		{
			k = (j+1) % N;
			if((boundaries[i][j] == wn1 && boundaries[i][k] == wn2) 
				|| (boundaries[i][j] == wn2 && boundaries[i][k] == wn1))
			{
				boundaries[i].insert(boundaries[i].begin() + k, wn);
				break;
			}
		}
		// set boundary
		attachedCells[i]->setBoundary(boundaries[i]);
	}
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
				{
					WorldCell* wc = new WorldCell(mRoadGraph, mSimpleRoadGraph, nodeCycles[0], roadCycles[0]);
					mSceneCellMap[wc->getSceneNode()] = wc;
					mCells.insert(wc);
				}
				break;
			// 2: divided an existing cell into two
			case 2:
				{
					// NOTE: maybe a copy constructor could be tidier

					// get old cell params
					GrowthGenParams g = alteredCell->getGenParams();
					// delete old cell
					mCells.erase(alteredCell);
					mSceneCellMap.erase(alteredCell->getSceneNode());
					delete alteredCell;


					// create 2 new cells in place of old cell with old cell params
					WorldCell* wc0 = new WorldCell(mRoadGraph, mSimpleRoadGraph, nodeCycles[0], roadCycles[0]); 
					mSceneCellMap[wc0->getSceneNode()] = wc0;
					WorldCell* wc1 = new WorldCell(mRoadGraph, mSimpleRoadGraph, nodeCycles[1], roadCycles[1]);
					mSceneCellMap[wc1->getSceneNode()] = wc1;

					wc0->setGenParams(g);
					wc1->setGenParams(g);
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
	vector<WorldCell*> aCells;
	set<WorldObject*> attachments(wr->getAllAttachments());
	set<WorldObject*>::iterator aIt, aEnd;
	for(aIt = attachments.begin(), aEnd = attachments.end(); aIt != aEnd; aIt++)
	{
		// if attachment is a cell
		if(typeid(*(*aIt)) == typeid(WorldCell))
		{
			aCells.push_back(static_cast<WorldCell*>(*aIt));
		}
	}

	switch(aCells.size())
	{
	case 0:
		mSceneRoadMap.erase(wr->getSceneNode());
		delete wr;
		break;
	case 1:
		{
			// could be a boundary edge or a filament
			const vector<RoadInterface*> &boundary(aCells[0]->getBoundaryRoads());
			size_t i;
			for(i=0; i<boundary.size(); i++)
				if(boundary[i] == wr) break;

			// if found on the boundary cycle
			if(i<boundary.size())
			{
				mSceneCellMap.erase(aCells[0]->getSceneNode());
				mCells.erase(aCells[0]);
				delete aCells[0];
			}
			else
			{
				aCells[0]->removeFilament(wr);
			}

			// delete road
			mSceneRoadMap.erase(wr->getSceneNode());
			delete wr;
		}
		break;
	case 2:
		{
			// should favour one - can do a vector swap to set preference

			// save params from the biggest attached cell by area
			GrowthGenParams gp = aCells[0]->calcArea2D() > aCells[1]->calcArea2D() ? 
								aCells[0]->getGenParams() : aCells[1]->getGenParams();

			// delete cells
			BOOST_FOREACH(WorldCell* c, aCells)
			{
				mSceneCellMap.erase(c->getSceneNode());
				mCells.erase(c);
				delete c;
			}

			// delete road
			mSceneRoadMap.erase(wr->getSceneNode());
			delete wr;

			// run cell decomposition
			vector< vector<NodeInterface*> > nodeCycles;
			vector< vector<RoadInterface*> > roadCycles;
			vector<RoadInterface*> filaments;
			mSimpleRoadGraph.extractPrimitives(filaments, nodeCycles, roadCycles);

			// find the new cell if there is one
			bool newCell = true;
			for(size_t i = 0; i < roadCycles.size(); i++)
			{
				BOOST_FOREACH(WorldCell* c, mCells)
				{
					if(c->compareBoundary(roadCycles[i]))
					{
						newCell = false;
						break;
					}
				}
				// create the new cell and break
				if(newCell)
				{
					WorldCell* wc = new WorldCell(mRoadGraph, mSimpleRoadGraph, nodeCycles[i], roadCycles[i]);
					mSceneCellMap[wc->getSceneNode()] = wc;
					mCells.insert(wc);
					wc->setGenParams(gp);
					break;
				}
			}
		}
		break;
	default:
		new Exception(Exception::ERR_INTERNAL_ERROR, "What how many new cells have you got", "deleteRoad");
		break;
	}

}

void WorldFrame::onNewDoc()
{
	mHighlightedNode = 0;
	mSelectedNode = 0;
	mSelectedRoad = 0;
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
	Ray verticalRay(Vector3(x, 5000.0f, z), Vector3::NEGATIVE_UNIT_Y);
	mRaySceneQuery->setRay(verticalRay);
	RaySceneQueryResult result = mRaySceneQuery->execute();

	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if(itr->worldFragment)
		{
			y = itr->worldFragment->singleIntersection.y;
			return true;
		}
	}
	return false;
}

bool WorldFrame::plotPointOnTerrain(const Vector2& pos2D, Vector3& pos3D)
{
	Vector3 tmp(pos2D.x, 0, pos2D.y);
	if(plotPointOnTerrain(tmp.x, tmp.y, tmp.z))
	{
		pos3D = tmp;
		return true;
	}
	return false;
}

void WorldFrame::modify(bool b)
{
	static_cast<MainWindow*>(GetParent())->modify(b);
}

void WorldFrame::updateProperties()
{
	static_cast<MainWindow*>(GetParent())->updateProperties();
}

void WorldFrame::moveSelectedNode(const Vector3& pos)
{
	WorldNode* wn = getSelected();
	if(wn)
	{
		Vector2 pos2D(pos.x, pos.z);
		wn->move(pos2D);
		updateProperties();
		update();
	}
}

bool WorldFrame::pickCell(wxMouseEvent& e, WorldCell *&wc)
{
	// Setup the ray scene query
	float mouseX = float(1.0f/mViewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/mViewport->getActualHeight()) * e.GetY();
	Ray mouseRay = mCamera->getCameraToViewportRay(mouseX, mouseY);

	mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setSortByDistance(true);
	RaySceneQueryResult result = mRaySceneQuery->execute();
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if (itr->movable && itr->movable->getName().substr(0, 4) == "cell")
		{
			wc = mSceneCellMap[itr->movable->getParentSceneNode()];
    		return true;
		}
	}
	return false;
}

void WorldFrame::selectCell(WorldCell* wn)
{
	if(mSelectedCell) mSelectedCell->showSelected(false);
	mSelectedCell = wn;
	if(mSelectedCell)
	{
		mSelectedCell->showSelected(true);
	}
	updateProperties();
}

bool WorldFrame::pickRoad(wxMouseEvent& e, WorldRoad *&wr)
{
	// Setup the ray scene query
	float mouseX = float(1.0f/mViewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/mViewport->getActualHeight()) * e.GetY();
	Ray mouseRay = mCamera->getCameraToViewportRay(mouseX, mouseY);

	mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setSortByDistance(true);
	RaySceneQueryResult result = mRaySceneQuery->execute();
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if (itr->movable && itr->movable->getName().substr(0, 4) == "road")
		{
			wr = mSceneRoadMap[itr->movable->getParentSceneNode()];
    		return true;
		}
	}
	return false;
}

void WorldFrame::selectRoad(WorldRoad* wr)
{
	if(mSelectedRoad) mSelectedRoad->showSelected(false);
	mSelectedRoad = wr;
	if(mSelectedRoad)
	{
		mSelectedRoad->showSelected(true);
	}
	updateProperties();
}

void  WorldFrame::setViewMode(MainWindow::ViewMode mode)
{
	bool showRoads, showBuildings;
	switch(mode)
	{
	case MainWindow::view_primary:
		showRoads = false;
		showBuildings = false;
		break;
	case MainWindow::view_cell:
		showRoads = true;
		showBuildings = false;
		break;
	case MainWindow::view_building:
		showRoads = true;
		showBuildings = true;
		break;
	}
	set<WorldCell*>::iterator cIt,cEnd;
	for(cIt = mCells.begin(), cEnd = mCells.end(); cIt != cEnd; cIt++)
	{
		(*cIt)->showRoads(showRoads);
		(*cIt)->showBuildings(showBuildings);
	}
	mViewMode = mode;
	update();
}

template<> WorldFrame* Ogre::Singleton<WorldFrame>::ms_Singleton = 0;
WorldFrame* WorldFrame::getSingletonPtr(void)
{
    return ms_Singleton;
}
WorldFrame& WorldFrame::getSingleton(void)
{  
    assert(ms_Singleton);  return (*ms_Singleton);  
}
