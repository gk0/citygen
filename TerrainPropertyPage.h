#ifndef TERRAINPROPERTYPAGE_H
#define TERRAINPROPERTYPAGE_H

#include "stdafx.h"
#include "PropertyPage.h"

class TerrainPropertyPage : public PropertyPage
{
	DECLARE_CLASS(TerrainPropertyPage)

protected:
	DECLARE_EVENT_TABLE()

public:
	TerrainPropertyPage(WorldFrame* wf) : PropertyPage(wf) {}

	virtual void Init();
	void update();
	virtual void OnPropertyGridChange(wxPropertyGridEvent& event);
};

#endif
