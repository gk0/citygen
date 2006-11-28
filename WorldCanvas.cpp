#include "stdafx.h"
#include "WorldCanvas.h"
#include "WorldDocument.h"
#include "MoveableText.h"

using namespace Ogre;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

#define CRAZY_MOUSE_OFFSET 0	//this really should be reqd. but is here until i find out whats going on


BEGIN_EVENT_TABLE(WorldCanvas, wxOgre)
	EVT_CHAR(WorldCanvas::OnChar)
    EVT_MOUSE_EVENTS(WorldCanvas::OnMouse)
//	EVT_ERASE_BACKGROUND(WorldCanvas::onEraseBackground) 
//	EVT_KILL_FOCUS(WorldCanvas::onLostFocus)
//	EVT_PAINT(WorldCanvas::onPaint)
//	EVT_SET_FOCUS(WorldCanvas::onSetFocus)
//	EVT_SIZE(WorldCanvas::onSize)
//	EVT_TIMER(ID_RENDERTIMER, WorldCanvas::onTimer)
END_EVENT_TABLE()


// Define a constructor for my canvas
WorldCanvas::WorldCanvas(wxView *v, wxFrame *frame, NodePropertyPage* nProp, CellPropertyPage* cProp):
    wxOgre(frame),
	mNodePropertyPage(nProp),
	mCellPropertyPage(cProp)
{

    view = v;
	mNodePropertyPage->setCanvas(this);
	mCellPropertyPage->setCanvas(this);

	mCurrentNode = 0;
	mRoadNode = 0;
	mEditMode = EditModeListener::view;
	mSelectMode = sel;
	mNodeCount = mRoadCount = 0;
	mManualObject = 0;
	prepared = false;
}

