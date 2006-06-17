// Includes //
#include "stdafx.h"
#include "OgreView.h"


// Namespace //
using namespace Ogre;

enum
{
	TIMER_ID = 500,
};

// Event Table //
BEGIN_EVENT_TABLE(OgreView, wxWindow)
//	EVT_PAINT(OgreView::OnPaint)
	EVT_SIZE(OgreView::OnSize)
	EVT_MOUSE_EVENTS(OgreView::OnMouse)
	EVT_TIMER(TIMER_ID, OgreView::OnTimer)
	EVT_SET_FOCUS(OgreView::OnSetFocus)
	EVT_KILL_FOCUS(OgreView::OnLostFocus)
	EVT_ERASE_BACKGROUND(OgreView::OnEraseBackground) 
END_EVENT_TABLE()

// Constructor //
OgreView::OgreView(wxWindow* parent, const wxPoint &pos, const wxSize &size) 
	 : wxWindow(parent, -1, pos, size, wxFULL_REPAINT_ON_RESIZE)
{
	Ogre::NameValuePairList params;
	params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)GetHandle());

	mOgreRenderWindow = Ogre::Root::getSingleton().createRenderWindow("MageRenderWindow", size.GetWidth(), size.GetHeight(), false, &params);

	mCamera = 0;
	mViewport = 0;
	mCurrentObject = 0;

	mViewMode = normal;

	//Init Scene Stuff
	chooseSceneManager();
    createCamera();
    createViewports();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	createScene();

	// Create our ray query
	mRaySceneQuery = mScene->createRayQuery(Ray());
}


// Destructor //
OgreView::~OgreView()
{
}

// Resize //
void OgreView::OnSize(wxSizeEvent &e)
{
	if (mCamera)
	{
		wxSize s = this->GetClientSize();

		mCamera->setAspectRatio(float(s.GetWidth()) / float(s.GetHeight()));
		mOgreRenderWindow->windowMovedOrResized();
	}
}

void OgreView::OnPaint(wxPaintEvent &WXUNUSED(e))
{
	wxPaintDC dc(this);
	Ogre::Root::getSingleton().renderOneFrame();
}


// Update
void OgreView::Update()
{
	Ogre::Root::getSingleton().renderOneFrame();
}



// Rotates the view //
void OgreView::RotateView(Real yaw, Real pitch)
{
	mCamera->yaw(yaw * (mCamera->getFOVy() / mCamera->getAspectRatio() / 320.0f));
	mCamera->pitch(pitch * (mCamera->getFOVy() / 240.0f));
}

// Moves the view //
void OgreView::MoveView(Real x, Real y, Real z)
{
	mCamera->moveRelative(Vector3(x, y, z));
}

void OgreView::chooseSceneManager(void)
{
    // Create the SceneManager, in this case a generic one
    mScene = Ogre::Root::getSingleton().createSceneManager("TerrainSceneManager");
}

void OgreView::createCamera(void)
{    
	// Create the camera
    mCamera = mScene->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(825,175,825);
    mCamera->setOrientation(Quaternion(-0.49, 0.17, 0.81, 0.31));

    // Look back along -Z
    //mCamera->lookAt(Vector3(0,-10,0));
	mCamera->setNearClipDistance( 1 );
    mCamera->setFarClipDistance( 1000 );
}


void OgreView::createScene(void) 
{	
    Plane waterPlane;

    // Set ambient light
    mScene->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

    // Create a light
    Light* l = mScene->createLight("MainLight");
    // Accept default settings: point light, white diffuse, just set position
    // NB I could attach the light to a SceneNode if I wanted it to move automatically with
    //  other objects, but I don't
    l->setPosition(20,80,50);

    // Fog
    // NB it's VERY important to set this before calling setWorldGeometry 
    // because the vertex program picked will be different
	ColourValue fadeColour(0.93, 0.86, 0.76);
	mScene->setFog( FOG_LINEAR, fadeColour, .001, 500, 1000);
	mOgreRenderWindow->getViewport(0)->setBackgroundColour(fadeColour);

    std::string terrain_cfg("terrain.cfg");
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        terrain_cfg = mResourcePath + terrain_cfg;
#endif
        mScene -> setWorldGeometry( terrain_cfg );
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

void OgreView::destroyScene(void)
{
}

void OgreView::createViewports(void)
{
    // Create one viewport, entire window
    mViewport = mOgreRenderWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));
}

