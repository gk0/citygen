#ifndef TOOL_H
#define TOOL_H

#include "stdafx.h"

class Tool
{

public:
	virtual ~Tool() {}
	virtual void activate() = 0;
	virtual void deactivate() = 0;
	virtual void OnChar(wxKeyEvent& e) = 0;
	virtual void OnMouseMove(wxMouseEvent &e) = 0;
	virtual void OnLeftPressed(wxMouseEvent &e) = 0;
	virtual void OnMouseWheel(wxMouseEvent &e) = 0;
};

#endif