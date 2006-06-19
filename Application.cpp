#include "stdafx.h"
#include "Application.h"

using namespace Ogre;

// Platform specific crap that creates the application, etc. //
IMPLEMENT_APP(Application);

// Function definitions //
// App Init
bool Application::OnInit()
{
	if (!wxApp::OnInit()) 
		return false;

	InitializeOgre();

	setupResources();

	// now we need to create MainWindow singleton because
	// it creates RenderWindow, which is needed for resource loading (for example, textures and VBO)
	// we create it, but don't show to user to avoid flickness
	mWindow = new MainWindow();
	SetTopWindow(mWindow); // set our MainWindow the main application window
	mWindow->Show(true);

	// Resources and resource initialization
	
	loadResources();

	//
	

	// All clear!
	return true;
}

// App Exit
int Application::OnExit()
{
	// Clean Up OGRE
	if (mRoot)
		delete mRoot;

	// All done here.
	return 0;
}


bool Application::InitializeOgre()
{
	// Make the root
	mRoot = new Root();

	try
	{
		if (!mRoot->restoreConfig())
		{
			mRoot->showConfigDialog();
		}
		// initialise with false doesn't create a window
		mRoot->initialise(false);
   }
   catch(Ogre::Exception &e)
   {
	  String s = "OgreView::Init() - Exception:\n" + e.getFullDescription() + "\n";
	  LogManager::getSingleton().logMessage(s, LML_CRITICAL);
      wxMessageBox( e.getFullDescription().c_str(), _T("Exception!"), wxICON_EXCLAMATION);
      return false;
   }
   return true;
}


/// Method which will define the source of resources (other than current folder)
void Application::setupResources(void)
{
    // Load resource paths from config file
    ConfigFile cf;
    cf.load("resources.cfg");

    // Go through all sections & settings in the file
    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}

/// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
void Application::loadResources(void)
{
	// Initialise, parse scripts etc
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
