// Includes
#include "stdafx.h"
#include "WorldFrame.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldCell.h"
#include "PerformanceTimer.h"
#include "Statistics.h"
#include "ExportDoc.h"
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
#include <wx/msgdlg.h>

#include <OgreMaterialManager.h>
#include <OgreRenderSystem.h>
#include <OgreStringConverter.h>
#include <OgreRoot.h>
#include <tinyxml.h>

//#define THREADME 1


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
EVT_PAINT(WorldFrame::OnPaint)
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

	_highlightedNode = 0;
	_highlightedRoad = 0;
	_highlightedCell = 0;
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
	int attrlist[] =
	{	GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, None};
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
	_renderWindow = Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false,
			&params);
	_renderWindow->setActive(true);

	// Create the SceneManager, in this case the terrainscenemanager
	_sceneManager = Root::getSingleton().createSceneManager("TerrainSceneManager");
	createCamera();
	createViewport();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Desperate attempt to improve image quality on linux w/fglrx
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(4);

	// Make sure assets are loaded before we create the scene
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	//createScene();
	_isDocOpen = false;

	_viewMode = WorldCell::view_box;
	_toolsetMode = MainWindow::view;
	_activeTool = MainWindow::addNode;

	_tools.push_back(new ToolView(this));
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
	if (_isDocOpen)
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
	BOOST_FOREACH(Tool* tool, _tools) delete tool;
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
	_camera->setNearClipDistance(3);
	_camera->setFarClipDistance(1000);

	// camera is positioned in createScene - not here
}

void WorldFrame::createScene(void)
{
	// First check that vertex programs and dot3 or fragment programs are supported
	const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
	if (!caps->hasCapability(RSC_VERTEX_PROGRAM))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"Your card does not support vertex programs, so cannot "
					"run this application. Sorry!", "WorldFrame::createScene");
	}
	if (!(caps->hasCapability(RSC_FRAGMENT_PROGRAM)
			|| caps->hasCapability(RSC_DOT3)))
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"Your card does not support dot3 blending or fragment programs, so cannot "
					"run this application. Sorry!", "WorldFrame::createScene");
	}

	// Shadow settings
	//_sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	//_sceneManager->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Hey, it's the sun!
	_mainLight = _sceneManager->createLight("SunLight");
	_mainLight->setType(Light::LT_SPOTLIGHT);
	_mainLight->setPosition(0,12000,10000);
	_mainLight->setSpotlightRange(Degree(30), Degree(50));
	_mainLight->setDirection(-_mainLight->getPosition().normalisedCopy());

	_sceneManager->setAmbientLight(ColourValue(0.34, 0.34, 0.38));	// blueish
	_mainLight->setDiffuseColour(0.91, 0.91, 0.85);					// yellowish
	_mainLight->setSpecularColour(0.5, 0.5, 0.5);

	// Fog
	// NB it's VERY important to set this before calling setWorldGeometry 
	// because the vertex program picked will be different
	ColourValue fadeColour(0.76f, 0.86f, 0.93f);
	//_sceneManager->setFog(FOG_LINEAR, fadeColour, .001f, 500, 1000);
	_viewport->setBackgroundColour(fadeColour);

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
	_cameraNode = _sceneManager->createSceneNode("cameraNode");
	_cameraNode->setPosition(6957.0f, 10.0f, 6400.0f);
	_cameraNode->attachObject(_camera);
	_camera->setPosition(0, 0, 500); // zoom on z axis, look down -z
	_camera->lookAt(_cameraNode->getPosition());
	_cameraNode->setOrientation(0.631968, -0.163438, -0.733434, -0.189679);
}

void WorldFrame::exportScene(ExportDoc& doc)
{
	// add the camera
	doc.addCamera(_camera, _cameraNode);

	// add the light
	doc.addLight(_mainLight);

	// export nodes, road and cells
	BOOST_FOREACH(WorldNode* wn, _nodeVec) wn->exportObject(doc);
	BOOST_FOREACH(WorldRoad* wr, _roadVec) wr->exportObject(doc);
	BOOST_FOREACH(WorldCell* c, _cellVec) c->exportObject(doc);

	// build a test mesh
	//Entity* ent = _sceneManager->createEntity("mcube", "cube.mesh");
	//ent->getMesh()->getSubMesh(0)->setMaterialName("gk/Building3");
	//doc.addMesh(ent->getMesh());
	//_sceneManager->destroyEntity(ent);
	//doc.exportMesh();
}