void WorldCanvas::createScene(void) 
{	
    Plane waterPlane;

    // Set ambient light
    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    // Create a light
    Light* l = mSceneMgr->createLight("MainLight");
    // Accept default settings: point light, white diffuse, just set position
    // NB I could attach the light to a SceneNode if I wanted it to move automatically with
    //  other objects, but I don't
    l->setPosition(20,80,50);

    // Fog
    // NB it's VERY important to set this before calling setWorldGeometry 
    // because the vertex program picked will be different
	fadeColour = ColourValue(0.76f, 0.86f, 0.93f);
	mSceneMgr->setFog( FOG_LINEAR, fadeColour, .001f, 500, 1000);
	mRenderWindow->getViewport(0)->setBackgroundColour(fadeColour);

    std::string terrain_cfg("terrain.cfg");
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        terrain_cfg = mResourcePath + terrain_cfg;
#endif
    mSceneMgr -> setWorldGeometry( terrain_cfg );

    // Infinite far plane?
    if (Ogre::Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
    {
        mCamera->setFarClipDistance(0);
    }

    // Define the required skyplane
    Plane plane;
    // 5000 world units from the camera
    plane.d = 5000;
    // Above the camera, facing down
    plane.normal = -Vector3::UNIT_Y;
}

/*
 * The idea behind this function is to initialise the OgreView in a 
 * default state and set it up with default assets and camera etc.
 */
void WorldCanvas::prepare()
{
	//Ensure run once - reqd because we want to get called by view->OnCreate
	//maybe tidy this up some day
	if(prepared) return;
	prepared = true;

	Init();

	// Create our ray query
	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());

	// Create the texture
	TexturePtr texture = TextureManager::getSingleton().createManual(
		"DynamicTexture", // name
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D,      // type
		1024, 1024,         // width & height
		0,                // number of mipmaps
		PF_BYTE_BGRA,     // pixel format
		TU_RENDERTARGET);      // usage; should be TU_DYNAMIC_WRITE_ONLY_DISCARDABLE for
						  // textures updated very often (e.g. each frame)

	/* 
	I only need this to create a partially transparent 
	texture to test alpha blend.

	// Get the pixel buffer
	HardwarePixelBufferSharedPtr pixelBuffer = texture->getBuffer();

	// Lock the pixel buffer and get a pixel box
	pixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
	const PixelBox& pixelBox = pixelBuffer->getCurrentLock();
	uint8* pDest = static_cast<uint8*>(pixelBox.data);

	// Fill in some pixel data. This will give a semi-transparent blue,
	// but this is of course dependent on the chosen pixel format.
	for (size_t j = 0; j < 512; j++)
		for(size_t i = 0; i < 512; i++)
		{
			*pDest++ = 255; // B
			*pDest++ =   0; // G
			*pDest++ =   0; // R
			*pDest++ = 32; // A
		}
	// Unlock the pixel buffer
	pixelBuffer->unlock();
	*/



	RenderTarget *rttTex = texture->getBuffer()->getRenderTarget();
    {
        mRTTCam = mSceneMgr->createCamera("ReflectCam");
        mRTTCam->setNearClipDistance(mCamera->getNearClipDistance());
        mRTTCam->setFarClipDistance(mCamera->getFarClipDistance());
        mRTTCam->setAspectRatio(
            (Real)mRenderWindow->getViewport(0)->getActualWidth() / 
            (Real)mRenderWindow->getViewport(0)->getActualHeight());

		////Test Values 
		mRTTCam->setPosition(905, 238,880);
		mRTTCam->setOrientation(Quaternion(-0.429f, 0.276f, 0.714f, 0.501f));

        Viewport *v = rttTex->addViewport( mRTTCam );
        v->setClearEveryFrame( true );
		v->setShadowsEnabled( false );
		v->setSkiesEnabled( false );
		v->setBackgroundColour( ColourValue::ZERO ); //I think this be the magic transparent line

		//Test Values 
		//mRTTCam->setPosition(905, 238, 880);
		//mRTTCam->setOrientation(Quaternion(-0.429f, 0.276f, 0.714f, 0.501f));
		//mRTTCam->setFixedYawAxis(false);
		//mRTTCam->setProjectionType(PT_ORTHOGRAPHIC);
		MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName("gk/Terrain");
		TextureUnitState* t = mat->getTechnique(0)->getPass(1)->createTextureUnitState("DynamicTexture");
		mat->getTechnique(0)->getPass(0)->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		t->setProjectiveTexturing(true, mRTTCam);  //let sproject our texture onto the object (yes, like a projector)
		rttTex->addListener(this);
    }

	//Make big cube or plane to view the tex on
	//Entity *ent = mSceneMgr->createEntity("cubey", "cube.mesh");
	//ent->setMaterialName("RttMat");
	//testo = mSceneMgr->getRootSceneNode( )->createChildSceneNode();
	//testo->attachObject( ent );
	//testo->setScale(1, 0.01f, 1);
	//testo->setPosition(1017,16,963);
}

void WorldCanvas::OnDraw(wxDC& dc)
{
    if (view)
        view->OnDraw(& dc);
}

void WorldCanvas::moveCurrentNode(wxMouseEvent &e)
{
	if(!mCurrentNode) return;

	RaySceneQueryResult rayResult = doMouseRay(e);
	Vector3* vec = getTerrainIntersection(rayResult);
	if(vec) 
		moveNode(mCurrentNode, *vec);
}

void WorldCanvas::moveNode(SceneNode* sn, const Vector3& pos)
{	
	// Find any roads that link to the node and update them too
	WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
	if(!doc) return;

	// update position of node
	doc->moveNode(mSceneNodeMap[sn].nodeDesc, convert3DPointTo2D(pos));

	// update Node
    mCurrentNode->setPosition(pos);

	// update my property display
	mNodePropertyPage->updateData(sn->getName(), pos.x, pos.y, pos.z);

	// update position of all connnected roads
	RoadIterator2 rit, rend;
	boost::tie(rit, rend) = doc->getRoadsFromNode(mSceneNodeMap[mCurrentNode].nodeDesc);
	for(; rit!=rend; rit++)
	{
		Ogre::SceneNode* sn = static_cast<Ogre::SceneNode*>(doc->getRoadData1(*rit));
		if(sn) 
		{
			// destroy road in scene 
			std::string name = sn->getName();
			mSceneMgr->destroySceneNode(name);

			// create a new road from the doc road
			WorldNode* sourceNode = static_cast< WorldNode* >(doc->getNodeData1(doc->getRoadSource(*rit)));
			WorldNode* targetNode = static_cast< WorldNode* >(doc->getNodeData1(doc->getRoadTarget(*rit)));
			Ogre::Vector3 sourcePos = sourceNode->getPosition();
			Ogre::Vector3 targetPos = targetNode->getPosition();

			SceneNode* roadNode = createRoad(sourcePos, targetPos, name);
			doc->setRoadData1(*rit, roadNode);
			mSceneRoadMap[roadNode] = *rit;
		}
	}
}


