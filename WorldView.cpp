// Includes 
#include "stdafx.h"
#include "WorldView.h"
#include "WorldDoc.h"

// Namespace
using namespace Ogre;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

// Required for WX
//IMPLEMENT_CLASS(WorldView, OgreView)

// Event Table 
BEGIN_EVENT_TABLE(WorldView, wxControl)
	EVT_ERASE_BACKGROUND(WorldView::onEraseBackground) 
	EVT_KILL_FOCUS(WorldView::onLostFocus)
	EVT_MOUSE_EVENTS(WorldView::onMouse)
//	EVT_PAINT(WorldView::onPaint)
	EVT_SET_FOCUS(WorldView::onSetFocus)
	EVT_SIZE(WorldView::onSize)
	EVT_TIMER(ID_RENDERTIMER, WorldView::onTimer)
END_EVENT_TABLE()


// Constructor 
WorldView::WorldView(wxFrame* parent) 
	: OgreView(parent)
{
	mCurrentNode = 0;
	mRoadNode = 0;
	mEditMode = view;
	mSelectMode = sel;
	mNodeCount = mRoadCount = 0;

	// Create our ray query
	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());
}


// Destructor //
WorldView::~WorldView()
{
}

void WorldView::moveNode(float x, float y)
{
	if(!mCurrentNode) return;

	Ray mouseRay = mCamera->getCameraToViewportRay(x,y);
    mRaySceneQuery->setRay( mouseRay );
    mRaySceneQuery->setSortByDistance( false );

    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr;

    for ( itr = result.begin( ); itr != result.end(); itr++ )
    {
        if ( itr->worldFragment )
        {
            mCurrentNode->setPosition( itr->worldFragment->singleIntersection );
            break;
        }
    }
}

void WorldView::onSelectNode(wxMouseEvent &e)
{
	// Turn off bounding box.
    if(mCurrentNode)
    	mCurrentNode->showBoundingBox( false );
    	
    // Setup the ray scene query
    Ray mouseRay = mCamera->getCameraToViewportRay(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()) / float(mViewport->getActualHeight()) );
	mRaySceneQuery->setRay( mouseRay );
    mRaySceneQuery->setSortByDistance( true );
    
    // Execute query
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr = result.begin();

	// Get results, create a node/entity on the position
	for( itr = result.begin( ); itr != result.end(); itr++ )
	{
		if(mSelectMode == sel || mSelectMode==del) {
			if ( itr->movable && itr->movable->getName().substr(0, 4) == "Node" )
			{
        		mCurrentNode = itr->movable->getParentSceneNode( );
				break;
			}else{
				mCurrentNode = 0;
			}
		}else if(mSelectMode == add) {
			if ( itr->worldFragment )
			{
				Entity *ent;
				std::stringstream oss;
				oss << "Node" << mNodeCount++;
				String name(oss.str());
				ent = mSceneMgr->createEntity(name, "Node.mesh" );
				ent->setMaterialName("Examples/Hilite/Yellow");
				mCurrentNode = mSceneMgr->getRootSceneNode( )->createChildSceneNode( name+"ScNode", itr->worldFragment->singleIntersection );
				mCurrentNode->attachObject( ent );
				mCurrentNode->setScale( 0.1f, 0.1f, 0.1f );
				break;
			}
		}
	}

	if(mSelectMode==del) {
		//Delete if we can
		if(mCurrentNode) {
			mSceneMgr->destroySceneNode(mCurrentNode->getName());
			mCurrentNode = 0;
		}
	}
	
	// Turn on bounding box.
    if(mCurrentNode)
    	mCurrentNode->showBoundingBox( true );
}


