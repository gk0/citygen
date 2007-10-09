// Includes
#include "stdafx.h"
#include "WorldFrame.h"
#include "SimpleNode.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldCell.h"
#include "PerformanceTimer.h"
#include "Geometry.h"
#include "MeshBuilder.h"
#include "Statistics.h"
size_t Statistics::_buildingCount = 0;

//tools
#include "ToolView.h"
#include "ToolNodeSelect.h"
#include "ToolNodeAdd.h"
#include "ToolNodeDelete.h"
#include "ToolRoadSelect.h"
#include "ToolRoadAdd.h"
#include "ToolRoadDelete.h"
#include "ToolCellSelect.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

#ifdef __WXGTK__
#include <gdk/gdk.h>
#include <gtk/gtk.h> 
#include <gdk/gdkx.h>
#include <wx/gtk/win_gtk.h>
#include <GL/glx.h> 
#endif

#define THREADME 1


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
	EVT_LEFT_UP(WorldFrame::OnLeftReleased)
	EVT_MIDDLE_DOWN(WorldFrame::OnMiddlePressed)
	EVT_MIDDLE_UP(WorldFrame::OnMiddleReleased)
	EVT_RIGHT_DOWN(WorldFrame::OnRightPressed)
	EVT_RIGHT_UP(WorldFrame::OnRightReleased)
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
	  _timer(this, ID_RENDERTIMER)
{
	_camera = 0;
	_renderWindow = 0;
	_sceneManager = 0;
	_viewport = 0;

	_selectedNode = 0;
	_selectedRoad = 0;
	_selectedCell = 0;

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
	_renderWindow = Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);
	_renderWindow->setActive(true);

	// Create the SceneManager, in this case the terrainscenemanager
    _sceneManager = Root::getSingleton().createSceneManager("TerrainSceneManager");
    createCamera();
    createViewport();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);
	
	// Desperate attempt to improve image quality on linux w/fglrx
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(8);

	// Make sure assets are loaded before we create the scene
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	//createScene();
	_isDocOpen = false;

	_toolsetMode = MainWindow::view;
	_activeTool = MainWindow::addNode;

	_tools.push_back(new ToolView(this));
	_tools.push_back(new ToolNodeSelect(this));
	_tools.push_back(new ToolNodeAdd(this, _sceneManager, _roadGraph, _simpleRoadGraph));
	_tools.push_back(new ToolNodeDelete(this));
	_tools.push_back(new ToolRoadSelect(this));
	_tools.push_back(new ToolRoadAdd(this, _sceneManager, _roadGraph, _simpleRoadGraph));
	_tools.push_back(new ToolRoadDelete(this));
	_tools.push_back(new ToolCellSelect(this));
	_activeTool = MainWindow::viewTool;
}


// Destructor //
WorldFrame::~WorldFrame()
{
	// destroy scene objects
	if(_isDocOpen)
	{
		_tools[_activeTool]->deactivate();
		destroyScene();
	}
	destroyViewport();
	destroyCamera();
	
	// destroy scene manager
	Root::getSingleton().destroySceneManager(_sceneManager);

	// destroy render window
	Root::getSingleton().detachRenderTarget(_renderWindow);
	delete _renderWindow;
	_renderWindow = 0;

	// delete tools
	vector<Tool*>::iterator tIt, tEnd;
	for(uint32 i=0; i<_tools.size(); i++) 
		delete _tools[i];
}

void WorldFrame::destroyCamera() 
{
	if (_camera)
	{
		_sceneManager->destroyCamera(_camera);
		_camera = 0;
	}
}

void WorldFrame::destroyViewport() 
{
	if (_viewport)
	{
		_renderWindow->removeViewport(_viewport->getZOrder());
		_viewport = 0;
	}
}

void WorldFrame::createCamera(void)
{   
	// Create the camera
    _camera = _sceneManager->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    _camera->setPosition(50, 0, 0);
    //_camera->setOrientation(Quaternion(-0.49f, 0.17f, 0.81f, 0.31f));
	//Quaternion q(&v);
	//_camera->setOrientation(q);
	_camera->lookAt(1041.25f, 0.1f, 965.25f);
	_camera->setNearClipDistance(1);
    _camera->setFarClipDistance(1000);
}


