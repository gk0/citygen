#include "stdafx.h"
#include "LogWindow.h"

LogWindow::LogWindow(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY), mLogRedirector(this)
{
	Ogre::LogManager::getSingleton().addListener(this);
}

LogWindow::~LogWindow()
{
	Ogre::LogManager::getSingleton().removeListener(this);
}

void LogWindow::write(const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml /* = Ogre::LML_NORMAL */, bool maskDebug /* = false */)
{
	AppendText("\n" + wxString(message.c_str()));
}