void WorldCanvas::OnSelectNode(wxMouseEvent &e)
{
	// Turn off bounding box.
    if(mCurrentNode)
    	mSceneNodeMap[mCurrentNode].worldNode->showBoundingBox(false);

	RaySceneQueryResult rayResult = doMouseRay(e);

	switch(mSelectMode) 
	{
	case sel:
		mCurrentNode = getMovable(rayResult, "Node");
		if(mCurrentNode)
			selectNode(mCurrentNode);
		break;
	case del:
		mCurrentNode = getMovable(rayResult, "Node");
		if(mCurrentNode) deleteNode(mCurrentNode);
		break;
	case add:
		Vector3* intersection = getTerrainIntersection(rayResult);
		if(intersection) createNode(*intersection);
		break;
	}
	
	// Turn on bounding box.
    if(mCurrentNode)
    	mSceneNodeMap[mCurrentNode].worldNode->showBoundingBox(true);
}

void WorldCanvas::clear()
{
	// Remove nodes from the scene
	SceneNodeMap::const_iterator ni, nend;
	for (ni = mSceneNodeMap.begin(), nend = mSceneNodeMap.end(); ni != nend; ++ni)
	{
		delete ni->second.worldNode;
	}
	mCurrentNode = 0;

	// Remove roads from the scene
	SceneRoadMap::const_iterator ri, rend;
	for (ri = mSceneRoadMap.begin(), rend = mSceneRoadMap.end(); ri != rend; ++ri)
	{
		// get scene pointer
		SceneNode* sn(ri->first);
		mSceneMgr->destroyManualObject(sn->getName());
		mSceneMgr->destroySceneNode(sn->getName());
	}
	mRoadNode = 0;
	mRoadCount = 0;  

	mNodeCount = 0;
	mSceneRoadMap.clear();
	mSceneNodeMap.clear();
}

void WorldCanvas::Refresh()
{
	wxOgre::Refresh();
}

void WorldCanvas::loadDoc() 
{
	WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
	if(!doc) return;

	// clear existing data
	//clear();
	
	// create nodes in the scene
	NodeIterator ni, nend;
	for (boost::tie(ni, nend) = doc->getNodes(); ni != nend; ++ni)
	{
		//create scene doc using doc.x -> x, doc.y -> z  and y from ray cast
		Vector2 loc(doc->getNodePosition(*ni));
		Ray verticalRay(Vector3(loc.x, 5000.0f, loc.y), Vector3::NEGATIVE_UNIT_Y );
		mRaySceneQuery->setRay(verticalRay);
		RaySceneQueryResult rayResult = mRaySceneQuery->execute();

		Vector3* intersection = getTerrainIntersection(rayResult);
		if(intersection) {
			WorldNode* wn = createNode(*intersection);
			doc->setNodeData1(*ni, wn);
			NodePair np;
			np.worldNode = wn;
			np.nodeDesc = *ni;
			mSceneNodeMap[wn->getSceneNode()] = np;
		}
		else
		{
			//TODO:
			// -should really thorugh an error or at least alert user
			assert(true);
		}
	}

	// create roads in the scene
	RoadIterator ri, rend;
	for (boost::tie(ri, rend) = doc->getRoads(); ri != rend; ++ri)
	{
		NodeDescriptor nd1 = doc->getRoadSource(*ri);
		NodeDescriptor nd2 = doc->getRoadTarget(*ri);
		WorldNode* sn1 = static_cast<WorldNode*>(doc->getNodeData1(nd1));
		WorldNode* sn2 = static_cast<WorldNode*>(doc->getNodeData1(nd2));
		SceneNode* rn = createRoad(sn1->getPosition(), sn2->getPosition());
		doc->setRoadData1(*ri, rn);
		mSceneRoadMap[rn] = *ri;
	}
}