void WorldFrame::createViewport(void)
{
	if (_viewport)
		destroyViewport();
	// Create one view port, entire window
	_viewport = _renderWindow->addViewport(_camera);
	_viewport->setBackgroundColour(ColourValue(0.5f, 0.5f, 0.5f));

	// Alter the camera aspect ratio to match the view port
	_camera->setAspectRatio(Real(_viewport->getActualWidth())
			/ Real(_viewport->getActualHeight()));
}

void WorldFrame::destroyScene(void)
{
	_viewport->setBackgroundColour(ColourValue(0.5f, 0.5f, 0.5f));

	// Delete cells, road and nodes
	BOOST_FOREACH(WorldCell* c, _cellVec) delete c;
	BOOST_FOREACH(WorldRoad* wr, _roadVec) delete wr;
	BOOST_FOREACH(WorldNode* wn, _nodeVec) delete wn;
	_cellVec.clear();
	_roadVec.clear();
	_nodeVec.clear();

	// clear anything else
	_sceneManager->clearScene();
	
	// destroy ray
	delete _raySceneQuery;

	// reset counts
	WorldNode::resetInstanceCount();
	WorldRoad::resetInstanceCount();
	WorldCell::resetInstanceCount();
}

void WorldFrame::cameraMove(Real x, Real y, Real z)
{
	_camera->moveRelative(Vector3(x, y, z));
	onCameraUpdate();
}

void WorldFrame::cameraNodeMove(Real x, Real y, Real z)
{
	_cameraNode->translate(x, y, z);
	onCameraUpdate();
}

void WorldFrame::cameraNodeRotate(Real yaw, Real pitch)
{
	_cameraNode->rotate(Vector3::UNIT_X, Degree(pitch));
	_cameraNode->rotate(Vector3::UNIT_Y, Degree(yaw), Node::TS_WORLD);
	onCameraUpdate();
}

void WorldFrame::cameraRotate(Real yaw, Real pitch)
{
	_camera->yaw(yaw * (_camera->getFOVy() / _camera->getAspectRatio() / 320.0f));
	_camera->pitch(pitch * (_camera->getFOVy() / 240.0f));
	onCameraUpdate();
}

void WorldFrame::cameraZoom(Ogre::Real z)
{
	_camera->moveRelative(Vector3(0.0f, 0.0f, z));
	onCameraUpdate();
}

void WorldFrame::onCameraUpdate()
{
	modify(true);
	if (_toolsetMode == MainWindow::view) updateProperties();
	update();
}

void WorldFrame::OnChar(wxKeyEvent& e)
{
	if (!_isDocOpen)
		return;
	_tools[_activeTool]->OnChar(e);
}

void WorldFrame::OnLeftPressed(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	// if you click on me get back focus
	// focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();

	_tools[_activeTool]->OnLeftPressed(e);
}

void WorldFrame::OnLeftReleased(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	_tools[_activeTool]->OnLeftReleased(e);
}

void WorldFrame::OnMiddlePressed(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	// if you click on me get back focus
	// focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();

	_tools[_activeTool]->OnMiddlePressed(e);
}

void WorldFrame::OnMiddleReleased(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	_tools[_activeTool]->OnMiddleReleased(e);
}

void WorldFrame::OnRightPressed(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	// if you click on me get back focus
	// focus should really be assigned by what your mouse is over but until then...
	this->SetFocusFromKbd();
	this->SetFocus();
	_tools[_activeTool]->OnRightPressed(e);
}

void WorldFrame::OnRightReleased(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	_tools[_activeTool]->OnRightReleased(e);
}

void WorldFrame::OnMouseMove(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	_tools[_activeTool]->OnMouseMove(e);
}

void WorldFrame::OnMouseWheel(wxMouseEvent &e)
{
	if (!_isDocOpen)
		return;
	_tools[_activeTool]->OnMouseWheel(e);
}

void WorldFrame::OnPaint(wxPaintEvent &WXUNUSED(e))
{
	wxPaintDC dc(this);
	update();
}

