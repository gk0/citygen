#include "stdafx.h"
#include "ToolNodeAdd.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "SimpleNode.h"

using namespace Ogre;


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
		// if a node is selected, then we're moving a road too
		if(mWorldFrame->getSelected())
		{
			// we have a selected node so we are creating a road here
			if(!mProposedRoad) 
				mProposedRoad = new WorldRoad(mWorldFrame->getSelected(), 
						mProposedNode, mRoadGraph, mSimpleRoadGraph, mSceneManager);
			

			mProposedNode->setVisible(true);
			mProposedNode->setPosition(intersection);
			mWorldFrame->highlightNode(mProposedNode);

			Vector2 newPoint;
			mSnapState = mProposedRoad->snap(25, mSnapNode, mIntersectingRoad, newPoint);
			LogManager::getSingleton().logMessage("Snap:"+StringConverter::toString(mSnapState));

			switch(mSnapState)
			{
			case WorldRoad::none:
			case WorldRoad::road:
				break;
			case WorldRoad::world_node:
				{
					NodeInterface* ni = mRoadGraph.getNode(mSnapNode);
					mWorldFrame->highlightNode(static_cast<WorldNode*>(ni));
					mProposedNode->setVisible(false);
				}
				break;
			}
		}
		// only a proposed node
		else
		{
			if(mProposedRoad){
				delete mProposedRoad;
				mProposedRoad = 0;
			}
			if(mWorldFrame->highlightNodeFromLoc(Vector2(intersection.x, intersection.z))
				&& mWorldFrame->getHighlighted() != mProposedNode)
				mProposedNode->setVisible(false);
			else
			{
				mWorldFrame->highlightNode(0);
				mProposedNode->setPosition(intersection);
				mProposedNode->setVisible(true);
			}
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
			case WorldRoad::none:
				// create new dest node
				dstNode = mWorldFrame->createNode();
				dstNode->setPosition2D(mProposedNode->getPosition2D());
				break;
			case WorldRoad::road:
				// create new dest node
				dstNode = mWorldFrame->createNode();
				dstNode->setPosition2D(mProposedNode->getPosition2D());
				{
					// in these cases we are connecting to an existing road and 
					// must insert a node at the junction
					WorldRoad* ir = static_cast<WorldRoad*>(mRoadGraph.getRoad(mIntersectingRoad));
					mWorldFrame->insertNodeOnRoad(dstNode, ir);
				}
				break;
			case WorldRoad::world_node:
				// get the destination node
				dstNode = static_cast<WorldNode*>(mRoadGraph.getNode(mSnapNode));
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