WorldNode* WorldCanvas::createNode(const Vector3& pos)
{
	std::stringstream oss;
	oss << "Node" << mNodeCount++;
	String name(oss.str());
	return new WorldNode(mSceneMgr, name, name.substr(4,name.length()), pos);
}

SceneNode* WorldCanvas::createRoad(const Vector3& u, const Vector3& v) 
{
	std::stringstream oss;
	oss << "manual" << mRoadCount++;
	return createRoad(u, v, oss.str());
}

SceneNode* WorldCanvas::createRoad(const Vector3& u, const Vector3& v, const std::string& name) 
{
	//omg i should like draw a line from my old node to my new node
	ManualObject* myManualObject = new ManualObject(name); 
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode(name); 

	/* Ogre::RenderOperation::OT_LINE_LIST*/
	myManualObject->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
	Vector3 offset(0,3,0);
	myManualObject->position(u+offset); 
	myManualObject->position(v+offset); 
	// etc 
	myManualObject->end(); 

	node->attachObject(myManualObject);
	//mManualObject = myManualObjectNode;
	return node;
}

void WorldCanvas::OnSelectRoad(wxMouseEvent &e)
{
	SceneNode* tmp = 0;

	// Turn off bounding box.
    if ( mRoadNode )
    	mRoadNode->showBoundingBox( false );

	RaySceneQueryResult rayResult = doMouseRay(e);

	switch(mSelectMode) 
	{
	case sel:
		mRoadNode = getMovable(rayResult, "Node");
		break;
	case del:
		tmp = getMovable(rayResult, "Node");
		if(tmp)
		{
			if(mRoadNode) 
			{
				WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
				if(doc) 
				{
					RoadDescriptor rd;
					if(doc->findRoad(mSceneNodeMap[mRoadNode].nodeDesc, mSceneNodeMap[tmp].nodeDesc, rd)) 
					{
						SceneNode* road = static_cast<Ogre::SceneNode*>(doc->getRoadData1(rd));
						mSceneMgr->destroySceneNode(road->getName());
						doc->removeRoad(mSceneNodeMap[mRoadNode].nodeDesc, mSceneNodeMap[tmp].nodeDesc);
						mSceneRoadMap.erase(road);
					}
				}
			}
		}
		else
		{
			mRoadNode = 0;
		}
		break;
		
	case add:
		tmp = getMovable(rayResult, "Node");
		if(tmp) 
		{
			if(mRoadNode)
			{
				WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
				RoadDescriptor rd;
				if(!doc->findRoad(mSceneNodeMap[mRoadNode].nodeDesc, mSceneNodeMap[tmp].nodeDesc, rd)) 
				{
					SceneNode* sn = createRoad(mRoadNode->getPosition(), tmp->getPosition());
					// Shit check road exists
					RoadDescriptor rd = doc->createRoad(mSceneNodeMap[mRoadNode].nodeDesc, mSceneNodeMap[tmp].nodeDesc, sn);
					mSceneRoadMap[sn] = rd;
				}
			}
			mRoadNode = tmp;
		}
		else
		{
			mRoadNode = 0;
		}
		break;
	}
	
	// Turn on bounding box.
    if ( mRoadNode )
    	mRoadNode->showBoundingBox( true );

	//mWorldDoc.printRoads();
}


void WorldCanvas::OnLostFocus(wxFocusEvent& e)
{
	//Should tidy up my dragging logic here
}