void WorldFrame::OnSize(wxSizeEvent &e)
{
	if (!_renderWindow)
		return;

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

void prebuild(
		pair< vector<WorldCell*>::iterator, vector<WorldCell*>::iterator > *cIt)
{
	while (true)
	{
		WorldCell* wc = 0;
		{
			boost::mutex::scoped_lock lock(cit_mutex);
			if (cIt->first != cIt->second)
			{
				wc = *(cIt->first);
				cIt->first++;
			}
			else
				break;
		}

		if (wc && !wc->isValid())
			/*DEBUG: for(size_t i=0; i<100; i++)*/wc->prebuild(); //do it 100 times
	}
}

void prebuild2(vector<WorldCell*> &cells)
{
	BOOST_FOREACH(WorldCell* c, cells)
	{
		if (c && !c->isValid())
			/*DEBUG: for(size_t i=0; i<100; i++)*/c->prebuild(); //do it 100 times
	}
}

void WorldFrame::update()
{
	try {
		//Statistics::resetBuildingCount();
	
		// render nodes, roads ...
		PerformanceTimer npf("Nodes");
		BOOST_FOREACH(WorldNode* wn, _nodeVec) wn->validate();
		npf.stop();
	
		PerformanceTimer rpf("Roads");
		BOOST_FOREACH(WorldRoad* wr, _roadVec) wr->validate();
		rpf.stop();
	
		PerformanceTimer cpf("Cells 1");
		pair<vector<WorldCell*>::iterator, vector<WorldCell*>::iterator> cPIt;
		cPIt.first = _cellVec.begin();
		cPIt.second = _cellVec.end();
	
	#ifdef THREADME
		boost::thread thrd1(boost::bind(&prebuild, &cPIt));
		boost::thread thrd2(boost::bind(&prebuild, &cPIt));
		thrd1.join();
		thrd2.join();
	#else
		prebuild(&cPIt);
	#endif
		cpf.stop();
	
		PerformanceTimer cpf2("Cells 2");
		BOOST_FOREACH(WorldCell* c, _cellVec) c->validate();
		cpf2.stop();
	
		PerformanceTimer renpf("Render");
		if (_camera)
			Root::getSingleton().renderOneFrame();
		renpf.stop();
	
		LogManager::getSingleton().logMessage(npf.toString()+" - "+rpf.toString()+" - "+cpf.toString()
			+" - "+cpf2.toString()+" - "+renpf.toString());
	} 
	catch(Exception &e)
	{
		wxMessageBox(_T("Ogre::Exception: ")+_U(e.getFullDescription().c_str()),
						_("Update Exception"), wxICON_EXCLAMATION);
	}
	catch(std::exception &e)
	{
		wxMessageBox(_T("std::exception: ")+_U(e.what()),
						_("Update Exception"), wxICON_EXCLAMATION);
	}
	catch(...)
	{
		wxMessageBox(_T("Undescribed"),
						_("Update Exception"), wxICON_EXCLAMATION);
	}
}

bool WorldFrame::loadXML(const TiXmlHandle& worldRoot, const std::string &filePath)
{
	//
	onCloseDoc();
	createScene();

	// load camera
	string camId = worldRoot.FirstChild("camera").Element()->Attribute("id");
	if (camId == "edit_cam")
	{
		TiXmlElement *cameraChild = worldRoot.FirstChild("camera").FirstChild().Element();

		for (; cameraChild; cameraChild=cameraChild->NextSiblingElement())
		{
			string key = cameraChild->Value();
			if (key == "position")
			{
				Real x, y, z;
				cameraChild->QueryFloatAttribute("x", &x);
				cameraChild->QueryFloatAttribute("y", &y);
				cameraChild->QueryFloatAttribute("z", &z);
				_camera->setPosition(x, y, z);
			}
			else if (key == "camOrientation")
			{
				Quaternion nodeOrient;
				cameraChild->QueryFloatAttribute("w", &nodeOrient.w);
				cameraChild->QueryFloatAttribute("x", &nodeOrient.x);
				cameraChild->QueryFloatAttribute("y", &nodeOrient.y);
				cameraChild->QueryFloatAttribute("z", &nodeOrient.z);
				_cameraNode->setOrientation(nodeOrient);
			}
			else if (key == "nodePosition")
			{
				Real x, y, z;
				cameraChild->QueryFloatAttribute("x", &x);
				cameraChild->QueryFloatAttribute("y", &y);
				cameraChild->QueryFloatAttribute("z", &z);
				_cameraNode->setPosition(x, y, z);
			}
		}
	}
	
	// load terrain
	TiXmlElement* terrainElem=worldRoot.FirstChild("terrain").Element();
	if(terrainElem)
		_worldTerrain.loadXML(TiXmlHandle(terrainElem), filePath);

	_worldTerrain.load(_sceneManager);

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
	for (; pElem; pElem=pElem->NextSiblingElement())
	{
		string key = pElem->Value();
		if (key == "node")
		{
			Real x, y;
			string strId = pElem->Attribute("id");
			pElem->QueryFloatAttribute("x", &x);
			pElem->QueryFloatAttribute("y", &y);
			string label = pElem->Attribute("label");
			WorldNode* wn = createNode();
			wn->setPosition2D(x, y);
			wn->setLabel(label);
			nodeIdTranslation.insert(make_pair(strId, wn));
		}
		else if (key == "edge")
		{
			edgeData.push_back(make_pair(pElem->Attribute("source"),
					pElem->Attribute("target")));
			edgeElements.push_back(pElem);
		}
		else if (key == "cells")
			break;
	}

	// create the edges now that all graph data has been read
	for (unsigned int i=0; i<edgeData.size(); i++)
	{
		map<string, WorldNode*>::iterator sourceIt, targetIt;
		sourceIt = nodeIdTranslation.find(edgeData[i].first);
		targetIt = nodeIdTranslation.find(edgeData[i].second);
		// if source node and target node can be found
		if (sourceIt != nodeIdTranslation.end() && targetIt
				!= nodeIdTranslation.end())
		{
			if (!_simpleRoadGraph.testRoad(sourceIt->second->mSimpleNodeId,
					targetIt->second->mSimpleNodeId))
			{
				// create the road in the scene
				WorldRoad* wr = new WorldRoad(sourceIt->second, targetIt->second,
						_roadGraph, _simpleRoadGraph, _sceneManager);
				_roadVec.push_back(wr);
				const TiXmlHandle roadRoot(edgeElements[i]);
				wr->loadXML(roadRoot);
			}
		}
	}

	pElem=worldRoot.FirstChild("cells").FirstChild().Element();
	for (; pElem; pElem=pElem->NextSiblingElement())
	{
		if (string(pElem->Value())=="cell")
		{
			vector<NodeInterface*> nodeCycle;

			//load the cycle
			TiXmlElement* pElem2=pElem->FirstChild("cycle")->FirstChildElement();
			for (; pElem2; pElem2=pElem2->NextSiblingElement())
			{
				if (string(pElem2->Value())=="node")
				{
					string strId = pElem2->Attribute("id");
					nodeCycle.push_back(nodeIdTranslation[strId]);
				}
			}

			//load the filaments
			vector<WorldRoad*> filaments;
			TiXmlNode* f = pElem->FirstChild("filaments");
			if (f)
				pElem2 = f->FirstChildElement();
			for (; pElem2; pElem2=pElem2->NextSiblingElement())
			{
				if (string(pElem2->Value())=="filament")
				{
					string srcId, dstId;
					TiXmlElement* pElem3=pElem2->FirstChildElement();

					if (string(pElem3->Value()) == "node")
						srcId = pElem3->Attribute("id");
					pElem3 = pElem3->NextSiblingElement();
					if (string(pElem3->Value()) == "node")
						dstId = pElem3->Attribute("id");

					filaments.push_back(getWorldRoad(nodeIdTranslation[srcId],
							nodeIdTranslation[dstId]));
				}
			}

			if (nodeCycle.size() > 2)
			{
				WorldCell* wc = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycle, _viewMode);
				_cellVec.push_back(wc);
				BOOST_FOREACH(WorldRoad* wr, filaments) wc->addFilament(wr);
				const TiXmlHandle cellRoot(pElem);
				wc->loadXML(pElem);
			}
		}
	}

	_tools[_activeTool]->activate();
	_isDocOpen = true;
	Refresh();
	return true;
}

