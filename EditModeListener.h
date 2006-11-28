#ifndef EditModeListener_H
#define EditModeListener_H

#include "stdafx.h"

class EditModeListener {
public:
	enum EditMode {
		view,
		node,
		edge,
		cell,
		XtoZ
	};
	
	virtual ~EditModeListener() {}
	virtual void setEditMode(EditMode mode) = 0;

};

#endif