void WorldCanvas::OnMouse(wxMouseEvent &e)
{
	if(!view) return;

	//If you click on me get back focus
	//focus should really be assigned by what your mouse is over but until then...
	if(e.m_leftDown) {
		this->SetFocusFromKbd();
		this->SetFocus();
	}

	//Check for mouse wheel scroll
	if (e.GetWheelRotation() != 0)
	{
		Vector3 mTranslateVector(0,0,0);
		mTranslateVector.z = e.GetWheelRotation() / 10;
		mCamera->moveRelative(mTranslateVector);
		//DEBUG crap
		//std::stringstream oss;
		//oss <<"Camera position: "<<mCamera->getPosition();
		//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
		//mCamera-> e.GetWheelRotation()
	}


	// Camera controls
	if(e.Dragging())
	{
		// Compute deltas
		long	delta_x = e.m_x - mMouseX,
				delta_y = e.m_y - mMouseY;

		// Dolly, orient or pan?
		if(e.m_leftDown && e.m_rightDown)
			cameraMove(0.0f, 0.0f, delta_y);
		else if(e.m_leftDown) {
			if(mEditMode == node) OnLeftDragged(e);
			else cameraRotate(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			cameraMove((Real)(-delta_x), (Real)delta_y, 0.0f);
	} 
	else 
	{
		if(e.m_leftDown) {
			switch(mEditMode) 
			{
			case node:
				OnSelectNode(e);
				break;
			case edge:
				OnSelectRoad(e);
				break;
			case cell:
				OnSelectCell(e);
				break;
			default:
				break;
			}
		}
	}

	// Save mouse position (for computing deltas for dragging)
	mMouseX = e.m_x;
	mMouseY = e.m_y;

	// Tell OGRE to redraw.
	Update();
}

void WorldCanvas::OnSetFocus(wxFocusEvent& e)
{
	
}

void WorldCanvas::OnLeftDragged(wxMouseEvent &e)
{
	switch(mEditMode) {
		case EditModeListener::view:

		case EditModeListener::node:
			if(mSelectMode ==sel)
			{
				moveCurrentNode(e);
			}
			break;
		default:
			break;
	}
}

void WorldCanvas::setEditMode(EditModeListener::EditMode mode) {
	mEditMode = mode;
	switch(mode) {
	case node:
		if(mRoadNode) mRoadNode->showBoundingBox(false);
		if(mCurrentNode) mCurrentNode->showBoundingBox(true);
		break;
	case edge:
		if(mCurrentNode) mCurrentNode->showBoundingBox(false);
		if(mRoadNode) mRoadNode->showBoundingBox(true);
		break;
	case cell:
		if(mRoadNode) mRoadNode->showBoundingBox(false);
		if(mCurrentNode) mCurrentNode->showBoundingBox(false);
		//ok lets show the cells
		WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
		if(doc)
		{
			doc->divideCells();

			//delete any inner roads on our 
			std::map< CityCell*, SceneNode*>::iterator csIt, csEnd;
			for(csIt = mCellSceneMap.begin(), csEnd = mCellSceneMap.end(); csIt != csEnd; csIt++)
			{
				Ogre::SceneNode* sn = csIt->second;
				mSceneMgr->destroyManualObject(sn->getName());
				mSceneMgr->destroySceneNode(sn->getName());
			}
			mCellSceneMap.clear();
			//mCellSceneMap[cellPtr]

			CityCellIterator ccIt, ccEnd;
			for(boost::tie(ccIt, ccEnd) = doc->getCells(); ccIt != ccEnd; ccIt++)
			{
				// do the generate
				ccIt->growthGenerate();
				createCell(&(*ccIt));
			}
		}

		break;
	}
	Update();
}

void WorldCanvas::setSelectMode(SelectModeListener::SelectMode mode) {
	mSelectMode = mode;
}

void WorldCanvas::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	
	//http://www.ogre3d.org/phpBB2/viewtopic.php?p=164361#164361
	mSceneMgr->setWorldGeometryRenderQueue(RENDER_QUEUE_WORLD_GEOMETRY_1); 

    // Hide terrain
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->addSpecialCaseRenderQueue(mSceneMgr->getWorldGeometryRenderQueue());
	mSceneMgr->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_EXCLUDE);

	mSceneMgr->setFog(FOG_NONE, fadeColour, .001f, 500, 1000);

	//if(testo) testo->setVisible(false);

	// Turn off bounding box.
    if(mCurrentNode)
    	mCurrentNode->showBoundingBox( false );

	// Turn off bounding box.
    if(mRoadNode)
		mRoadNode->showBoundingBox( false );

	if(view) {
		// Show roads on the scene
/*		SceneRoadMap::const_iterator ri, rend;
		for (ri = mSceneRoadMap.begin(), rend = mSceneRoadMap.end(); ri != rend; ++ri) 
			ri->first->setVisible(true);
*/
		// Hide nodes on the scene
		SceneNodeMap::const_iterator ni, nend;
		for (ni = mSceneNodeMap.begin(), nend = mSceneNodeMap.end(); ni != nend; ++ni) 
			ni->second.worldNode->setVisible(false);
	}
}

