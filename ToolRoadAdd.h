#ifndef TOOLROADADD_H
#define TOOLROADADD_H

#include "stdafx.h"
#include "ToolView.h"
#include "RoadGraph.h"

class ToolRoadAdd : public ToolView
{
private:
	Ogre::SceneManager* _sceneManager;
	RoadGraph&		_roadGraph;
	RoadGraph&		_simpleRoadGraph;

public:
	ToolRoadAdd(WorldFrame* wf, Ogre::SceneManager* sm, RoadGraph &g, RoadGraph &s);

	//void activate();
	//void deactivate();
	//void OnChar(wxKeyEvent& e);
	void OnMouseMove(wxMouseEvent &e);
	void OnLeftPressed(wxMouseEvent &e);
	void OnMiddlePressed(wxMouseEvent &e) {}
	void OnRightPressed(wxMouseEvent &e) {}
	//void OnMouseWheel(wxMouseEvent &e);
	bool alternate(wxMouseEvent &e);
};

#endif
