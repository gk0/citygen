#include "stdafx.h"
#include "ToolNodeAdd.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "SimpleNode.h"

using namespace Ogre;

#define SNAP_SZ 3


ToolNodeAdd::ToolNodeAdd(WorldFrame* wf, SceneManager* sm, RoadGraph &g, RoadGraph &s)
: ToolView(wf),
  _roadGraph(g),
  _simpleRoadGraph(s)
{
	_sceneManager = sm;
}

void ToolNodeAdd::activate()
{
	//
	//mProposedNode = new WorldNode(mWorldFrame->getSceneManager(), "proposedNode");
	_proposedNode = _worldFrame->createNode();
	_proposedNode->setLabel("?");   
	_proposedNode->setVisible(false);
	_proposedRoad = 0;
	_snapState = 0;
}


void ToolNodeAdd::deactivate()
{
	if(_proposedRoad)
	{
		delete _proposedRoad;
		_proposedRoad = 0;
	}
	_worldFrame->deleteNode(_proposedNode);
	_worldFrame->update();
}


void ToolNodeAdd::OnChar(wxKeyEvent& e)
{
	int key = e.GetKeyCode();
	switch(key)
	{
	case WXK_ESCAPE:
		_worldFrame->selectNode(0);
		if(_proposedRoad)
		{
			delete _proposedRoad;
			_proposedRoad = 0;
		}
		_worldFrame->update();
		break;
	default:
		ToolView::OnChar(e);
		break;
	}
}


void ToolNodeAdd::OnMouseMove(wxMouseEvent &e)
{
	updateState(e);
	_worldFrame->update();
}


void ToolNodeAdd::updateState(wxMouseEvent &e)
{
	Vector3 intersection;
	if(_worldFrame->pickTerrainIntersection(e, intersection)) 
	{
		Vector2 newPos;
		_proposedNode->setPosition(intersection);

		// if a node is selected, then we're moving a road too
		if(_worldFrame->getSelected())
		{
			//// we have a selected node so we are creating a road here
			if(!_proposedRoad) 
				_proposedRoad = new WorldRoad(_worldFrame->getSelected(), 
						_proposedNode, _roadGraph, _simpleRoadGraph, _sceneManager);
			
			_snapState = _proposedRoad->snapInfo(SNAP_SZ, newPos, _snapNode, _intersectingRoad);
			//LogManager::getSingleton().logMessage("State: "+StringConverter::toString(mSnapState));
			//mSnapState = 0;
		}
		// only a proposed node
		else
		{
			if(_proposedRoad){
				delete _proposedRoad;
				_proposedRoad = 0;
			}
			_snapState = _proposedNode->snapInfo(SNAP_SZ, newPos, _snapNode, _intersectingRoad);
			//LogManager::getSingleton().logMessage("State: "+StringConverter::toString(mSnapState));
		}
		// defaults
		_proposedNode->setLabel("?");
		_proposedNode->setVisible(true);
		_worldFrame->highlightNode(_proposedNode);

		switch(_snapState)
		{
		case 1:
			_proposedNode->setPosition2D(newPos);
			break;
		case 2:
			_proposedNode->setPosition2D(newPos);
			_worldFrame->highlightNode(_snapNode);
			_proposedNode->setVisible(false);
			_proposedNode->setLabel(" ");	// HACK: setVisible doesn't work when a road is connected,
											// it maybe to do with the building of junctions.
		}
	}
	else
	{
		// hide proposed node and road as we are not pointing over terrain
		_proposedNode->setVisible(false);
		if(!_proposedRoad){
			delete _proposedRoad;
			_proposedRoad = 0;
		}
	}
	//NOTE: must validate nodes before roads
	// proposed road won't be validate as part of the WorldFrame
	if(_proposedRoad) 
	{
		_worldFrame->getSelected()->validate();
		_proposedNode->validate();
		_proposedRoad->validate();
	}
}


void ToolNodeAdd::OnLeftPressed(wxMouseEvent &e)
{
	try
	{
		updateState(e);

		// if node is selected and the current node is it
		if(_worldFrame->getSelected() && 
			_worldFrame->getSelected() == _worldFrame->getHighlighted())
		{
			// deselect node
			_worldFrame->selectNode(0);
			if(_proposedRoad)
			{
				delete _proposedRoad;
				_proposedRoad = 0;
			}
			_proposedNode->setVisible(false);
		}
		// create proposed road
		else if(_proposedRoad)
		{
			// delete the proposal
			delete _proposedRoad;
			_proposedRoad = 0;

			// declare the src & dst for the new road
			WorldNode *srcNode = _worldFrame->getSelected();
			WorldNode *dstNode;

			switch(_snapState)
			{
			case 0:		// no intersection 
				dstNode = _worldFrame->createNode();
				dstNode->setPosition2D(_proposedNode->getPosition2D());
				break;
			case 1:		// road intersection
				dstNode = _worldFrame->createNode();
				dstNode->setPosition2D(_proposedNode->getPosition2D());
				_worldFrame->insertNodeOnRoad(dstNode, _intersectingRoad);			// TODO: this can be a simpleroad for some reason
				break;
			case 2:		// node snapped
				dstNode = _snapNode;
				break;
			default:
				throw new Exception(Exception::ERR_INVALID_STATE, "Invalid snap state", "ToolNodeAdd::OnLeftPressed");
				break;
			}

			// Create Road
			WorldRoad* wr = 0;
			wr = _worldFrame->createRoad(srcNode, dstNode);

			//NOTE: in the event that we try to double link two nodes
			// ie. a->b and then b->a a road will not be created and
			// we should fail silently
			if(wr == 0) return;

			//TODO: exception and catch to present error message

			// advance selected
			_worldFrame->selectNode(dstNode);
		}
		// create proposed node
		else if(_proposedNode->getVisible())
		{
			WorldNode* wn = _worldFrame->createNode();
			wn->move(_proposedNode->getPosition2D());
			if(_snapState == 1) _worldFrame->insertNodeOnRoad(wn, _intersectingRoad);
		}
		else
		{
			// change selection
			_worldFrame->selectNode(_worldFrame->getHighlighted());
		}
		updateState(e);
		_worldFrame->update();
	}
	catch(Exception &e)
	{
		LogManager::getSingleton().logMessage(
			"ToolNodeAdd::OnLeftPressed() "+e.getFullDescription());
	}
	catch(std::exception &e)
	{
		LogManager::getSingleton().logMessage(
			"ToolNodeAdd::OnLeftPressed() "+String(e.what()));
	}
	catch(...)
	{
		LogManager::getSingleton().logMessage(
			"ToolNodeAdd::OnLeftPressed() unkown exception");
	}
}
