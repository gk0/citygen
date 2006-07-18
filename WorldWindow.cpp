#include "stdafx.h"
#include "WorldWindow.h"
#include "WorldDocument.h"

using namespace Ogre;

// Required for the timer
const long ID_RENDERTIMER = wxNewId();

#define CRAZY_MOUSE_OFFSET 10	//this really should be reqd. but is here until i find out whats going on


BEGIN_EVENT_TABLE(WorldWindow, wxOgre)
    EVT_MOUSE_EVENTS(WorldWindow::OnMouse)
//	EVT_ERASE_BACKGROUND(WorldWindow::onEraseBackground) 
//	EVT_KILL_FOCUS(WorldWindow::onLostFocus)
//	EVT_PAINT(WorldWindow::onPaint)
//	EVT_SET_FOCUS(WorldWindow::onSetFocus)
//	EVT_SIZE(WorldWindow::onSize)
//	EVT_TIMER(ID_RENDERTIMER, WorldWindow::onTimer)
END_EVENT_TABLE()


// Define a constructor for my canvas
WorldWindow::WorldWindow(wxView *v, wxFrame *frame):
    wxOgre(frame)
{
    view = v;
	mCurrentNode = 0;
	mRoadNode = 0;
	mEditMode = EditModeListener::view;
	mSelectMode = sel;
	mNodeCount = mRoadCount = 0;
	mManualObject = 0;
}

// The idea behind this function is to initialise the OgreView in a 
// default state and set it up with default assets and camera etc.
void WorldWindow::prepare()
{
	Init();

	// Create our ray query
	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());
}


// Define the repainting behaviour
void WorldWindow::OnDraw(wxDC& dc)
{
    if (view)
        view->OnDraw(& dc);
}

void WorldWindow::moveNode(float x, float y)
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
			WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
			doc->moveNode(mCurrentNode->getPosition(), itr->worldFragment->singleIntersection);
            mCurrentNode->setPosition( itr->worldFragment->singleIntersection );
            break;
        }
    }
}

void WorldWindow::OnSelectNode(wxMouseEvent &e)
{
	// Turn off bounding box.
    if(mCurrentNode)
    	mCurrentNode->showBoundingBox( false );
    	
    // Setup the ray scene query
    Ray mouseRay = mCamera->getCameraToViewportRay(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()+CRAZY_MOUSE_OFFSET) / float(mViewport->getActualHeight()) );
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
        		mCurrentNode = itr->movable->getParentSceneNode();
				break;
			}else{
				mCurrentNode = 0;
			}
		}else if(mSelectMode == add) {
			if ( itr->worldFragment )
			{
				WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
				if(doc->addNode(itr->worldFragment->singleIntersection)) {
					createNode(itr->worldFragment->singleIntersection);
				}
				break;
			}
		}
	}

	if(mSelectMode==del) {
		//Delete if we can
		if(mCurrentNode) {
			WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
			doc->removeNode(mCurrentNode->getPosition());
			mSceneMgr->destroySceneNode(mCurrentNode->getName());
			mCurrentNode = 0;
		}
	}
	
	// Turn on bounding box.
    if(mCurrentNode)
    	mCurrentNode->showBoundingBox( true );
}
void WorldWindow::clear() {
	//or maybe lets just trash it all
	//OK lets go nuts
	UnInit();
}

void WorldWindow::Refresh()
{
	wxOgre::Refresh();
	//um, I think I'm just going to have to search for every nodey thing.
	//or maybe lets just trash it all

	//reset counters
	WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
	if(doc) {
		boost::graph_traits<RoadGraph>::vertex_iterator vi, vend;
		for (boost::tie(vi, vend) = vertices(doc->mRoadGraph); vi != vend; ++vi) {
			createNode(doc->mRoadGraph[*vi]);
		}

		boost::graph_traits<RoadGraph>::edge_iterator i, end;
		for (boost::tie(i, end) = edges(doc->mRoadGraph); i != end; ++i) 
		{
			RoadVertex u,v;
			u = source(*i,doc->mRoadGraph);
			v = target(*i,doc->mRoadGraph);
			createRoad(doc->mRoadGraph[u], doc->mRoadGraph[v], "");
		}
		doc->printRoads();
	}
}

void WorldWindow::createNode(const Vector3& v) 
{
	Entity *ent;
	std::stringstream oss;
	oss << "Node" << mNodeCount++;
	String name(oss.str());
	ent = mSceneMgr->createEntity(name, "Node.mesh" );
	ent->setMaterialName("Examples/Hilite/Yellow");
	mCurrentNode = mSceneMgr->getRootSceneNode( )->createChildSceneNode(name, v);
	mCurrentNode->attachObject( ent );
	mCurrentNode->setScale( 0.1f, 0.1f, 0.1f );
}

