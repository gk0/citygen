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

	WorldNode *mProposedNode;
	WorldRoad *mProposedRoad;

	WorldRoad::RoadIntersectionState mSnapState;
	RoadId mIntersectingRoad;


public:
	ToolNodeAdd(WorldFrame* wf, Ogre::SceneManager* sm, RoadGraph &g);

	void activate();
	void deactivate();
	void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	//void OnMouseWheel(wxMouseEvent &e);

	void updateState(wxMouseEvent &e);
};

#endif
