#ifndef _LOG_WINDOW_H_
#define _LOG_WINDOW_H_

#include "stdafx.h"

class LogFrame : public Ogre::LogListener, public wxTextCtrl
{
	wxLogTextCtrl mLogRedirector;
public:
	LogFrame(wxWindow* parent);
	~LogFrame();

	void write (const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml = Ogre::LML_NORMAL, bool maskDebug = false); 

	void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml = Ogre::LML_NORMAL, bool maskDebug = false, const Ogre::String &logName = "");
};

#endif
