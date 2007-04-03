#include "stdafx.h"
#include "LogFrame.h"

LogFrame::LogFrame(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY), mLogRedirector(this)
{
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR <= 2
	Ogre::LogManager::getSingleton().addListener(this);
#else
	Ogre::LogManager::getSingleton().getDefaultLog()->addListener(this);
#endif
}

LogFrame::~LogFrame()
{
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR <= 2
	Ogre::LogManager::getSingleton().removeListener(this);
#else
	Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(this);
#endif
}

void LogFrame::write(const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml /* = Ogre::LML_NORMAL */, bool maskDebug /* = false */)
{
	AppendText(_("\n") + wxString(message.c_str(), wxConvUTF8));
}

void LogFrame::messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, 
							bool maskDebug, const Ogre::String &logName)
{
	AppendText(_("\n") + wxString(message.c_str(), wxConvUTF8));
}