void WorldCanvas::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->setFog( FOG_LINEAR, fadeColour, .001f, 500, 1000);

	//if(testo) testo->setVisible(true);
	
	switch(mEditMode) {
		case node:
			if(mCurrentNode) mCurrentNode->showBoundingBox(true);
			break;
		case edge:
			if(mRoadNode) mRoadNode->showBoundingBox(true);
			break;
	}

	if(view) {
		// Hide roads on the scene
/*		SceneRoadMap::const_iterator ri, rend;
		for (ri = mSceneRoadMap.begin(), rend = mSceneRoadMap.end(); ri != rend; ++ri) 
			ri->first->setVisible(false);
*/
		// Show nodes on the scene
		SceneNodeMap::const_iterator ni, nend;
		for (ni = mSceneNodeMap.begin(), nend = mSceneNodeMap.end(); ni != nend; ++ni) 
			ni->second.worldNode->setVisible(true);
	}
	
}

void WorldCanvas::OnChar(wxKeyEvent& event)
{
	WorldDocument* doc = 0;
	int key = event.KeyCode();
	switch(key){
		case WXK_UP:
			cameraMove(0.0f, 0.0f, -4.0f);
			Update();
			break;
		case WXK_DOWN:
			cameraMove(0.0f, 0.0f, 4.0f);
			Update();
			break;
		case WXK_LEFT:
			cameraMove(-4.0f, 0.0f, 0.0f);
			Update();
			break;
		case WXK_RIGHT:
			cameraMove(4.0f, 0.0f, 0.0f);
			Update();
			break;
		case 'c':
		case 'C':
			doc = static_cast<WorldDocument*>(view->GetDocument());
			doc->divideCells();
			break;
		default:
			event.Skip();
			break;
	}
}

void WorldCanvas::OnSelectCell(wxMouseEvent &e)
{
	WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
	if(doc)
	{
		// divide network into cells so we have something to pick from
		doc->divideCells();

		// get intersection point from mouse
		RaySceneQueryResult rayResult = doMouseRay(e);
		Vector3* intersection = getTerrainIntersection(rayResult);

		if(intersection)
		{
			bool picked = false;
			CityCell* cell;
			picked = doc->pickCell(convert3DPointTo2D(*intersection), cell);

			if(picked)
			{
				// tell me what you got
				std::stringstream oss;
				oss << "bo selecta: "<< cell;
				LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);

				highlightCell(*cell, doc);
				mCityCell = cell;
				const GrowthGenParams& gp(cell->mGrowthGenParams);
				mCellPropertyPage->updateData(gp.seed, gp.segmentSize, gp.segmentDeviance, gp.degree, 
					gp.degreeDeviance, gp.snapSize, gp.snapDeviance);
			}
		}
	}
}

