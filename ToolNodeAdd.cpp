#include "stdafx.h"
#include "ToolNodeAdd.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "SimpleNode.h"

using namespace Ogre;

#define SNAP_SZ 3


ToolNodeAdd::ToolNodeAdd(WorldFrame* wf, SceneManager* sm, RoadGraph &g, RoadGraph &s)
: ToolView(wf),
  mRoadGraph(g),
  mSimpleRoadGraph(s)
{
	mSceneManager = sm;
}

void ToolNodeAdd::activate()
{
	//
	//mProposedNode = new WorldNode(mWorldFrame->getSceneManager(), "proposedNode");
	mProposedNode = mWorldFrame->createNode();
	mProposedNode->setLabel("?");   
	mProposedNode->setVisible(false);
	mProposedRoad = 0;
	mSnapState = 0;
}


void ToolNodeAdd::deactivate()
{
	if(mProposedRoad)
	{
		delete mProposedRoad;
		mProposedRoad = 0;
	}
	mWorldFrame->deleteNode(mProposedNode);
	mWorldFrame->update();
}


void ToolNodeAdd::OnChar(wxKeyEvent& e)
{
	int key = e.GetKeyCode();
	switch(key)
	{
	case WXK_ESCAPE:
		mWorldFrame->selectNode(0);
		if(mProposedRoad)
		{
			delete mProposedRoad;
			mProposedRoad = 0;
		}
		mWorldFrame->update();
		break;
	default:
		ToolView::OnChar(e);
		break;
	}
}


void ToolNodeAdd::OnMouseMove(wxMouseEvent &e)
{
	updateState(e);
	mWorldFrame->update();
}


void ToolNodeAdd::updateState(wxMouseEvent &e)
{
	Vector3 intersection;
	if(mWorldFrame->pickTerrainIntersection(e, intersection)) 
	{
		Vector2 newPos;
		mProposedNode->setPosition(intersection);

		// if a node is selected, then we're moving a road too
		if(mWorldFrame->getSelected())
		{
			//// we have a selected node so we are creating a road here
			if(!mProposedRoad) 
				mProposedRoad = new WorldRoad(mWorldFrame->getSelected(), 
						mProposedNode, mRoadGraph, mSimpleRoadGraph, mSceneManager);
			
			mSnapState = mProposedRoad->snapInfo(SNAP_SZ, newPos, mSnapNode, mIntersectingRoad);
			//LogManager::getSingleton().logMessage("State: "+StringConverter::toString(mSnapState));
			//mSnapState = 0;
		}
		// only a proposed node
		else
		{
			if(mProposedRoad){
				delete mProposedRoad;
				mProposedRoad = 0;
			}
			mSnapState = mProposedNode->snapInfo(SNAP_SZ, newPos, mSnapNode, mIntersectingRoad);
			//LogManager::getSingleton().logMessage("State: "+StringConverter::toString(mSnapState));
		}
		// defaults
		mProposedNode->setLabel("?");
		mProposedNode->setVisible(true);
		mWorldFrame->highlightNode(mProposedNode);

		switch(mSnapState)
		{
		case 1:
			mProposedNode->setPosition2D(newPos);
			break;
		case 2:
			mProposedNode->setPosition2D(newPos);
			mWorldFrame->highlightNode(mSnapNode);
			mProposedNode->setVisible(false);
			mProposedNode->setLabel(" ");	// HACK: setVisible doesn't work when a road is connected,
											// it maybe to do with the building of junctions.
		}
	}
	else
	{
		// hide proposed node and road as we are not pointing over terrain
		mProposedNode->setVisible(false);
		if(!mProposedRoad){
			delete mProposedRoad;
			mProposedRoad = 0;
		}
	}
	//NOTE: must validate nodes before roads
	// proposed road won't be validate as part of the WorldFrame
	if(mProposedRoad) 
	{
		mWorldFrame->getSelected()->validate();
		mProposedNode->validate();
		mProposedRoad->validate();
	}
}


void ToolNodeAdd::OnLeftPressed(wxMouseEvent &e)
{
	try
	{
		updateState(e);

		// if node is selected and the current node is it
		if(mWorldFrame->getSelected() && 
			mWorldFrame->getSelected() == mWorldFrame->getHighlighted())
		{
			// deselect node
			mWorldFrame->selectNode(0);
			if(mProposedRoad)
			{
				delete mProposedRoad;
				mProposedRoad = 0;
			}
			mProposedNode->setVisible(false);
		}
		// create proposed road
		else if(mProposedRoad)
		{
			// delete the proposal
			delete mProposedRoad;
			mProposedRoad = 0;

			// declare the src & dst for the new road
			WorldNode *srcNode = mWorldFrame->getSelected();
			WorldNode *dstNode;

			switch(mSnapState)
			{
			case 0:		// no intersection 
				dstNode = mWorldFrame->createNode();
				dstNode->setPosition2D(mProposedNode->getPosition2D());
				break;
			case 1:		// road intersection
				dstNode = mWorldFrame->createNode();
				dstNode->setPosition2D(mProposedNode->getPosition2D());
				mWorldFrame->insertNodeOnRoad(dstNode, mIntersectingRoad);
				break;
			case 2:		// node snapped
				dstNode = mSnapNode;
				break;
			default:
				throw new Exception(Exception::ERR_INVALID_STATE, "Invalid snap state", "ToolNodeAdd::OnLeftPressed");
				break;
			}

			// Create Road
			WorldRoad* wr = 0;
			wr = mWorldFrame->createRoad(srcNode, dstNode);

			//NOTE: in the event that we try to double link two nodes
			// ie. a->b and then b->a a road will not be created and
			// we should fail silently
			if(wr == 0) return;

			//TODO: exception and catch to present error message

			// advance selected
			mWorldFrame->selectNode(dstNode);
		}
		// create proposed node
		else if(mProposedNode->getVisible())
		{
			WorldNode* wn = mWorldFrame->createNode();
			wn->move(mProposedNode->getPosition2D());
			if(mSnapState == 1) mWorldFrame->insertNodeOnRoad(wn, mIntersectingRoad);
		}
		else
		{
			// change selection
			mWorldFrame->selectNode(mWorldFrame->getHighlighted());
		}
		updateState(e);
		mWorldFrame->update();
	}
	catch(Exception &e)
	{
		int z = 0;
	}
	catch(std::exception &e)
	{
		int z = 0;
	}
}