void WorldFrame::createScene(void) 
{	
	// First check that vertex programs and dot3 or fragment programs are supported
	const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
	if (!caps->hasCapability(RSC_VERTEX_PROGRAM))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support vertex programs, so cannot "
			"run this demo. Sorry!", 
			"Dot3Bump::createScene");
	}
	if (!(caps->hasCapability(RSC_FRAGMENT_PROGRAM) 
		|| caps->hasCapability(RSC_DOT3)) )
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support dot3 blending or fragment programs, so cannot "
			"run this demo. Sorry!", 
			"Dot3Bump::createScene");
	}

    // Set ambient light
    _sceneManager->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Light* l = _sceneManager->createLight("MainLight");
	// Accept default settings: point light, white diffuse, just set position
	// NB I could attach the light to a SceneNode if I wanted it to move automatically with
	//  other objects, but I don't
	l->setPosition(20,180,50);

    // Fog
    // NB it's VERY important to set this before calling setWorldGeometry 
    // because the vertex program picked will be different
	ColourValue fadeColour(0.76f, 0.86f, 0.93f);
	//_sceneManager->setFog(FOG_LINEAR, fadeColour, .001f, 500, 1000);
	_renderWindow->getViewport(0)->setBackgroundColour(fadeColour);

    string terrain_cfg("terrain.cfg");
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        terrain_cfg = mResourcePath + terrain_cfg;
#endif
    _sceneManager -> setWorldGeometry(terrain_cfg);

	//mWorldTerrain.load();

    // Infinite far plane?
    if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
    {
        _camera->setFarClipDistance(0);
    }
	_camera->setNearClipDistance(0.1);

    // Define the required sky plane
    Plane plane;
    // 5000 world units from the camera
    plane.d = 5000;
    // Above the camera, facing down
    plane.normal = -Vector3::UNIT_Y;

	// Create our ray query
	_raySceneQuery = _sceneManager->createRayQuery(Ray());

	// Set up the camera
	// Create the camera node	
	_cameraNode = _sceneManager->createSceneNode("cameraNode");
	_cameraNode->setPosition(1041.25f, 0.1f, 965.25f);
	_cameraNode->attachObject(_camera);
	_camera->setPosition(177, 0, 0);
	_camera->lookAt(1041.25f, 0.1f, 965.25f);
	_cameraNode->setOrientation(-0.267156, -0.236617, -0.931684, -0.067849);
}

void WorldFrame::createViewport(void)
{
	if(_viewport) destroyViewport();
    // Create one view port, entire window
    _viewport = _renderWindow->addViewport(_camera);
    _viewport->setBackgroundColour(ColourValue(0.5f,0.5f,0.5f));

    // Alter the camera aspect ratio to match the view port
    _camera->setAspectRatio(
        Real(_viewport->getActualWidth()) / Real(_viewport->getActualHeight()));
}

void WorldFrame::destroyScene(void)
{	
	_renderWindow->getViewport(0)->setBackgroundColour(ColourValue(0.5f, 0.5f, 0.5f));

	// destroy ray
	delete _raySceneQuery;

	// Remove Cells
	set<WorldCell*>::iterator cIt, cEnd;
	for(cIt = _cellSet.begin(), cEnd = _cellSet.end(); cIt != cEnd; cIt++)
		delete (*cIt);
	_cellSet.clear();

	// Remove roads from the scene
	map<SceneNode*, WorldRoad*>::const_iterator ri, rend;
	for (ri = _sceneRoadMap.begin(), rend = _sceneRoadMap.end(); ri != rend; ++ri)
	{
		delete ri->second;
	}
	_sceneRoadMap.clear();

	// Remove nodes from the scene
	map<SceneNode*, WorldNode*>::const_iterator ni, nend;
	for (ni = _sceneNodeMap.begin(), nend = _sceneNodeMap.end(); ni != nend; ++ni)
	{
		delete ni->second;
	}
	_sceneNodeMap.clear();

	//if(_cameraNode)
	//{
	//	_cameraNode->detachAllObjects();
	//	delete _cameraNode;
	//	_cameraNode = 0;
	//}
	//_sceneManager->destroyAllManualObjects();
	//_sceneManager->destroyAllEntities();
	//_sceneManager->destroyAllInstancedGeometry();
	try
	{
		_sceneManager->clearScene();
	}
	catch(Ogre::Exception &e)
	{
		LogManager::getSingleton().logMessage(e.getFullDescription());
	}
	catch(std::exception &e)
	{
		LogManager::getSingleton().logMessage(e.what());
	}

	// reset counts
	WorldNode::resetInstanceCount();
	WorldRoad::resetInstanceCount();
	WorldCell::resetInstanceCount();
}