void WorldCanvas::highlightCell(const CityCell& cell, WorldDocument* doc)
{
	//let's try and delete any existing cell
	SceneNode* node;
	if(mSceneMgr->hasSceneNode("cell"))
	{
		mSceneMgr->destroySceneNode("cell");
		mSceneMgr->destroyManualObject("cell");
	}
	node = mSceneMgr->getRootSceneNode()->createChildSceneNode("cell");

	ManualObject* obj = new ManualObject("cell");
	obj->begin("gk/Hilite/Yellow", Ogre::RenderOperation::OT_LINE_STRIP);
	Vector3 offset(0,3.1,0);
	const Primitive& p(cell.mBoundaryPrimitive);
	Primitive::const_iterator pIt, pEnd;
	for(pIt = p.begin(), pEnd = p.end(); pIt != pEnd; pIt++)
	{
		Vector3 vec3;
		plotPointOnTerrain(doc->getNodePosition(*pIt), vec3);
		obj->position(vec3+offset);
	}
	if(p.begin() != pEnd) 
	{
		Vector3 vec3;
		plotPointOnTerrain(doc->getNodePosition(*(p.begin())), vec3);
		obj->position(vec3+offset);
	}
	obj->end();
	node->attachObject(obj);
}

SceneNode* WorldCanvas::getMovable(const RaySceneQueryResult& result, const std::string& mask) 
{  
	for(RaySceneQueryResult::const_iterator itr = result.begin( ); itr != result.end(); itr++ )
	{
		if ( itr->movable && itr->movable->getName().substr(0, mask.length()) == mask )
		{
    		return itr->movable->getParentSceneNode();
		}
	}
	return (SceneNode*)0;
}

Vector3* WorldCanvas::getTerrainIntersection(const RaySceneQueryResult& result) 
{
	for(RaySceneQueryResult::const_iterator itr = result.begin(); itr != result.end(); itr++ )
	{
		if(itr->worldFragment)
		{
    		return &(itr->worldFragment->singleIntersection);
		}
	}
	return (Vector3*)0;
}

RaySceneQueryResult WorldCanvas::doMouseRay(const wxMouseEvent &e) 
{
	// Setup the ray scene query
	float mouseX = float(1.0f/mViewport->getActualWidth()) * e.GetX();
	float mouseY = float(1.0f/mViewport->getActualHeight()) * (e.GetY() + CRAZY_MOUSE_OFFSET);
	Ray mouseRay = mCamera->getCameraToViewportRay(mouseX, mouseY);

	mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setSortByDistance(true);
	return mRaySceneQuery->execute();
}

bool WorldCanvas::plotPointOnTerrain(const Ogre::Vector2& pos2D, Ogre::Vector3& pos3D) 
{
	//create scene doc using doc.x -> x, doc.y -> z  and y from ray cast
	Ray verticalRay(Vector3(pos2D.x, 5000.0f, pos2D.y), Vector3::NEGATIVE_UNIT_Y );
	mRaySceneQuery->setRay(verticalRay);
	RaySceneQueryResult rayResult = mRaySceneQuery->execute();

	Vector3* intersection = getTerrainIntersection(rayResult);
	if(intersection) {
		pos3D = *intersection;
		return true;
	}
	else
	{
		return false;
	}
}

Ogre::Vector2 WorldCanvas::convert3DPointTo2D(const Ogre::Vector3& pos3D) 
{
	return Ogre::Vector2(pos3D.x, pos3D.z);
}

void WorldCanvas::refreshSceneNodeMap()
{
	WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
	if(!doc) return;

	// clear the scene->node map
	mSceneNodeMap.clear();
	// get a list of nodes from the document
	NodeIterator nodeIt, nodeEnd;
	boost::tie(nodeIt, nodeEnd) = doc->getNodes();
	for(; nodeIt != nodeEnd; nodeIt++)
	{
		// get SceneNode ptr from doc node
		WorldNode* wn = static_cast<WorldNode*>(doc->getNodeData1(*nodeIt));
		NodePair np;
		np.worldNode = wn;
		np.nodeDesc = *nodeIt;

		// insert into the scene->node map
		mSceneNodeMap[wn->getSceneNode()] = np;
	}
}

void WorldCanvas::setNodeProperties(const string& l, const double& x, const double& y, const double& z)
{
	if(mCurrentNode)
	{
		mSceneNodeMap[mCurrentNode].worldNode->setLabel(l);

		//mCurrentNode->
		Vector3 pos(static_cast<Real>(x), static_cast<Real>(y), static_cast<Real>(z));
		if(plotPointOnTerrain(convert3DPointTo2D(pos), pos))
			moveNode(mCurrentNode, pos);
		else
			// no move has taken place because the new location could not be plotted on the terrain
			// -write back the last correct values to the property inspector
			selectNode(mCurrentNode);
		Update();
	}
}

