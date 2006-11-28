#include "stdafx.h"
#include "Application.h"
#include "WorldView.h"
#include "WorldDocument.h"

using namespace Ogre;

// Platform specific crap that creates the application, etc. //
IMPLEMENT_APP(Application);


Application::Application()
: mDocManager(0),
  mRoot(0),
  mWindow(0)
{
}

// Function definitions //
// App Init
bool Application::OnInit()
{

	if (!wxApp::OnInit()) 
		return false;

	//DocManager
	// Create a document manager
    mDocManager = new wxDocManager;

    // Create a template relating drawing documents to their views
    new wxDocTemplate(mDocManager, _T("Citygen XML"), _T("*.cgx"), _T(""), _T("cgx"), 
		_T("Citygen Doc"),
		_T("Citygen View"),
        CLASSINFO(WorldDocument), 
		CLASSINFO(WorldView)
		);
#ifdef __WXMAC__
    wxFileName::MacRegisterDefaultTypeAndCreator( wxT("drw") , 'WXMB' , 'WXMA' ) ;
#endif

    mDocManager->SetMaxDocsOpen(1);

	InitializeOgre();

	setupResources();

	// now we need to create MainWindow singleton because
	// it creates RenderWindow, which is needed for resource loading (for example, textures and VBO)
	// we create it, but don't show to user to avoid flickness
	mWindow = new MainWindow(mDocManager);
	SetTopWindow(mWindow); // set our MainWindow the main application window
	mWindow->Show(true);
	mWindow->updateOgre();

	// Resources and resource initialization
	
	loadResources();

	//Lets Start With a Document
	mDocManager->CreateDocument(wxT("bla"), wxDOC_NEW);
	
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
      wxMessageBox( wxString(e.getFullDescription().c_str(), wxConvUTF8),
      wxT("Exception!"), 
      wxICON_EXCLAMATION);
      return false;
   }
   return true;
}


/// Method which will define the source of resources (other than current folder)
void Application::setupResources(void)
{
    // Load resource paths from config file
    ConfigFile cf;
    cf.load("./Media/resources.cfg");

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
