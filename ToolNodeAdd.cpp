#include "stdafx.h"
#include "ToolNodeAdd.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "SimpleNode.h"

using namespace Ogre;


ToolNodeAdd::ToolNodeAdd(WorldFrame* wf, SceneManager* sm, RoadGraph &g)
: ToolView(wf),
  mRoadGraph(g)
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
			if(!mProposedRoad) mProposedRoad = new WorldRoad(mSceneManager, 
				mWorldFrame->getSelected(), mProposedNode, mRoadGraph, false);
			else {
				mProposedRoad->setSrcNode(mWorldFrame->getSelected());
				mProposedRoad->setDstNode(mProposedNode);
			}
			
			NodeId nd;
			Vector2 newPoint;
			mProposedNode->setVisible(true);
			mProposedNode->setPosition(intersection);
			mWorldFrame->highlightNode(0);
			
			mSnapState = mProposedRoad->snap(25, nd, mIntersectingRoad, newPoint);
#ifdef DEBUG
			LogManager::getSingleton().logMessage("Snap:"+StringConverter::toString(mSnapState), LML_CRITICAL);
#endif
			switch(mSnapState)
			{
			case WorldRoad::none:
			case WorldRoad::road:
			case WorldRoad::simple_node:
				break;
			case WorldRoad::world_node:
				{
					NodeInterface* nb = mRoadGraph.getNode(nd);
					mWorldFrame->highlightNode(static_cast<WorldNode*>(nb));
					mProposedRoad->setDstNode(static_cast<WorldNode*>(nb));
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
	if(mProposedRoad) mProposedRoad->validate();
}


void ToolNodeAdd::OnLeftPressed(wxMouseEvent &e)
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
		// create a real node in place of proposed node if rqd.
		if(mProposedRoad->getDstNode() == mProposedNode)
		{
			WorldNode* wn = mWorldFrame->createNode();
			wn->setPosition(mProposedNode->getPosition());
			mProposedRoad->setDstNode(wn);
		}
		WorldRoad* wr = 0;

		// in these cases we are connecting to an existing road and 
		// alterations to the road graph are necessary
		if(mSnapState == WorldRoad::road || 
			mSnapState == WorldRoad::simple_node)
		{
			// insert node on road
			WorldRoad* ir = static_cast<WorldRoad*>(mRoadGraph.getRoad(mIntersectingRoad));
			WorldNode* wn = static_cast<WorldNode*>(mProposedRoad->getDstNode());
			mWorldFrame->insertNodeOnRoad(wn, ir);
		}
		//else
		// create road
		wr = mWorldFrame->createRoad(static_cast<WorldNode*>(mProposedRoad->getSrcNode()), 
									static_cast<WorldNode*>(mProposedRoad->getDstNode()));

		//NOTE: in the event that we try to double link two nodes
		// ie. a->b and then b->a a road will not be created and
		// we should fail silently
		if(wr == 0) return;


		// advance selected
		mWorldFrame->selectNode(static_cast<WorldNode*>(wr->getDstNode()));
	}
	// create proposed node
	else if(mProposedNode->getVisible())
	{
		WorldNode* wn = mWorldFrame->createNode();
		wn->setPosition(mProposedNode->getPosition());
	}
	else
	{
		// change selection
		mWorldFrame->selectNode(mWorldFrame->getHighlighted());
	}
	updateState(e);
	mWorldFrame->update();
}
