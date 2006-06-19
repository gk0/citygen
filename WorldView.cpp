// Includes 
#include "stdafx.h"
#include "WorldView.h"


// Namespace
using namespace Ogre;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();


// Event Table 
BEGIN_EVENT_TABLE(WorldView, wxControl)
	EVT_ERASE_BACKGROUND(WorldView::OnEraseBackground) 
	EVT_KILL_FOCUS(WorldView::OnLostFocus)
	EVT_MOUSE_EVENTS(WorldView::OnMouse)
//	EVT_PAINT(WorldView::OnPaint)
	EVT_SET_FOCUS(WorldView::OnSetFocus)
	EVT_SIZE(WorldView::OnSize)
	EVT_TIMER(ID_RENDERTIMER, WorldView::OnTimer)
END_EVENT_TABLE()


// Constructor 
WorldView::WorldView(wxFrame* parent) 
	: OgreView(parent)
{
	mCurrentObject = 0;
	mViewMode = normal;

	// Create our ray query
	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());
}


// Destructor //
WorldView::~WorldView()
{
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


void WorldView::deleteSelectedNode()
{
	if(mCurrentObject) {
		mSceneMgr->destroySceneNode(mCurrentObject->getName());
		mCurrentObject = 0;
	}
}

void WorldView::OnLeftPressed(wxMouseEvent &e)
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

void WorldView::OnLostFocus(wxFocusEvent& e)
{
	//Should tidy up my dragging logic here
}

void WorldView::OnMouse(wxMouseEvent &e)
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
			if(mViewMode == node) OnLeftDragged(e);
			else cameraRotate(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			cameraMove((Real)(-delta_x), (Real)delta_y, 0.0f);
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
	update();
}

void WorldView::OnSetFocus(wxFocusEvent& e)
{
	
}

void WorldView::OnLeftDragged(wxMouseEvent &e)
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

void WorldView::setMode(WorldViewMode mode)
{
	mViewMode = mode;
}
