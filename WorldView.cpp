// Includes //
#include "stdafx.h"
#include "WorldView.h"


// Namespace //
using namespace Ogre;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

// Event Table //
BEGIN_EVENT_TABLE(WorldView, wxWindow)
	EVT_ERASE_BACKGROUND(WorldView::onEraseBackground) 
	EVT_KILL_FOCUS(WorldView::onLostFocus)
	EVT_MOUSE_EVENTS(WorldView::onMouse)
//	EVT_PAINT(WorldView::onPaint)
	EVT_SET_FOCUS(WorldView::onSetFocus)
	EVT_SIZE(WorldView::onSize)
	EVT_TIMER(ID_RENDERTIMER, WorldView::onTimer)
END_EVENT_TABLE()


// Constructor //
WorldView::WorldView(wxFrame* parent) 
	: wxControl(parent, -1),
	  mTimer(this, ID_RENDERTIMER)
{
	mCamera = 0;
	mRenderWindow = 0;
	mSceneMgr = 0;
	mViewport = 0;
	
	mCurrentObject = 0;
	mViewMode = normal;
	
	// Create a new parameters list according to compiled OS
	Ogre::NameValuePairList params;
#ifdef WIN32
	params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)GetHandle());
#else
	// TODO: Replace the HWND with Linux or OSX compatible window handler types
#endif

	// Get wx control window size
	int width, height;
	GetSize(&width, &height);
	// Create the render window
	mRenderWindow = Ogre::Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);

	//Init Scene Stuff
	chooseSceneManager();
    createCamera();
    createViewports();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	createScene();

	// Create our ray query
	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());
}


// Destructor //
WorldView::~WorldView()
{
	// destroy Viewport and RenderWindow
	if (mViewport)
	{
		mRenderWindow->removeViewport(mViewport->getZOrder());
		mViewport = 0;
	}

	Ogre::Root::getSingleton().detachRenderTarget(mRenderWindow);
	mRenderWindow = 0;
}

void WorldView::addNode(float x, float y)
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


void WorldView::chooseSceneManager(void)
{
    // Create the SceneManager, in this case a generic one
    mSceneMgr = Ogre::Root::getSingleton().createSceneManager("TerrainSceneManager");
}

void WorldView::createCamera(void)
{    
	// Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(825,175,825);
    mCamera->setOrientation(Quaternion(-0.49, 0.17, 0.81, 0.31));

    // Look back along -Z
    //mCamera->lookAt(Vector3(0,-10,0));
	mCamera->setNearClipDistance( 1 );
    mCamera->setFarClipDistance( 1000 );
}


void WorldView::createScene(void) 
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
	ColourValue fadeColour(0.93, 0.86, 0.76);
	mSceneMgr->setFog( FOG_LINEAR, fadeColour, .001, 500, 1000);
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

void WorldView::createViewports(void)
{
    // Create one viewport, entire window
    mViewport = mRenderWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));
}

void WorldView::deleteSelectedNode()
{
	if(mCurrentObject) {
		mSceneMgr->destroySceneNode(mCurrentObject->getName());
		mCurrentObject = 0;
	}
}

void WorldView::destroyScene(void)
{
}


// Moves the view //
void WorldView::moveView(Real x, Real y, Real z)
{
	mCamera->moveRelative(Vector3(x, y, z));
}

void WorldView::onLeftPressed(wxMouseEvent &e)
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
            ent = mSceneMgr->createEntity( name, "Node.mesh" );
			ent->setMaterialName("Examples/Hilite/Yellow");
            mCurrentObject = mSceneMgr->getRootSceneNode( )->createChildSceneNode( String(name) + "Node", itr->worldFragment->singleIntersection );
            mCurrentObject->attachObject( ent );
            mCurrentObject->setScale( 0.1f, 0.1f, 0.1f );
            break;
        }
	}
	
	// Turn on bounding box.
    if ( mCurrentObject )
    	mCurrentObject->showBoundingBox( true );
}

void WorldView::onLostFocus(wxFocusEvent& e)
{
	//Should tidy up my dragging logic here
}

void WorldView::onMouse(wxMouseEvent &e)
{
	// Camera controls
	if(e.Dragging())
	{
		// Compute deltas
		long	delta_x = e.m_x - mMouseX,
				delta_y = e.m_y - mMouseY;

		// Dolly, orient or pan?
		if(e.m_leftDown && e.m_rightDown)
			moveView(0.0f, 0.0f, delta_y);
		else if(e.m_leftDown) {
			if(mViewMode == node) onLeftDragged(e);
			else rotateView(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			moveView((Real)(-delta_x), (Real)delta_y, 0.0f);
	} 
	else 
	{
		if(e.m_leftDown) {
			if(mViewMode == node) onLeftPressed(e);
		}
	}

	// Save mouse position (for computing deltas for dragging)
	mMouseX = e.m_x;
	mMouseY = e.m_y;

	// Tell OGRE to redraw.
	update();
}

void WorldView::onPaint(wxPaintEvent &WXUNUSED(e))
{
	wxPaintDC dc(this);
	update();
}

void WorldView::onSetFocus(wxFocusEvent& e)
{
	
}

// Resize //
void WorldView::onSize(wxSizeEvent &e)
{
	// Setting new size;
	int width;
	int height;
	GetSize(&width, &height);
	mRenderWindow->resize( width, height );
	// Letting Ogre know the window has been resized;
	mRenderWindow->windowMovedOrResized();
	// Set the aspect ratio for the new size;
	if (mCamera)
		mCamera->setAspectRatio(Ogre::Real(width) / Ogre::Real(height));

	update();
}

// Timer tick
void WorldView::onTimer(wxTimerEvent &e)
{
	update();
}

void WorldView::onLeftDragged(wxMouseEvent &e)
{
	switch(mViewMode) {
		case normal:

		case node:
			addNode(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()) / float(mViewport->getActualHeight()));
			break;
		default:
			break;
	}
}




void WorldView::onEraseBackground(wxEraseEvent &e)
{
	update();
}

// Rotates the view //
void WorldView::rotateView(Real yaw, Real pitch)
{
	mCamera->yaw(yaw * (mCamera->getFOVy() / mCamera->getAspectRatio() / 320.0f));
	mCamera->pitch(pitch * (mCamera->getFOVy() / 240.0f));
}

void WorldView::setMode(WorldViewMode mode)
{
	mViewMode = mode;
}


// update
void WorldView::update()
{
	Ogre::Root::getSingleton().renderOneFrame();
}
