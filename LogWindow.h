#ifndef _LOG_WINDOW_H_
#define _LOG_WINDOW_H_

#include "stdafx.h"

class LogWindow : public Ogre::LogListener, public wxTextCtrl
{
	wxLogTextCtrl mLogRedirector;
public:
	LogWindow(wxWindow* parent);
	~LogWindow();

	void write (const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml = Ogre::LML_NORMAL, bool maskDebug = false); 
};

#endif