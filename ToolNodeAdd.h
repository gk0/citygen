#ifndef TOOLNODEADD_H
#define TOOLNODEADD_H

#include "stdafx.h"
#include "ToolView.h"
#include "WorldRoad.h"


class ToolNodeAdd : public ToolView
{
private:
	Ogre::SceneManager* mSceneManager;
	RoadGraph& mRoadGraph;
	RoadGraph& mSimpleRoadGraph;

	WorldNode *mProposedNode;
	WorldRoad *mProposedRoad;

	WorldRoad::RoadIntersectionState mSnapState;
	RoadId mIntersectingRoad;
	NodeId mSnapNode;


public:
	ToolNodeAdd(WorldFrame* wf, Ogre::SceneManager* sm, RoadGraph &g, RoadGraph &s);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);

	void updateState(wxMouseEvent &e);
};

#endif
