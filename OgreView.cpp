// Includes
#include "stdafx.h"
#include "OgreView.h"


// Namespace 
using namespace Ogre;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

// Required for WX
IMPLEMENT_CLASS(OgreView, wxControl)

// Event Table
BEGIN_EVENT_TABLE(OgreView, wxControl)
	EVT_ERASE_BACKGROUND(OgreView::onEraseBackground) 
	EVT_KILL_FOCUS(OgreView::onFocusLost)
	EVT_SET_FOCUS(OgreView::onFocusSet)
	EVT_MOUSE_EVENTS(OgreView::onMouse)
//	EVT_PAINT(OgreView::onPaint)
	EVT_SIZE(OgreView::onSize)
	EVT_TIMER(ID_RENDERTIMER, OgreView::onTimer)
END_EVENT_TABLE()

/** Brief description which ends at this dot. Details follow
 *  here.
 */
OgreView::OgreView(wxFrame* parent) 
	: wxControl(parent, -1),
	  mTimer(this, ID_RENDERTIMER)
{
	mCamera = 0;
	mRenderWindow = 0;
	mSceneMgr = 0;
	mViewport = 0;
	
	// Create a new parameters list according to compiled OS
	Ogre::NameValuePairList params;
	String handle;
#ifdef __WXMSW__
	handle = Ogre::StringConverter::toString((size_t)((HWND)GetHandle()));
#elif defined(__WXGTK__)
	// TODO: Someone test this. you might to use "parentWindowHandle" if this
	// does not work.  Ogre 1.2 + Linux + GLX platform wants a string of the
	// format display:screen:window, which has variable types ulong:uint:ulong.
	GdkWindow * window = GetHandle()->window
	handle = Ogre::StringConverter::toString((ulong)GDK_WINDOW_XDISPLAY(window));
	handle += ":0:";
	handle += Ogre::StringConverter::toString((uint)GDK_WINDOW_XID(window));
#else
	#error Not supported on this platform.
#endif
	params["externalWindowHandle"] = handle;

	// Get wx control window size
	int width, height;
	GetSize(&width, &height);
	// Create the render window
	mRenderWindow = Ogre::Root::getSingleton().createRenderWindow("OgreRenderWindow", width, height, false, &params);

	// Scene Stuff
	chooseSceneManager();
    createCamera();
    createViewports();

	// Set default mipmap level (NB some APIs ignore this)
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	createScene();
}


// Destructor //
OgreView::~OgreView()
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

void OgreView::chooseSceneManager(void)
{
    // Create the SceneManager, in this case a generic one
    mSceneMgr = Ogre::Root::getSingleton().createSceneManager("TerrainSceneManager");
}

void OgreView::createCamera(void)
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


void OgreView::createScene(void) 
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

void OgreView::createViewports(void)
{
    // Create one viewport, entire window
    mViewport = mRenderWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));
}

void OgreView::destroyScene(void)
{
}

// Moves the view
void OgreView::cameraMove(Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
	mCamera->moveRelative(Vector3(x, y, z));
}

// Rotates the view
void OgreView::cameraRotate(Ogre::Real yaw, Ogre::Real pitch)
{
	mCamera->yaw(yaw * (mCamera->getFOVy() / mCamera->getAspectRatio() / 320.0f));
	mCamera->pitch(pitch * (mCamera->getFOVy() / 240.0f));
}

void OgreView::onEraseBackground(wxEraseEvent &e)
{
	update();
}

void OgreView::onFocusSet(wxFocusEvent& e)
{
}

void OgreView::onFocusLost(wxFocusEvent& e)
{
}

void OgreView::onLeftDragged(wxMouseEvent &e)
{
}

void OgreView::onLeftPressed(wxMouseEvent &e)
{
}

void OgreView::onMouse(wxMouseEvent &e)
{
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
			cameraRotate(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			cameraMove((Real)(-delta_x), (Real)delta_y, 0.0f);
	}

	// Save mouse position (for computing deltas for dragging)
	mMouseX = e.m_x;
	mMouseY = e.m_y;

	// Tell OGRE to redraw.
	update();
}

void OgreView::onPaint(wxPaintEvent &WXUNUSED(e))
{
	wxPaintDC dc(this);
	update();
}

void OgreView::onSize(wxSizeEvent &e)
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

void OgreView::onTimer(wxTimerEvent &e)
{
	update();
}

void OgreView::update()
{
	Ogre::Root::getSingleton().renderOneFrame();
}