// Moves the view
void WorldFrame::cameraMove(Real x, Real y, Real z)
{
	modify(true);
	_camera->moveRelative(Vector3(x, y, z));

	if(_toolsetMode == MainWindow::view)
		updateProperties();
}

// Rotates the view
void WorldFrame::cameraRotate(Real yaw, Real pitch)
{
	modify(true);
	_camera->yaw(yaw * (_camera->getFOVy() / _camera->getAspectRatio() / 320.0f));
	_camera->pitch(pitch * (_camera->getFOVy() / 240.0f));


	if(_toolsetMode == MainWindow::view)
		updateProperties();
}

void WorldFrame::OnChar(wxKeyEvent& e)
{
	if(!_isDocOpen) return;
	_tools[_activeTool]->OnChar(e);
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
	if(!_isDocOpen) return;
	// if you click on me get back focus
	// focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();

	_tools[_activeTool]->OnLeftPressed(e);
}

void WorldFrame::OnLeftReleased(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	_tools[_activeTool]->OnLeftReleased(e);
}

void WorldFrame::OnMiddlePressed(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	// if you click on me get back focus
	// focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();

	_tools[_activeTool]->OnMiddlePressed(e);
}

void WorldFrame::OnMiddleReleased(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	_tools[_activeTool]->OnMiddleReleased(e);
}

void WorldFrame::OnRightPressed(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	// if you click on me get back focus
	// focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();
	_tools[_activeTool]->OnRightPressed(e);
}

void WorldFrame::OnRightReleased(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	_tools[_activeTool]->OnRightReleased(e);
}


void WorldFrame::OnMouseMove(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	_tools[_activeTool]->OnMouseMove(e);
}

void WorldFrame::OnMouseWheel(wxMouseEvent &e)
{
	if(!_isDocOpen) return;
	_tools[_activeTool]->OnMouseWheel(e);
}

void WorldFrame::OnPaint(wxPaintEvent &WXUNUSED(e))
{
	wxPaintDC dc(this);
	update();
}

void WorldFrame::OnSize(wxSizeEvent &e)
{
	if(!_renderWindow) return;
	
	// Setting new size;
	int width;
	int height;
	GetSize(&width, &height);
	_renderWindow->resize(width, height);
	// Letting Ogre know the window has been resized;
	_renderWindow->windowMovedOrResized();
	// Set the aspect ratio for the new size;
	if (_camera)
		_camera->setAspectRatio(Real(width) / Real(height));

	update();
}


void WorldFrame::OnTimer(wxTimerEvent &e)
{
	update();
}

void WorldFrame::toggleTimerRendering()
{
	// toggle start/stop
	if (_timer.IsRunning())
		_timer.Stop();
	_timer.Start(10);
}


boost::mutex cit_mutex;

void prebuild(pair< set<WorldCell*>::iterator, set<WorldCell*>::iterator > *cIt)
{
	while(true)
	{	
		WorldCell* wc = 0;
		{
			boost::mutex::scoped_lock lock(cit_mutex);
			if(cIt->first != cIt->second)
			{
				wc = *(cIt->first);
				cIt->first++;
			}
			else break;
		}

		if(wc && !wc->isValid())
			/*DEBUG: for(size_t i=0; i<100; i++)*/ wc->prebuild(); //do it 100 times
	}
}


void WorldFrame::update()
{
	//wxControl::update();
	Statistics::resetBuildingCount();

	// render nodes
	map< Ogre::SceneNode*, WorldNode* >::iterator nIt, nEnd;
	for(nIt = _sceneNodeMap.begin(), nEnd = _sceneNodeMap.end(); nIt != nEnd; nIt++)
		nIt->second->validate();

	// render roads
	map< Ogre::SceneNode*, WorldRoad* >::iterator rIt, rEnd;
	for(rIt = _sceneRoadMap.begin(), rEnd = _sceneRoadMap.end(); rIt != rEnd; rIt++)
		rIt->second->validate();

	// render cells
	//map< Ogre::SceneNode*, WorldCell* >::iterator cIt, cEnd;
	set<WorldCell*>::iterator cIt, cEnd;

	//PerformanceTimer genPT("Generate");

	// NON THREADED
	//pair<set<WorldCell*>::iterator, set<WorldCell*>::iterator> cPIt;
	//cPIt.first = mCells.begin();
	//cPIt.second = mCells.end();
	//prebuild2(cPIt);

	//THREADED
	//
	pair<set<WorldCell*>::iterator, set<WorldCell*>::iterator> cPIt;
	cPIt.first = _cellSet.begin();
	cPIt.second = _cellSet.end();

#ifdef THREADME
	boost::thread thrd1(
		boost::bind(&prebuild, &cPIt));
	boost::thread thrd2(
		boost::bind(&prebuild, &cPIt));

	thrd1.join();
	thrd2.join();
#else
	prebuild(&cPIt);
#endif

	//genPT.stop();

	try {

	//PerformanceTimer buildPT("Build");
	for(cIt = _cellSet.begin(), cEnd = _cellSet.end(); cIt != cEnd; cIt++)
	{
		(*cIt)->validate();
	}
	//buildPT.stop();
	//LogManager::getSingleton().logMessage(genPT.toString()+" "+buildPT.toString());

	}catch(...)
	{
	}
	//LogManager::getSingleton().logMessage("Building Count: "+StringConverter::toString(Statistics::getBuildingCount()));

	if(_camera) Root::getSingleton().renderOneFrame();
}