void WorldWindow::createRoad(const Vector3& u, const Vector3& v, const std::string& name) 
{
	//std::stringstream oss;
	//oss << "manual" << mRoadCount++;
	//String name(oss.str());

	//omg i should like draw a line from my old node to my new node
	ManualObject* myManualObject = new ManualObject(name); 
	SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(name); 
/*
	MaterialPtr myManualObjectMaterial = MaterialManager::getSingleton().create(name+"Material","debugger"); 
	myManualObjectMaterial->setReceiveShadows(false); 
	myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1); 
	myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1); 
*/
//	myManualObject->begin(name+"Material", Ogre::RenderOperation::OT_LINE_LIST); 

	myManualObject->begin("Examples/Hilite/Yellow", Ogre::RenderOperation::OT_LINE_LIST);
	myManualObject->position(u); 
	myManualObject->position(v); 
	// etc 
	myManualObject->end(); 

	myManualObjectNode->attachObject(myManualObject);
	mManualObject = myManualObjectNode;
}

void WorldWindow::OnSelectRoad(wxMouseEvent &e)
{
	// Turn off bounding box.
    if ( mRoadNode )
    	mRoadNode->showBoundingBox( false );
    	
    // Setup the ray scene query
    Ray mouseRay = mCamera->getCameraToViewportRay(float(e.GetX()) / float(mViewport->getActualWidth()), 
			float(e.GetY()+CRAZY_MOUSE_OFFSET) / float(mViewport->getActualHeight()) );
	mRaySceneQuery->setRay( mouseRay );
    mRaySceneQuery->setSortByDistance( true );
    
    // Execute query
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr = result.begin();

	bool deselectNode = false;
	Ogre::SceneNode* previousRoadNode = 0;

	// Get results, create a node/entity on the position
	for( itr = result.begin( ); itr != result.end(); itr++ )
	{
		if((mSelectMode == sel) || (mSelectMode == del)) {
			if ( itr->movable && itr->movable->getName().substr(0, 4) == "Node" )
			{
				previousRoadNode = mRoadNode;
	     		mRoadNode = itr->movable->getParentSceneNode( );
				deselectNode = false;
				break;
			}else{
				deselectNode = true;
			}
		}else if(mSelectMode == add) {
			if ( itr->movable && itr->movable->getName().substr(0, 4) == "Node" )
			{
				if(mRoadNode)
				{
					WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
					Road r;
					if(doc->addRoad(mRoadNode->getPosition(), itr->movable->getParentSceneNode()->getPosition(), r))
					{
						//r to string for name
						createRoad(mRoadNode->getPosition(), itr->movable->getParentSceneNode()->getPosition(), "Road"+WorldDocument::roadToString(r));
					}
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
		//Delete a Road node a node ass monker
		//Delete if we can
		if(previousRoadNode && mRoadNode) {
			//mSceneMgr->destroySceneNode(mRoadNode->getName());
			//mRoadNode = 0;
			WorldDocument* doc = static_cast<WorldDocument*>(view->GetDocument());
			Road r;
			if(doc->findRoad(previousRoadNode->getPosition(), mRoadNode->getPosition(), r)) 
			{
				mSceneMgr->destroySceneNode("Road"+WorldDocument::roadToString(r));
			}
			//OK we need to get info about the road in order to be able to delete it.
			//mSceneMgr->destroySceneNode(mManualObject->getName());
		}
	}
	
	// Turn on bounding box.
    if ( mRoadNode )
    	mRoadNode->showBoundingBox( true );

	//mWorldDoc.printRoads();
}


void WorldWindow::OnLostFocus(wxFocusEvent& e)
{
	//Should tidy up my dragging logic here
}

void WorldWindow::OnMouse(wxMouseEvent &e)
{
	if(!view) return;

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
			if(mEditMode == node) OnLeftDragged(e);
			else cameraRotate(delta_x*2, delta_y);
		}else if(e.m_rightDown)
			cameraMove((Real)(-delta_x), (Real)delta_y, 0.0f);
	} 
	else 
	{
		if(e.m_leftDown) {
			if(mEditMode == node) OnSelectNode(e);
			else if(mEditMode == edge) OnSelectRoad(e);
		}
	}

	// Save mouse position (for computing deltas for dragging)
	mMouseX = e.m_x;
	mMouseY = e.m_y;

	// Tell OGRE to redraw.
	Update();
}

//void WorldWindow::onMouseWheel(wxMouseEvent &e) {
//}

void WorldWindow::OnSetFocus(wxFocusEvent& e)
{
	
}

void WorldWindow::OnLeftDragged(wxMouseEvent &e)
{
	switch(mEditMode) {
		case EditModeListener::view:

		case EditModeListener::node:
			if(mSelectMode ==sel){
				moveNode(float(e.GetX()) / float(mViewport->getActualWidth()), 
					float(e.GetY()+CRAZY_MOUSE_OFFSET) / float(mViewport->getActualHeight()));
			}
			break;
		default:
			break;
	}
}

void WorldWindow::setEditMode(EditModeListener::EditMode mode) {
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
	Update();
}

void WorldWindow::setSelectMode(SelectModeListener::SelectMode mode) {
	mSelectMode = mode;
}