TiXmlElement* WorldFrame::saveXML(const std::string &filePath)
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

	// Save Terrain
	TiXmlElement *terrain = _worldTerrain.saveXML(filePath);
	root->LinkEndChild(terrain);

	//<graph id="roadgraph" edgedefault="undirected">
	TiXmlElement *roadNetwork = new TiXmlElement("graph");
	roadNetwork->SetAttribute("id", "roadgraph");
	roadNetwork->SetAttribute("edgedefault", "undirected");
	root->LinkEndChild(roadNetwork);

	NodeIterator nIt, nEnd;
	for ( boost::tie(nIt, nEnd) = _simpleRoadGraph.getNodes(); nIt != nEnd; nIt++)
	{
		NodeInterface* ni = _simpleRoadGraph.getNode(*nIt);
		Vector2 loc(ni->getPosition2D());

		TiXmlElement * node;
		node = new TiXmlElement("node");
		node->SetAttribute("id", (int) ni);
		node->SetDoubleAttribute("x", loc.x);
		node->SetDoubleAttribute("y", loc.y);
		node->SetAttribute("label", static_cast<WorldNode*>(ni)->getLabel().c_str());
		roadNetwork->LinkEndChild(node);
	}

	RoadIterator rIt, rEnd;
	for ( boost::tie(rIt, rEnd) = _simpleRoadGraph.getRoads(); rIt != rEnd; rIt++)
	{
		WorldRoad* wr = static_cast<WorldRoad*>(_simpleRoadGraph.getRoad(*rIt));
		roadNetwork->LinkEndChild(wr->saveXML());
	}

	TiXmlElement *cells = new TiXmlElement("cells");
	cells->SetAttribute("id", "cellset0");
	root->LinkEndChild(cells);

	BOOST_FOREACH(WorldCell* c, _cellVec)
	cells->LinkEndChild(c->saveXML());

	return root;
}

