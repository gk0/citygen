#ifndef NODEPROPERTYPAGE_H
#define NODEPROPERTYPAGE_H

#include "stdafx.h"

class WorldCanvas;

class NodePropertyPage : public wxPropertyGridPage
{
	DECLARE_CLASS(NodePropertyPage)

private:
	wxPGProperty* labelProp;
	wxPGProperty* xProp;
	wxPGProperty* yProp;
	wxPGProperty* zProp;
	WorldCanvas* mWorldCanvas;

protected:
	DECLARE_EVENT_TABLE()

public:
	virtual void Init();

	void updateData(const std::string& l, const float& x, const float& y, const float& z);
	void setCanvas(WorldCanvas* c);

	virtual void OnPropertyGridChange( wxPropertyGridEvent& event );

};

#endif