bool WorldFrame::loadXML(const TiXmlHandle& worldRoot)
{
	//
	onCloseDoc();
	onNewDoc();

	// Load Camera
	string camId = worldRoot.FirstChild("camera").Element()->Attribute("id");
	if(camId == "edit_cam")
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
				_camera->setPosition(x, y, z);
			}
			else if(key == "camOrientation")
			{
				Quaternion nodeOrient;
				cameraChild->QueryFloatAttribute("w", &nodeOrient.w);
				cameraChild->QueryFloatAttribute("x", &nodeOrient.x);
				cameraChild->QueryFloatAttribute("y", &nodeOrient.y);
				cameraChild->QueryFloatAttribute("z", &nodeOrient.z);
				_cameraNode->setOrientation(nodeOrient);
			}
			else if(key == "nodePosition")
			{
				Real x, y, z;
				cameraChild->QueryFloatAttribute("x", &x);
				cameraChild->QueryFloatAttribute("y", &y);
				cameraChild->QueryFloatAttribute("z", &z);
				_cameraNode->setPosition(x, y, z);
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
			if(!_simpleRoadGraph.testRoad(sourceIt->second->mSimpleNodeId, 
				targetIt->second->mSimpleNodeId))
			{
				// create the road in the scene
				WorldRoad* wr = new WorldRoad(sourceIt->second, targetIt->second, 
										_roadGraph, _simpleRoadGraph, _sceneManager);
				_sceneRoadMap[wr->getSceneNode()] = wr;
				const TiXmlHandle roadRoot(edgeElements[i]);
				wr->loadXML(roadRoot);
			}
		}
	}

	pElem=worldRoot.FirstChild("cells").FirstChild().Element();
	for(; pElem; pElem=pElem->NextSiblingElement())
	{
		if(string(pElem->Value())=="cell")
		{
			vector<NodeInterface*> nodeCycle;

			//load the cycle
			TiXmlElement* pElem2=pElem->FirstChild("cycle")->FirstChildElement();
			for(; pElem2; pElem2=pElem2->NextSiblingElement())
			{
				if(string(pElem2->Value())=="node")
				{
					string strId = pElem2->Attribute("id");
					nodeCycle.push_back(nodeIdTranslation[strId]);
				}
			}

			//load the filaments
			vector<WorldRoad*> filaments;
			TiXmlNode* f = pElem->FirstChild("filaments");
			if(f) pElem2 = f->FirstChildElement();
			for(; pElem2; pElem2=pElem2->NextSiblingElement())
			{
				if(string(pElem2->Value())=="filament")
				{
					string srcId, dstId;
					TiXmlElement* pElem3=pElem2->FirstChildElement();

					if(string(pElem3->Value()) == "node") 
						srcId = pElem3->Attribute("id");
					pElem3 = pElem3->NextSiblingElement();
					if(string(pElem3->Value()) == "node") 
						dstId = pElem3->Attribute("id");
					
					filaments.push_back(getWorldRoad(nodeIdTranslation[srcId], nodeIdTranslation[dstId]));
				}
			}

			if(nodeCycle.size() > 2)
			{
				WorldCell* wc = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycle);
				_sceneCellMap[wc->getSceneNode()] = wc;
				_cellSet.insert(wc);
				BOOST_FOREACH(WorldRoad* wr, filaments) wc->addFilament(wr);
				const TiXmlHandle cellRoot(pElem);
				wc->loadXML(pElem);
			}
		}
	}
		
	PerformanceTimer pt("Total Generate Time");
	update();
	pt.stop();
	LogManager::getSingleton().logMessage(pt.toString());
	return true;
}

