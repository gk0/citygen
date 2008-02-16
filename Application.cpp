#include "stdafx.h"
#include "Application.h"
#include "MainWindow.h"

#include <wx/msgdlg.h>
#include <wx/image.h>
#include <OgreConfigFile.h>

using namespace Ogre;

// Platform specific crap that creates the application, etc. //
IMPLEMENT_APP(Application);


Application::Application()
: wxApp(),
  _root(0),
  _window(0)
{
}

// Function definitions //
// App Init
bool Application::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxJPEGHandler);

	if (!wxApp::OnInit()) 
		return false;

	InitializeOgre();

	setupResources();
	//loadResources();

	_window = new MainWindow(0); 

	// now we need to create MainWindow singleton because
	// it creates RenderWindow, which is needed for resource loading (for example, textures and VBO)
	// we create it, but don't show to user to avoid flickering
	SetTopWindow(_window); // set our MainWindow the main application window
	_window->Show(true);
	_window->init();
	_window->updateOgre();

	// Resources and resource initialization
	
	loadResources();

	if(_inputFile.size() > 1)
	{
		std::replace(_inputFile.begin(), _inputFile.end(), '\\', '/');
		_window->openFile(_U(_inputFile.c_str()));
	}
	else
	{
		//_window->openFile(_("C:\\Documents and Settings\\George\\Desktop\\cgx\\test city.cgx"));
		//_window->openFile(_("C:/Documents and Settings/George/Desktop/cgx2/testo2.cgx"));
		//_window->openFile(_("C:/Documents and Settings/George/Desktop/cgx2/shit.cgx"));
		_window->openFile(_("C:/Documents and Settings/George/Desktop/cgx2/city neue 4.cgx"));
		//_window->donew();
	}

	//_window->onExport(wxCommandEvent());

	if(_outputFile.size() > 1)
	{
		std::replace(_outputFile.begin(), _outputFile.end(), '\\', '/');
		_window->saveFile(_U(_outputFile.c_str()));
	}
	
	// All clear!
	return true;
}

// App Exit
int Application::OnExit()
{
	// Clean Up OGRE
	if (_root)
		delete _root;

	// All done here.
	return 0;
}


bool Application::InitializeOgre()
{
	// Make the root
	_root = new Root();

	try
	{
		if(!_root->restoreConfig())
		{
			_root->showConfigDialog();
		}
		// initialise with false doesn't create a window
		_root->initialise(false);
   }
   catch(Ogre::Exception &e)
   {
	  String s = "OgreView::Init() - Exception:\n" + e.getFullDescription() + "\n";
	  LogManager::getSingleton().logMessage(s, LML_CRITICAL);
      wxMessageBox(wxString(e.getFullDescription().c_str(), wxConvUTF8),
      _("Exception!"), 
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

int Application::OnRun()
{
	return wxApp::OnRun();
}


void Application::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc(cmdLineDesc);

    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}
 
bool Application::OnCmdLineParsed(wxCmdLineParser& parser)
{
	// get unnamed parameters
	if(parser.GetParamCount() >= 1) _inputFile = std::string(_C(parser.GetParam(0)));
	if(parser.GetParamCount() >= 2) _outputFile = std::string(_C(parser.GetParam(1)));
 
    return true;
}