void WorldFrame::setToolsetMode(MainWindow::ToolsetMode mode)
{
	// hide any highlighted or selected items
	highlightNode(0);
	if(_selectedNode) _selectedNode->setSelected(false);
	highlightRoad(0);
	if(_selectedRoad) _selectedRoad->setSelected(false);
	highlightCell(0);
	if(_selectedCell) _selectedCell->setSelected(false);

	// show any relevant selections
	switch(mode)
	{
	case MainWindow::node:
		if(_selectedNode) _selectedNode->setSelected(true);
		break;
	case MainWindow::road:
		if(_selectedRoad) _selectedRoad->setSelected(true);
		break;
	case MainWindow::cell:
		if(_selectedCell) _selectedCell->setSelected(true);
		break;
	}
	_toolsetMode = mode;
	Refresh();
}

void WorldFrame::endNodeMode()
{
	selectNode(0);
}

void WorldFrame::beginNodeMode()
{
	selectNode(0);
}

void WorldFrame::highlightNode(WorldNode* wn)
{
	if(_highlightedNode) _highlightedNode->setHighlighted(false);
	if(wn) wn->setHighlighted(true);
	_highlightedNode = wn;
}

void WorldFrame::highlightRoad(WorldRoad* wr)
{
	if(_highlightedRoad) _highlightedRoad->setHighlighted(false); 
	if(wr) wr->setHighlighted(true);
	_highlightedRoad = wr;
}

void WorldFrame::highlightCell(WorldCell* wc)
{
	if(_highlightedCell) _highlightedCell->setHighlighted(false);
	if(wc) wc->setHighlighted(true);
	_highlightedCell = wc;
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
	for (RaySceneQueryResult::const_iterator itr = result.begin(); itr
			!= result.end(); itr++)
	{
		if (itr->worldFragment)
		{
			pos = itr->worldFragment->singleIntersection;
			return true;
		}
	}
	return false;
}

