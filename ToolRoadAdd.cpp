#include "stdafx.h"
#include "ToolRoadAdd.h"
#include "WorldRoad.h"
#include "WorldNode.h"
#include "WorldFrame.h"

using namespace Ogre;

ToolRoadAdd::ToolRoadAdd(WorldFrame* wf, SceneManager* sm, RoadGraph &g, RoadGraph &s)
: ToolView(wf),
_roadGraph(g),
_simpleRoadGraph(s)
{
	_sceneManager = sm;
}

void ToolRoadAdd::OnMouseMove(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		_worldFrame->highlightNode(0);
		ToolView::OnMouseMove(e);
		return;
	}

	WorldNode *wn;
	if(_worldFrame->pickNode(e, 7, wn)) _worldFrame->highlightNode(wn);
	else _worldFrame->highlightNode(0);
	_worldFrame->Refresh();
}

void ToolRoadAdd::OnLeftPressed(wxMouseEvent &e)
{
	if(alternate(e) == true)
	{
		_worldFrame->highlightNode(0);
		ToolView::OnLeftPressed(e);
		return;
	}

	//TODO: 
	if(_worldFrame->getSelectedNode())
	{
		// delete a road
		WorldNode *wn;
		if(_worldFrame->pickNode(e, 7, wn))
		{
			if(wn == _worldFrame->getSelectedNode())
			{
				// deselect node
				_worldFrame->selectNode(0);
				_worldFrame->Refresh();
				return;
			}

			// need to check this road can't create it willy nilly
			WorldNode* proposedNode = _worldFrame->createNode();
			proposedNode->setPosition3D(wn->getPosition3D());
			
			WorldRoad* wr = new WorldRoad(_worldFrame->getSelectedNode(), 
								wn, _roadGraph, _simpleRoadGraph, _sceneManager);
			
			int				snapState;
			WorldRoad*		intersectingRoad;
			WorldNode*		snapNode;
			Ogre::Vector2	newPos;

			// needs a good studying
			snapState = wr->snapInfo(5, newPos, snapNode, intersectingRoad);
			delete wr;
			_worldFrame->deleteNode(proposedNode);
			if(snapState != 0)
			{
				Ogre::LogManager::getSingleton().logMessage("INVALID OPERATION: Cannot add a road that intersect with others. Try reducing the road deviation or use the add node tool.");
				//if(snapState == 1) _worldFrame->selectRoad(intersectingRoad);
			}
			else
			{
				_worldFrame->createRoad(_worldFrame->getSelectedNode(), wn);
				_worldFrame->selectNode(wn);
			}
			_worldFrame->Refresh();
		}
	}
	else
	{
		// just select a node
		WorldNode *wn;
		if(_worldFrame->pickNode(e, 7, wn))
		{
			_worldFrame->selectNode(wn);
			_worldFrame->Refresh();
		}
	}
}

bool ToolRoadAdd::alternate(wxMouseEvent &e)
{
	if(e.ControlDown()) return true;
	else return false;
}

void ToolRoadAdd::OnMiddlePressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnMiddlePressed(e);
}

void ToolRoadAdd::OnRightPressed(wxMouseEvent &e)
{
   if(alternate(e) == true)
      return ToolView::OnRightPressed(e);
}