void WorldView::onSelectRoad(wxMouseEvent &e)
{
	// Turn off bounding box.
    if ( mRoadNode )
    	mRoadNode->showBoundingBox( false );
    	
    // Setup the ray scene query
    Ray mouseRay = mCamera->getCameraToViewportRay(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()) / float(mViewport->getActualHeight()) );
	mRaySceneQuery->setRay( mouseRay );
    mRaySceneQuery->setSortByDistance( true );
    
    // Execute query
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr = result.begin();

	bool deselectNode = false;

	// Get results, create a node/entity on the position
	for( itr = result.begin( ); itr != result.end(); itr++ )
	{
		if((mSelectMode == sel) || (mSelectMode == del)) {
			/*if ( itr->movable && itr->movable->getName().substr(0, 5) != "tile[" )
			{
        		mRoadNode = itr->movable->getParentSceneNode( );
				break;
			}else{
				mRoadNode = 0;
			}*/
		}else if(mSelectMode == add) {
			if ( itr->movable && itr->movable->getName().substr(0, 4) == "Node" )
			{
				if(mRoadNode) {

					
					std::stringstream oss;
					oss << "manual" << mRoadCount++;
					String name(oss.str());

					//omg i should like draw a line from my old node to my new node
					ManualObject* myManualObject = new ManualObject(name); 
					SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(name+"_node"); 

					MaterialPtr myManualObjectMaterial = MaterialManager::getSingleton().create(name+"Material","debugger"); 
					myManualObjectMaterial->setReceiveShadows(false); 
					myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true); 
					myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0); 
					myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1); 
					myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1); 

					myManualObject->begin(name+"Material", Ogre::RenderOperation::OT_LINE_LIST); 
					myManualObject->position(mRoadNode->getPosition()); 
					myManualObject->position(itr->movable->getParentSceneNode()->getPosition()); 
					// etc 
					myManualObject->end(); 

					myManualObjectNode->attachObject(myManualObject);

					/////////////////////
				}
        		mRoadNode = itr->movable->getParentSceneNode();
				deselectNode = false;
				break;
			}else{
				deselectNode = true;
			}
		}
	}
	if(deselectNode) mRoadNode = 0;

	if(mSelectMode==del) {
		//Delete if we can
		if(mRoadNode) {
			mSceneMgr->destroySceneNode(mRoadNode->getName());
			mRoadNode = 0;
		}
	}
	
	// Turn on bounding box.
    if ( mRoadNode )
    	mRoadNode->showBoundingBox( true );
}


void WorldView::onLostFocus(wxFocusEvent& e)
{
	//Should tidy up my dragging logic here
}

void WorldView::onMouse(wxMouseEvent &e)
{
	//Check for mouse wheel scroll
	if (e.GetWheelRotation() != 0)
	{
		Vector3 mTranslateVector(0,0,0);
		mTranslateVector.z = e.GetWheelRotation() / 10;
		mCamera->moveRelative(mTranslateVector);
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
			if(mEditMode == node) onLeftDragged(e);
			else cameraRotate(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			cameraMove((Real)(-delta_x), (Real)delta_y, 0.0f);
	} 
	else 
	{
		if(e.m_leftDown) {
			if(mEditMode == node) onSelectNode(e);
			else if(mEditMode == edge) onSelectRoad(e);
		}
	}

	// Save mouse position (for computing deltas for dragging)
	mMouseX = e.m_x;
	mMouseY = e.m_y;

	// Tell OGRE to redraw.
	update();
}
/*
void WorldView::onMouseWheel(wxMouseEvent &e) {
}
*/
void WorldView::onSetFocus(wxFocusEvent& e)
{
	
}

void WorldView::onLeftDragged(wxMouseEvent &e)
{
	switch(mEditMode) {
		case view:

		case node:
			if(mSelectMode ==sel){
				moveNode(float(e.GetX()) / float(mViewport->getActualWidth()), 
					float(e.GetY()) / float(mViewport->getActualHeight()));
			}
			break;
		default:
			break;
	}
}

void WorldView::setEditMode(EditModeListener::EditMode mode) {
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
	}
	update();
}

void WorldView::setSelectMode(SelectModeListener::SelectMode mode) {
	mSelectMode = mode;
}