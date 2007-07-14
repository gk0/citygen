#ifndef _PERFTIMER_H_
#define _PERFTIMER_H_

#include "stdafx.h"

class PerformanceTimer
{
private:
	Ogre::String mTitle;
	wxDateTime mTime;
	wxDateTime mStopTime;
	bool mActive;

public:
	PerformanceTimer(const Ogre::String& t = "Default Timer")
		: mTitle(t)
	{
		reset();
	}

	void setTitle(const Ogre::String& t)
	{
		mTitle = t;
	}

	void reset()
	{
		mActive = true;
		mTime = wxDateTime::UNow();
	}

	void stop()
	{
		if(mActive)
		{
			mStopTime = wxDateTime::UNow();
			mActive = false;
		}
	}

	void log()
	{
		stop();
		Ogre::LogManager::getSingleton().logMessage(toString());
	}

	Ogre::String toString()
	{
		if(mActive) return Ogre::String(mTitle+": active.");
		wxLongLong ms = (mStopTime - mTime).GetMilliseconds();
		Ogre::String strMs = static_cast<const char*>(ms.ToString().mb_str());
		std::stringstream oss;
		oss << mTitle << ": " << strMs << "ms.";
		return Ogre::String(oss.str());
	}
};




#endif
