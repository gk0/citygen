#ifndef _PERFTIMER_H_
#define _PERFTIMER_H_

#include "stdafx.h"

class PerformanceTimer
{
private:
	Ogre::String	_title;
	wxDateTime		_time;
	wxDateTime		_pauseTime;
	wxDateTime		_stopTime;
	bool			_active;

public:
	PerformanceTimer(const Ogre::String& t = "Default Timer")
		: _title(t)
	{
		reset();
	}

	void setTitle(const Ogre::String& t)
	{
		_title = t;
	}

	void reset()
	{
		_active = true;
		_time = wxDateTime::UNow();
	}

	void pause()
	{
		_pauseTime =  wxDateTime::UNow();
	}

	void resume()
	{
		_time += wxDateTime::UNow() - _pauseTime;
	}

	void stop()
	{
		if(_active)
		{
			_stopTime = wxDateTime::UNow();
			_active = false;
		}
	}

	void log()
	{
		stop();
		Ogre::LogManager::getSingleton().logMessage(toString());
	}

	Ogre::String toString()
	{
		if(_active) return Ogre::String(_title+": active.");
		wxLongLong ms = (_stopTime - _time).GetMilliseconds();
		Ogre::String strMs = static_cast<const char*>(ms.ToString().mb_str());
		std::stringstream oss;
		oss << _title << ": " << strMs << "ms.";
		return Ogre::String(oss.str());
	}
};




#endif