void WorldCanvas::selectNode(SceneNode* sn)
{
	mCurrentNode = sn;
	const Vector3& pos = mSceneNodeMap[mCurrentNode].worldNode->getPosition();
	mNodePropertyPage->updateData(mSceneNodeMap[mCurrentNode].worldNode->getLabel(), pos.x, pos.y, pos.z);
}


void WorldCanvas::deleteNode(SceneNode* sn)
{
	WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
	if(doc) 
	{
		//Delete Roads From Our Scene
		RoadIterator2 rit, rend;
		boost::tie(rit, rend) = doc->getRoadsFromNode(mSceneNodeMap[mCurrentNode].nodeDesc);
		for(; rit!=rend; rit++)
		{
			Ogre::SceneNode* sn = static_cast<Ogre::SceneNode*>(doc->getRoadData1(*rit));
			if(sn) 
			{
				//BUG
				std::string s = sn->getName();
				mSceneMgr->destroySceneNode(s);
				//mSceneMgr->destroySceneNode(sn->getName());
			}
		}
		//Delete the Node
		NodeDescriptor nd = mSceneNodeMap[mCurrentNode].nodeDesc;
		doc->removeNode(nd);
		mSceneNodeMap.erase(mCurrentNode);

		if(mRoadNode == mCurrentNode) mRoadNode = 0;
		mSceneMgr->destroySceneNode(mCurrentNode->getName());
		mCurrentNode = 0;

		// if using vecS for vertices deleting a node will remove
		//We need to update
		//#if() // macro check for vector 
		refreshSceneNodeMap();
	}
}

void WorldCanvas::setCellProperties(const int& seed, const float& segSz, const float& segDev, const int& degree,
									const float& degreeDev, const float& snapSz, const float& snapDev)
{
	if(mCityCell)
	{
		mCityCell->mGrowthGenParams.seed = seed;
		mCityCell->mGrowthGenParams.segmentSize = segSz;
		mCityCell->mGrowthGenParams.segmentDeviance = segDev;
		mCityCell->mGrowthGenParams.degree = degree;
		mCityCell->mGrowthGenParams.degreeDeviance = degreeDev;
		mCityCell->mGrowthGenParams.snapSize = snapSz;
		mCityCell->mGrowthGenParams.snapDeviance = snapDev;

		// destroy 
		string name(mCellSceneMap[mCityCell]->getName());
		mSceneMgr->destroySceneNode(name);
		mSceneMgr->destroyManualObject(name);
		Update();

		mCityCell->growthGenerate();
		createCell(mCityCell);

		Update();
	}
}


void WorldCanvas::createCell(CityCell* cell)
{
	std::stringstream oss;
	oss << "cell" << mRoadCount++;
	ManualObject* myManualObject = new ManualObject(oss.str()); 
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode(oss.str()); 

	/* Ogre::RenderOperation::OT_LINE_LIST*/
	myManualObject->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
	Vector3 offset(0,3,0);

	//better create some roads for these cells.
	RoadIterator ri, rend;
	for(boost::tie(ri, rend) = cell->mRoadGraph.getRoads(); ri != rend; ++ri)
	{
		const Ogre::Vector2& source2(cell->mRoadGraph.getNodePosition(cell->mRoadGraph.getRoadSource(*ri)));
		const Ogre::Vector2& target2(cell->mRoadGraph.getNodePosition(cell->mRoadGraph.getRoadTarget(*ri)));
		Ogre::Vector3 source3, target3;

		if(plotPointOnTerrain(source2, source3) && plotPointOnTerrain(target2, target3))
		{
			myManualObject->position(source3+offset); 
			myManualObject->position(target3+offset); 
		}
	}
	myManualObject->end(); 

	node->attachObject(myManualObject);

	//CityCell* cellPtr = &cell;

	mCellSceneMap[cell] = node;
}