void OgreView::OnLeftPressed(wxMouseEvent &e)
{
	// Turn off bounding box.
    if ( mCurrentObject )
    	mCurrentObject->showBoundingBox( false );
    	
    // Setup the ray scene query
    Ray mouseRay = mCamera->getCameraToViewportRay(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()) / float(mViewport->getActualHeight()) );
	mRaySceneQuery->setRay( mouseRay );
    mRaySceneQuery->setSortByDistance( true );
    
    // Execute query
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr = result.begin( );

	// Get results, create a node/entity on the position
	for( itr = result.begin( ); itr != result.end(); itr++ )
	{
		if ( itr->movable && itr->movable->getName().substr(0, 5) != "tile[" )
        {
        	mCurrentObject = itr->movable->getParentSceneNode( );
            break;
        }
        else if ( itr->worldFragment )
        {
            Entity *ent;
            char name[16];
            sprintf( name, "Robot%d", mCount++ );
            ent = mScene->createEntity( name, "Node.mesh" );
			ent->setMaterialName("Examples/Hilite/Yellow");
            mCurrentObject = mScene->getRootSceneNode( )->createChildSceneNode( String(name) + "Node", itr->worldFragment->singleIntersection );
            mCurrentObject->attachObject( ent );
            mCurrentObject->setScale( 0.1f, 0.1f, 0.1f );
            break;
        }
	}
	
	// Turn on bounding box.
    if ( mCurrentObject )
    	mCurrentObject->showBoundingBox( true );
}

void OgreView::OnMouse(wxMouseEvent &e)
{
	// Camera controls
	if(e.Dragging())
	{
		// Compute deltas
		long	delta_x = e.m_x - mMouseX,
				delta_y = e.m_y - mMouseY;

		// Dolly, orient or pan?
		if(e.m_leftDown && e.m_rightDown)
			MoveView(0.0f, 0.0f, delta_y);
		else if(e.m_leftDown) {
			if(mViewMode == node) OnLeftDragged(e);
			else RotateView(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			MoveView((Real)(-delta_x), (Real)delta_y, 0.0f);
	} 
	else 
	{
		if(e.m_leftDown) {
			if(mViewMode == node) OnLeftPressed(e);
		}
	}

	// Save mouse position (for computing deltas for dragging)
	mMouseX = e.m_x;
	mMouseY = e.m_y;

	// Tell OGRE to redraw.
	Update();
}

void OgreView::OnSetFocus(wxFocusEvent& e)
{
	
}

void OgreView::OnLostFocus(wxFocusEvent& e)
{
	
}

// Timer tick
void OgreView::OnTimer(wxTimerEvent &e)
{
	Update();
}

void OgreView::OnLeftDragged(wxMouseEvent &e)
{
	switch(mViewMode) {
		case normal:

		case node:
			AddNode(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()) / float(mViewport->getActualHeight()));
			break;
		default:
			break;
	}
}

void OgreView::AddNode(float x, float y)
{
	Ray mouseRay = mCamera->getCameraToViewportRay(x,y);
    mRaySceneQuery->setRay( mouseRay );
    mRaySceneQuery->setSortByDistance( false );

    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr;

    for ( itr = result.begin( ); itr != result.end(); itr++ )
    {
        if ( itr->worldFragment )
        {
            mCurrentObject->setPosition( itr->worldFragment->singleIntersection );
            break;
        }
    }
}


void OgreView::OnEraseBackground(wxEraseEvent &e)
{
	Update();
}


void OgreView::DeleteSelectedNode()
{
	if(mCurrentObject) {
		mScene->destroySceneNode(mCurrentObject->getName());
		mCurrentObject = 0;
	}
}

void OgreView::SetMode(OgreViewMode mode)
{
	mViewMode = mode;
}
/*
void OgreView::AddEdge() {
 Line3D *myLine = new Line3D();
   myLine->addPoint(Vector3(0.0, 9.6, 0.0));
   myLine->addPoint(Vector3(160.0, 9.6, 0.0));
   myLine->addPoint(Vector3(160.0, 9.6, 160.0));
   myLine->addPoint(Vector3(0.0, 9.6, 160.0));
   myLine->addPoint(Vector3(0.0, 9.6, 0.0));
   myLine->drawLines();

   SceneNode *myNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
   myNode->attachObject(myLine);
}*/