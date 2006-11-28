#ifndef SelectModeListener_H
#define SelectModeListener_H

#include "stdafx.h"

class SelectModeListener {

public:
	enum SelectMode {
		sel,
		add,
		del
	};
	
	virtual ~SelectModeListener() {}
	virtual void setSelectMode(SelectMode mode) = 0;

};

#endif