TiXmlElement* WorldFrame::saveXML()
{
	TiXmlElement * root = new TiXmlElement("WorldDocument");  

	// Save Camera
	{
		TiXmlElement *camera = new TiXmlElement("camera"); 
		camera->SetAttribute("id", "edit_cam");
		root->LinkEndChild(camera); 

		const Vector3 &camPos(_camera->getPosition());
		TiXmlElement *position = new TiXmlElement("position");
		position->SetDoubleAttribute("x", camPos.x);
		position->SetDoubleAttribute("y", camPos.y);
		position->SetDoubleAttribute("z", camPos.z);
		camera->LinkEndChild(position);

		Quaternion camOrient = _cameraNode->getOrientation();
		TiXmlElement *direction = new TiXmlElement("camOrientation");
		direction->SetDoubleAttribute("w", camOrient.w);
		direction->SetDoubleAttribute("x", camOrient.x);
		direction->SetDoubleAttribute("y", camOrient.y);
		direction->SetDoubleAttribute("z", camOrient.z);
		camera->LinkEndChild(direction);

		const Vector3 &camNodePos(_cameraNode->getPosition());
		TiXmlElement *position2 = new TiXmlElement("nodePosition");
		position2->SetDoubleAttribute("x", camNodePos.x);
		position2->SetDoubleAttribute("y", camNodePos.y);
		position2->SetDoubleAttribute("z", camNodePos.z);
		camera->LinkEndChild(position2);
	}

	//<graph id="roadgraph" edgedefault="undirected">
	TiXmlElement *roadNetwork = new TiXmlElement("graph"); 
	roadNetwork->SetAttribute("id", "roadgraph");
	roadNetwork->SetAttribute("edgedefault", "undirected");
	root->LinkEndChild(roadNetwork);

	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = _simpleRoadGraph.getNodes(); nIt != nEnd; nIt++) 
	{
		NodeInterface* ni = _simpleRoadGraph.getNode(*nIt);
		Vector2 loc(ni->getPosition2D());
		//string id(pointerToString(*vi));
		//string id
	
		TiXmlElement * node;
		node = new TiXmlElement("node");  
		node->SetAttribute("id", (int) ni);
		node->SetDoubleAttribute("x", loc.x);
		node->SetDoubleAttribute("y", loc.y);
		node->SetAttribute("label", static_cast<WorldNode*>(ni)->getLabel().c_str());
		roadNetwork->LinkEndChild(node);
	}

	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = _simpleRoadGraph.getRoads(); rIt != rEnd; rIt++) 
	{
		WorldRoad* wr = static_cast<WorldRoad*>(_simpleRoadGraph.getRoad(*rIt));
		roadNetwork->LinkEndChild(wr->saveXML());
	}

	TiXmlElement *cells = new TiXmlElement("cells"); 
	cells->SetAttribute("id", "cellset0");
	root->LinkEndChild(cells);

	set<WorldCell*>::iterator cIt,cEnd;
	for(cIt = _cellSet.begin(), cEnd = _cellSet.end(); cIt != cEnd; cIt++)
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
	_toolsetMode = mode;
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
	if(_selectedNode) _selectedNode->showSelected(false);
	_selectedNode = wn;
	if(_selectedNode)
	{
		_selectedNode->showSelected(true);
	}
	updateProperties();
}

void WorldFrame::highlightNode(WorldNode* wn)
{
	if(_highlightedNode) _highlightedNode->showHighlighted(false);
	_highlightedNode = wn;
	if(_highlightedNode)
	{
		_highlightedNode->showHighlighted(true);
	}
}

void WorldFrame::setActiveTool(MainWindow::ActiveTool tool) 
{
	_tools[_activeTool]->deactivate();
	_activeTool = tool;
	_tools[_activeTool]->activate();
}

