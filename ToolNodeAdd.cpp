#include "stdafx.h"
#include "ToolNodeAdd.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "SimpleNode.h"
#include "WorldFrame.h"

using namespace Ogre;

#define NODEADDSNAPSZ 50


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
	_worldFrame->selectNode(0);
	_worldFrame->deleteNode(_proposedNode);
	_worldFrame->update();
}


void ToolNodeAdd::OnChar(wxKeyEvent& e)
{
	if(e.GetKeyCode() == WXK_ESCAPE)
	{
		_worldFrame->selectNode(0);
		if(_proposedRoad)
		{
			delete _proposedRoad;
			_proposedRoad = 0;
		}
		_proposedNode->setVisible(false);
		_worldFrame->update();
	}
}

bool ToolNodeAdd::alternate(wxMouseEvent &e)
{
	if(e.ControlDown())
	{
		if(_proposedRoad)
		{
			delete _proposedRoad;
			_proposedRoad = 0;
		}
		_worldFrame->selectNode(0);
		_proposedNode->setVisible(false);
		return true;
	}
	return false;
}

void ToolNodeAdd::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		ToolView::OnMouseMove(e);
		return;
	}
	//TODO: should detect state change and not auto update 
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
		if(_worldFrame->getSelectedNode())
		{
			//// we have a selected node so we are creating a road here
			if(!_proposedRoad) 
				_proposedRoad = new WorldRoad(_worldFrame->getSelectedNode(), 
						_proposedNode, _roadGraph, _simpleRoadGraph, _sceneManager);
			
			_proposedRoad->setSelected(true);
			_intersectingRoad = 0;
/*
			NodeId nd;
			RoadId rd;

			_snapState = _roadGraph.findClosestIntscnOrNode(_worldFrame->getSelectedNode()->_nodeId, _proposedNode->getPosition2D(),15, 
				newPos, nd, rd);


			if(_snapState == 2)
			{
				NodeInterface* ni = _roadGraph.getNode(nd);
				_snapNode = static_cast<WorldNode*>(ni);
				if(typeid(*ni) == typeid(WorldNode))
					LogManager::getSingleton().logMessage("Snap to Node: "+static_cast<WorldNode*>(_roadGraph.getNode(nd))->getLabel());
				else
					LogManager::getSingleton().logMessage("Snap to Node: !WN.");
			}
			else if(_snapState == 1)
				_intersectingRoad = static_cast<WorldRoad*>(_roadGraph.getRoad(rd));
			else
				LogManager::getSingleton().logMessage("State: "+StringConverter::toString(_snapState));
*/
			if(!e.ShiftDown())
				_snapState = _proposedRoad->snapInfo(NODEADDSNAPSZ, newPos, _snapNode, _intersectingRoad);
			else
				_snapState = 0;

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
			_snapState = _proposedNode->snapInfo(NODEADDSNAPSZ, newPos, _snapNode, _intersectingRoad);
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
			_proposedNode->setLabel("-");	// HACK: setVisible doesn't work when a road is connected,
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
		_worldFrame->getSelectedNode()->validate();
		_proposedNode->validate();
		_proposedRoad->validate();
	}
}


void ToolNodeAdd::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e))
	{
		ToolView::OnLeftPressed(e);
		return;
	}

	try
	{
		updateState(e);

		// if node is selected and the current node is it
		if(_worldFrame->getSelectedNode() && 
			_worldFrame->getSelectedNode() == _worldFrame->getHighlightedNode())
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
			WorldNode *srcNode = _worldFrame->getSelectedNode();
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
				_worldFrame->insertNodeOnRoad(dstNode, _intersectingRoad);
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
			_worldFrame->selectNode(_worldFrame->getHighlightedNode());
		}
		updateState(e);
		_worldFrame->Refresh();
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
			"ToolNodeAdd::OnLeftPressed() unknown exception");
	}
}

void ToolNodeAdd::OnMiddlePressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnMiddlePressed(e);
}

void ToolNodeAdd::OnRightPressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnRightPressed(e);
}
