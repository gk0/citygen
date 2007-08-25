#ifndef _LOG_WINDOW_H_
#define _LOG_WINDOW_H_

#include "stdafx.h"

class LogFrame : public Ogre::LogListener, public wxTextCtrl
{
private:
	wxLogTextCtrl _logRedirector;

public:
	LogFrame(wxWindow* parent,  const wxSize& sz = wxDefaultSize);
	~LogFrame();

	void write(const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml = Ogre::LML_NORMAL, bool maskDebug = false); 

	void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml = Ogre::LML_NORMAL, bool maskDebug = false, const Ogre::String &logName = "");
};

#endif