bool WorldFrame::pickTerrainIntersection(wxMouseEvent& e, Vector3& pos)
{
	// Setup the ray scene query
	float mouseX = float(1.0f/_viewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/_viewport->getActualHeight()) * e.GetY();
	Ray mouseRay = _camera->getCameraToViewportRay(mouseX, mouseY);

	_raySceneQuery->setRay(mouseRay);
    _raySceneQuery->setSortByDistance(false);
	RaySceneQueryResult result = _raySceneQuery->execute();
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
	float mouseX = float(1.0f/_viewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/_viewport->getActualHeight()) * e.GetY();
	Ray mouseRay = _camera->getCameraToViewportRay(mouseX, mouseY);

	_raySceneQuery->setRay(mouseRay);
    _raySceneQuery->setSortByDistance(true);
	RaySceneQueryResult result = _raySceneQuery->execute();
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if (itr->movable && itr->movable->getName().substr(0, 4) == "node")
		{
			wn = _sceneNodeMap[itr->movable->getParentSceneNode()];
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
		if(_simpleRoadGraph.snapToNode(pos2D, snapSq, nd))
		{
			// all nodes in the simple graph are of type WorldNode
			NodeInterface *ni = _simpleRoadGraph.getNode(nd);
			assert(typeid(*ni) == typeid(WorldNode));
			wn = static_cast<WorldNode*>(ni);
			return true;
		}
	}
	return false;
}

WorldNode* WorldFrame::createNode()
{
	WorldNode* wn = new WorldNode(_roadGraph, _simpleRoadGraph, _sceneManager);
	//wn->mSimpleNodeId = _simpleRoadGraph.addNode(wn);
	//wn->mNodeId = _roadGraph.addNode(wn);
	_sceneNodeMap[wn->getSceneNode()] = wn;

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
	BOOST_FOREACH(WorldObject* wo, wr->getAllAttachments())
	{
		// if attachment is a cell
		if(typeid(*wo) == typeid(WorldCell))
		{
			WorldCell* wc = static_cast<WorldCell*>(wo);

			// get cell ptr
			attachedCells.push_back(wc);

			// get cell boundary data 
			boundaries.push_back(wc->getBoundaryCycle());

			// clear cell graph, remember we are messing with its data
			wc->clear();
		}
	}

	// delete road node
	_sceneRoadMap.erase(wr->getSceneNode());
	delete wr;

	// create replacement roads
	WorldRoad* wr1 = new WorldRoad(wn1, wn, _roadGraph, _simpleRoadGraph, _sceneManager);
	_sceneRoadMap[wr1->getSceneNode()] = wr1;
	WorldRoad* wr2 = new WorldRoad(wn, wn2, _roadGraph, _simpleRoadGraph, _sceneManager);
	_sceneRoadMap[wr2->getSceneNode()] = wr2;

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
		boost::tie(rIt, rEnd) = _simpleRoadGraph.getRoadsFromNode(wn->mSimpleNodeId);
		if(rIt == rEnd) break;
		WorldRoad* wr = static_cast<WorldRoad*>(_simpleRoadGraph.getRoad(*rIt));
		deleteRoad(wr);
	}

	// update current node if necessary
	if(_highlightedNode == wn) _highlightedNode = 0;
	if(_selectedNode == wn) selectNode(0);

	//Delete the Node
	_simpleRoadGraph.removeNode(wn->mSimpleNodeId);
	_roadGraph.removeNode(wn->_nodeId);
	_sceneNodeMap.erase(wn->getSceneNode());

	delete wn;
}