bool WorldFrame::pickNode(wxMouseEvent &e, Real snapSq, WorldNode *&wn)
{
	Vector3 pos3D;
	if (pickTerrainIntersection(e, pos3D))
	{
		Vector2 pos2D(pos3D.x, pos3D.z);
		NodeId nd;
		if (_simpleRoadGraph.snapToNode(pos2D, snapSq, nd))
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
	_nodeVec.push_back(wn);
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

	// get road parameters
	RoadGenParams rg = wr->getGenParams();

	// delete road node
	vector<WorldRoad*>::iterator rIt = find(_roadVec.begin(), _roadVec.end(),
			wr);
	if (rIt != _roadVec.end())
		_roadVec.erase(rIt);
	delete wr;

	// create replacement roads
	WorldRoad* wr1 = new WorldRoad(wn1, wn, _roadGraph, _simpleRoadGraph, _sceneManager);
	WorldRoad* wr2 = new WorldRoad(wn, wn2, _roadGraph, _simpleRoadGraph, _sceneManager);
	wr1->setGenParams(rg);
	wr2->setGenParams(rg);
	_roadVec.push_back(wr1);
	_roadVec.push_back(wr2);

	// update cell boundaries
	size_t numOfCells = attachedCells.size();
	for (size_t i=0; i<numOfCells; i++)
	{
		// insert new node into boundary cycle
		size_t j, k, N = boundaries[i].size();
		for (j=0; j<N; j++)
		{
			k = (j+1) % N;
			if ((boundaries[i][j] == wn1 && boundaries[i][k] == wn2)
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

	// special case, if road is connected to two roads
	if(wn->getDegree() == 2)
	{
		RoadIterator2 rIt, rEnd;
		boost::tie(rIt, rEnd) = _simpleRoadGraph.getRoadsFromNode(wn->mSimpleNodeId);
		WorldRoad* wr1 = static_cast<WorldRoad*>(_simpleRoadGraph.getRoad(*rIt));
		WorldRoad* wr2 = static_cast<WorldRoad*>(_simpleRoadGraph.getRoad(*(++rIt)));
		RoadGenParams gp = wr1->getLengthSquared() > wr2->getLengthSquared() ?
			wr1->getGenParams() : wr2->getGenParams();
		WorldNode* wn1 = (wn == wr1->getSrcNode()) ? 
			static_cast<WorldNode*>(wr1->getDstNode()) : static_cast<WorldNode*>(wr1->getSrcNode());
		WorldNode* wn2 = (wn == wr2->getSrcNode()) ?
			static_cast<WorldNode*>(wr2->getDstNode()) : static_cast<WorldNode*>(wr2->getSrcNode());
		deleteRoad(wr1);
		deleteRoad(wr2);

		// test to see if a replacement road can be created
		WorldRoad* wr = createRoad(wn1, wn2);

		// road may already exist and hence wr == 0
		if(wr != 0) 
		{
			wr->setGenParams(gp);
			wr->validate();
			Vector2 pos;
			WorldNode* wns;
			WorldRoad* wrs;
			int snapState = wr->snapInfo(10, pos, wns, wrs);

			// we tried but a valid road could not be created
			if(snapState == 1 || (snapState == 2 && (wns != wn1 && wns != wn2))) deleteRoad(wr);
		}
	}
	else
	{
		//delete any connected roads
		while (true)
		{
			RoadIterator2 rIt, rEnd;
			boost::tie(rIt, rEnd) = _simpleRoadGraph.getRoadsFromNode(wn->mSimpleNodeId);
			if (rIt == rEnd)
				break;
			WorldRoad* wr = static_cast<WorldRoad*>(_simpleRoadGraph.getRoad(*rIt));
			deleteRoad(wr);
		}
	}

	// update current node if necessary
	if (_highlightedNode == wn)
		_highlightedNode = 0;
	if (_selectedNode == wn)
		selectNode(0);

	//Delete the Node
	vector<WorldNode*>::iterator nIt = find(_nodeVec.begin(), _nodeVec.end(),
			wn);
	if (nIt != _nodeVec.end())
		_nodeVec.erase(nIt);
	delete wn;
}

WorldRoad* WorldFrame::createRoad(WorldNode* wn1, WorldNode* wn2)
{
	modify(true);

	// if road is not present in graph
	if (!_simpleRoadGraph.testRoad(wn1->mSimpleNodeId, wn2->mSimpleNodeId))
	{
		// create the road in the scene
		WorldRoad* wr = new WorldRoad(wn1, wn2, _roadGraph, _simpleRoadGraph, _sceneManager);
		_roadVec.push_back(wr);

		// check the road graph to get a count of the number of cycles
		vector< vector<NodeInterface*> > nodeCycles;
		vector< vector<NodeInterface*> > filaments;
		_simpleRoadGraph.extractPrimitives(filaments, nodeCycles);

		// if the number of cycles is greater than the number of cells
		// then we have most definitely made a new cell
		if (nodeCycles.size() > _cellVec.size())
		{
			// find the new cycles
			WorldCell* alteredCell = 0;
			BOOST_FOREACH(WorldCell* wc, _cellVec)
			{
				bool cellFound = false;

				vector< vector<NodeInterface*> >::iterator ncIt, ncEnd;
				for (ncIt = nodeCycles.begin(), ncEnd = nodeCycles.end(); ncIt
						!= ncEnd; ncIt++)
				{
					// if cell has boundary of cycle
					if (wc->compareBoundary(*ncIt))
					{
						// remove cycle, as its not new
						nodeCycles.erase(ncIt);
						cellFound = true;
						break;
					}
				}

				// if no match was found
				if (!cellFound)
				{
					assert(alteredCell == 0); // there should only ever be one changed cell
					alteredCell = wc;
				}
			}

			// two options are possible:
			switch (nodeCycles.size())
			{
			// 1: created a new cell
			case 1:
				_cellVec.push_back(new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycles[0], _viewMode));
				break;
				// 2: divided an existing cell into two
			case 2:
			{
				// NOTE: maybe a copy constructor could be tidier

				// get old cell params
				CellParams g = alteredCell->getGenParams();
				// delete old cell
				vector<WorldCell*>::iterator cIt = find(_cellVec.begin(),
						_cellVec.end(), alteredCell);
				if (cIt != _cellVec.end())
					_cellVec.erase(cIt);
				delete alteredCell;

				// create 2 new cells in place of old cell with old cell params
				WorldCell* wc0 = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycles[0], _viewMode);
				WorldCell* wc1 = new WorldCell(_roadGraph, _simpleRoadGraph, nodeCycles[1], _viewMode);
				wc0->setGenParams(g);
				wc1->setGenParams(g);
				_cellVec.push_back(wc0);
				_cellVec.push_back(wc1);
			}
				break;
			default:
				new Exception(Exception::ERR_INTERNAL_ERROR, "What how many new cycles was that", "createRoad");
				break;
			}
		}
		else
		{
			// road is probably a filament, if its in a cell add it
			BOOST_FOREACH(WorldCell* wc, _cellVec)
			{
				if (wc->isInside(wr->getSrcNode()->getPosition2D()))
				{
					if (wc->isInside(wr->getDstNode()->getPosition2D())
							|| wc->isBoundaryNode(wr->getDstNode()))
					{
						wc->addFilament(wr);
						break;
					}
				}
				else if (wc->isInside(wr->getDstNode()->getPosition2D()))
				{
					if (wc->isInside(wr->getSrcNode()->getPosition2D())
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

	//
	if(wr == _highlightedRoad) _highlightedRoad = 0;
	if(wr == _selectedRoad) _selectedRoad = 0;

	// get cells attached to this road
	vector<WorldCell*> aCells;
	set<WorldObject*> attachments(wr->getAllAttachments());
	BOOST_FOREACH(WorldObject* wo, attachments)
	{
		// if attachment is a cell
		if (typeid(*wo) == typeid(WorldCell))
			aCells.push_back(static_cast<WorldCell*>(wo));
	}

	switch (aCells.size())
	{
	case 0:
	{
		vector<WorldRoad*>::iterator rIt = find(_roadVec.begin(),
				_roadVec.end(), wr);
		if (rIt != _roadVec.end())
			_roadVec.erase(rIt);
		delete wr;
		break;
	}
	case 1:
	{
		// could be a boundary edge or a filament
		const vector<RoadInterface*> &boundary(aCells[0]->getBoundaryRoads());

		// if found on the boundary cycle
		if (find(boundary.begin(), boundary.end(), wr) != boundary.end())
		{
			vector<WorldCell*>::iterator cIt = find(_cellVec.begin(),
					_cellVec.end(), aCells[0]);
			if (cIt != _cellVec.end())
				_cellVec.erase(cIt);
			if(aCells[0] == _highlightedCell) _highlightedCell = 0;
			if(aCells[0] == _selectedCell) _selectedCell = 0;
			delete aCells[0];
		}
		else
		{
			aCells[0]->removeFilament(wr);
		}

		// delete road
		vector<WorldRoad*>::iterator rIt = find(_roadVec.begin(),
				_roadVec.end(), wr);
		if (rIt != _roadVec.end())
			_roadVec.erase(rIt);
		delete wr;
	}
		break;
	case 2:
	{
		// should favor one - can do a vector swap to set preference

		// save params from the biggest attached cell by area
		CellParams gp = aCells[0]->calcArea2D() > aCells[1]->calcArea2D() ? 
			aCells[0]->getGenParams() : aCells[1]->getGenParams();

		// delete cells
		BOOST_FOREACH(WorldCell* c, aCells)
		{
			vector<WorldCell*>::iterator cIt = find(_cellVec.begin(),
					_cellVec.end(), c);
			if (cIt != _cellVec.end())
				_cellVec.erase(cIt);
			if(c == _highlightedCell) _highlightedCell = 0;
			if(c == _selectedCell) _selectedCell = 0;
			delete c;
		}

		// delete road
		vector<WorldRoad*>::iterator rIt = find(_roadVec.begin(),
				_roadVec.end(), wr);
		if (rIt != _roadVec.end())
			_roadVec.erase(rIt);
		delete wr;

		// run cell decomposition
		vector< vector<NodeInterface*> > nodeCycles;
		vector< vector<NodeInterface*> > filaments;
		_simpleRoadGraph.extractPrimitives(filaments, nodeCycles);

		// find the new cell if there is one
		BOOST_FOREACH(vector<NodeInterface*> &cycle, nodeCycles)
		{
			bool newCell = true;
			BOOST_FOREACH(WorldCell* c, _cellVec)
			{
				if (c->compareBoundary(cycle))
				{
					newCell = false;
					break;
				}
			}
			// create the new cell and break
			if (newCell)
			{
				WorldCell* wc = new WorldCell(_roadGraph, _simpleRoadGraph, cycle, _viewMode);
				wc->setGenParams(gp);
				_cellVec.push_back(wc);
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
	onCloseDoc();
	createScene();
	_worldTerrain.load(_sceneManager);
	_tools[_activeTool]->activate();
	_isDocOpen = true;
	Refresh();
}

void WorldFrame::onCloseDoc()
{
	if (_isDocOpen)
	{
		_isDocOpen = false;
		_highlightedNode = 0;
		_highlightedRoad = 0;
		_highlightedCell = 0;
		_selectedNode = 0;
		_selectedRoad = 0;
		_selectedCell = 0;
		_tools[_activeTool]->deactivate();

		// destroy scene data
		destroyScene();

		// destroy graph data
		_simpleRoadGraph.clear();
		_roadGraph.clear();

		Refresh();
	}
}

bool WorldFrame::plotPointOnTerrain(Real x, Real &y, Real z)
{
	//create scene doc using doc.x -> x, doc.y -> z  and y from ray cast
	Ray verticalRay(Vector3(x, 5000.0f, z), Vector3::NEGATIVE_UNIT_Y);
	RaySceneQuery* ray = _sceneManager->createRayQuery(verticalRay);
	RaySceneQueryResult result = ray->execute();

	for (RaySceneQueryResult::const_iterator itr = result.begin(); itr
			!= result.end(); itr++)
	{
		if (itr->worldFragment)
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
	if (plotPointOnTerrain(tmp.x, tmp.y, tmp.z))
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
	WorldNode* wn = getSelectedNode();
	if (wn)
	{
		Vector2 pos2D(pos.x, pos.z);
		wn->move(pos2D);
		updateProperties();
	}
}

bool WorldFrame::pickCell(wxMouseEvent& e, WorldCell *&wc)
{
	Vector3 pos3D;
	if (pickTerrainIntersection(e, pos3D))
	{
		BOOST_FOREACH(WorldCell* c, _cellVec)
		{
			if (c->isInside(Vector2(pos3D.x, pos3D.z)))
			{
				wc = c;
				return true;
			}
		}
	}
	return false;
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
	for (RaySceneQueryResult::const_iterator itr = result.begin(); itr
			!= result.end(); itr++)
	{
		if (itr->movable && itr->movable->getName().substr(0, 4) == "road")
		{
			vector<WorldRoad*>::iterator rIt, rEnd = _roadVec.end();
			for (rIt=_roadVec.begin(); rIt != rEnd; rIt++)
			{
				if ((*rIt)->getSceneNode()
						== itr->movable->getParentSceneNode())
				{
					wr = (*rIt);
					return true;
				}
			}
		}
	}
	return false;
}

void WorldFrame::selectCell(WorldCell* wn)
{
	if(_selectedCell) _selectedCell->setSelected(false);
	if(wn) wn->setSelected(true);
	_selectedCell = wn;
	updateProperties();
}

void WorldFrame::selectNode(WorldNode* wn)
{
	if(_selectedNode) _selectedNode->setSelected(false);
	if(wn) wn->setSelected(true);
	_selectedNode = wn;
	updateProperties();
}

void WorldFrame::selectRoad(WorldRoad* wr)
{
	if(_selectedRoad) _selectedRoad->setSelected(false);
	if(wr) wr->setSelected(true);
	_selectedRoad = wr;
	updateProperties();
}

void WorldFrame::setViewMode(WorldCell::Display mode)
{
	_viewMode = mode;
	BOOST_FOREACH(WorldCell* cell, _cellVec)
		cell->setDisplayMode(_viewMode);
	Refresh();
}

template<> WorldFrame* Ogre::Singleton<WorldFrame>::ms_Singleton = 0;
WorldFrame* WorldFrame::getSingletonPtr(void)
{
	return ms_Singleton;
}
WorldFrame& WorldFrame::getSingleton(void)
{
	assert(ms_Singleton);
	return (*ms_Singleton);
}

void WorldFrame::updateTerrain()
{ 
	_worldTerrain.load(_sceneManager); 
	BOOST_FOREACH(WorldNode* wn, _nodeVec) wn->setPosition2D(wn->getPosition2D());
	BOOST_FOREACH(WorldRoad* wr, _roadVec) wr->invalidate();
	BOOST_FOREACH(WorldCell* c, _cellVec) c->invalidate();
	Refresh();
}