WorldRoad* WorldFrame::createRoad(WorldNode* wn1, WorldNode* wn2)
{
	modify(true);

	// if road is not present in graph
	if(!_simpleRoadGraph.testRoad(wn1->mSimpleNodeId, wn2->mSimpleNodeId))
	{
		// create the road in the scene
		WorldRoad* wr = new WorldRoad(wn1, wn2, _roadGraph, _simpleRoadGraph, _sceneManager);
		_sceneRoadMap[wr->getSceneNode()] = wr;

		// check the road graph to get a count of the number of cycles
		vector< vector<NodeInterface*> > nodeCycles;
		vector< vector<NodeInterface*> > filaments;
		_simpleRoadGraph.extractPrimitives(filaments, nodeCycles);

		// DEBUG
//		stringstream os;
//		size_t f = 0;
//		os << "Filaments:\n";
//		BOOST_FOREACH(vector<NodeInterface*> &filament, filaments)
//		{
//			os << "F" << f++ << ":";
//			BOOST_FOREACH(NodeInterface* fn, filament) os << *fn;
//			os << "\n";
//		}
//		LogManager::getSingleton().logMessage(os.str());

		// if the number of cycles is greater than the number of cells
		// then we have most definitely made a new cell
		if(nodeCycles.size() > _cellSet.size())
		{
			// find the new cycles
			WorldCell* alteredCell = 0;
			BOOST_FOREACH(WorldCell* wc, _cellSet)
			{
				bool cellFound = false;

				vector< vector<NodeInterface*> >::iterator ncIt, ncEnd;
				for(ncIt = nodeCycles.begin(), ncEnd = nodeCycles.end(); ncIt != ncEnd; ncIt++)
				{
					// if cell has boundary of cycle
					if(wc->compareBoundary(*ncIt))
					{
						// remove cycle, as its not new
						nodeCycles.erase(ncIt);
						cellFound = true;
						break;
					}
				}

				// if no match was found
				if(!cellFound)
				{
					assert(alteredCell == 0);	// there should only ever be one changed cell
					alteredCell = wc;
				}
			}

			// two options are possible:
			switch(nodeCycles.size()) 
			{
			// 1: created a new cell
			case 1:
				{
					WorldCell* wc = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycles[0]);
					_sceneCellMap[wc->getSceneNode()] = wc;
					_cellSet.insert(wc);
				}
				break;
			// 2: divided an existing cell into two
			case 2:
				{
					// NOTE: maybe a copy constructor could be tidier

					// get old cell params
					CellGenParams g = alteredCell->getGenParams();
					// delete old cell
					_cellSet.erase(alteredCell);
					_sceneCellMap.erase(alteredCell->getSceneNode());
					delete alteredCell;


					// create 2 new cells in place of old cell with old cell params
					WorldCell* wc0 = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycles[0]); 
					_sceneCellMap[wc0->getSceneNode()] = wc0;
					WorldCell* wc1 = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycles[1]);
					_sceneCellMap[wc1->getSceneNode()] = wc1;

					wc0->setGenParams(g);
					wc1->setGenParams(g);
					_cellSet.insert(wc0);
					_cellSet.insert(wc1);
				}
				break;
			default:
				new Exception(Exception::ERR_INTERNAL_ERROR, "What how many new cycles was that", "createRoad");
				break;
			}
		}
		else
		{
			//TODO: sort out filaments
/*
			// find out which filament wr is part of
			vector<NodeInterface*> *wrFilament = 0;
			BOOST_FOREACH(vector<NodeInterface*> &filament, filaments)
			{
				WorldRoad* wrF = 0;
				for(size_t i=0; i<(filament.size()-1) && wr != wrF; i++)
					wrF = getWorldRoad(static_cast<WorldNode*>(filament[i]), 
										static_cast<WorldNode*>(filament[i+1]));

				// if wr found in filament, store filament
				if(wr == wrF) 
				{
					wrFilament = &filament;
					break;
				}
			}
			// better find it!
			assert(wrFilament != 0);

			// get the set of cells the filament could belong to.
			set<WorldCell*> possibleCells;
			BOOST_FOREACH(WorldRoad* w, static_cast<WorldNode*>((*wrFilament)[0])->getWorldRoads())
			{
			
			}
*/

			// road is probably a filament, if its in a cell add it
			BOOST_FOREACH(WorldCell* wc, _cellSet)
			{
				if(wc->isInside(wr->getSrcNode()->getPosition2D())) 
				{
					if(wc->isInside(wr->getDstNode()->getPosition2D()) 
						|| wc->isBoundaryNode(wr->getDstNode()))
					{
						wc->addFilament(wr);
						break;
					}
				}
				else if(wc->isInside(wr->getDstNode()->getPosition2D())) 
				{
					if(wc->isInside(wr->getSrcNode()->getPosition2D())
						|| wc->isBoundaryNode(wr->getSrcNode()))
					{
						wc->addFilament(wr);
						break;
					}
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
		_sceneRoadMap.erase(wr->getSceneNode());
		delete wr;
		break;
	case 1:
		{
			// could be a boundary edge or a filament
			const vector<RoadInterface*> &boundary(aCells[0]->getBoundaryRoads());

			// search for boundary cycle
			size_t i;
			for(i=0; i<boundary.size() && boundary[i] != wr; i++);

			// if found on the boundary cycle
			if(i<boundary.size())
			{
				_sceneCellMap.erase(aCells[0]->getSceneNode());
				_cellSet.erase(aCells[0]);
				delete aCells[0];
			}
			else
			{
				aCells[0]->removeFilament(wr);
			}

			// delete road
			_sceneRoadMap.erase(wr->getSceneNode());
			delete wr;
		}
		break;
	case 2:
		{
			// should favour one - can do a vector swap to set preference

			// save params from the biggest attached cell by area
			CellGenParams gp = aCells[0]->calcArea2D() > aCells[1]->calcArea2D() ? 
								aCells[0]->getGenParams() : aCells[1]->getGenParams();

			// delete cells
			BOOST_FOREACH(WorldCell* c, aCells)
			{
				_sceneCellMap.erase(c->getSceneNode());
				_cellSet.erase(c);
				delete c;
			}

			// delete road
			_sceneRoadMap.erase(wr->getSceneNode());
			delete wr;

			// run cell decomposition
			vector< vector<NodeInterface*> > nodeCycles;
			vector< vector<NodeInterface*> >  filaments;
			_simpleRoadGraph.extractPrimitives(filaments, nodeCycles);

			// find the new cell if there is one
			BOOST_FOREACH(vector<NodeInterface*> &cycle, nodeCycles)
			{
				bool newCell = true;
				BOOST_FOREACH(WorldCell* c, _cellSet)
				{
					if(c->compareBoundary(cycle))
					{
						newCell = false;
						break;
					}
				}
				// create the new cell and break
				if(newCell)
				{
					WorldCell* wc = new WorldCell(_roadGraph, _simpleRoadGraph, cycle);
					_sceneCellMap[wc->getSceneNode()] = wc;
					_cellSet.insert(wc);
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
	_highlightedNode = 0;
	_selectedNode = 0;
	_selectedRoad = 0;
	_intersectionPresent = false;

	
	onCloseDoc();
	createScene();
	_tools[_activeTool]->activate();
	update();
	_isDocOpen = true;
}

void WorldFrame::onCloseDoc()
{
	if(_isDocOpen)
	{
		_isDocOpen = false;
		_tools[_activeTool]->deactivate();
		
		// destroy scene data
		destroyScene();

		// destroy graph data
		_simpleRoadGraph.clear();
		_roadGraph.clear();

		update();
	}
}

bool WorldFrame::plotPointOnTerrain(Real x, Real &y, Real z)
{
	//create scene doc using doc.x -> x, doc.y -> z  and y from ray cast
	Ray verticalRay(Vector3(x, 5000.0f, z), Vector3::NEGATIVE_UNIT_Y);
	RaySceneQuery* ray = _sceneManager->createRayQuery(verticalRay);
	RaySceneQueryResult result = ray->execute();

	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if(itr->worldFragment)
		{
			y = itr->worldFragment->singleIntersection.y;
			delete ray;
			return true;
		}
	}
	delete ray;
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
	Vector3 pos3D;
	if(pickTerrainIntersection(e, pos3D)) 
	{
		BOOST_FOREACH(WorldCell* c, _cellSet)
		{
			if(c->isInside(Geometry::V2(pos3D)))
			{
				wc = c;
				return true;
			}
		}
	}
	return false;
}

void WorldFrame::selectCell(WorldCell* wn)
{
	if(_selectedCell) _selectedCell->showSelected(false);
	_selectedCell = wn;
	if(_selectedCell)
	{
		_selectedCell->showSelected(true);
	}
	updateProperties();
}

bool WorldFrame::pickRoad(wxMouseEvent& e, WorldRoad *&wr)
{
	// Setup the ray scene query
	float mouseX = float(1.0f/_viewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/_viewport->getActualHeight()) * e.GetY();
	Ray mouseRay = _camera->getCameraToViewportRay(mouseX, mouseY);

	_raySceneQuery->setRay(mouseRay);
    _raySceneQuery->setSortByDistance(true);
	RaySceneQueryResult result = _raySceneQuery->execute();
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++)
	{
		if (itr->movable && itr->movable->getName().substr(0, 4) == "road")
		{
			wr = _sceneRoadMap[itr->movable->getParentSceneNode()];
    		return true;
		}
	}
	return false;
}

void WorldFrame::selectRoad(WorldRoad* wr)
{
	if(_selectedRoad) _selectedRoad->showSelected(false);
	_selectedRoad = wr;
	if(_selectedRoad)
	{
		_selectedRoad->showSelected(true);
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
	case MainWindow::view_box:
		showRoads = true;
		showBuildings = true;
		break;
	case MainWindow::view_building:
		break;
	}
	set<WorldCell*>::iterator cIt,cEnd;
	for(cIt = _cellSet.begin(), cEnd = _cellSet.end(); cIt != cEnd; cIt++)
	{
		(*cIt)->showRoads(showRoads);
		(*cIt)->showBuildings(showBuildings);
	}
	_viewMode = mode;
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
