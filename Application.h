#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "stdafx.h"
#include <wx/app.h>
#include <wx/cmdline.h>
#include <OgreRoot.h>

class MainWindow;

/**Represents the application itself.
 * It is used to:
 *  set and get application-wide properties;
 *  implement the windowing system message or event loop;
 *  initiate application processing via wxApp::onInit;
 *  allow default processing of events not handled by other objects in the application. 
 */
class Application : public wxApp
{
	
private:
	Ogre::Root*		_root;
	MainWindow*		_window;
	std::string		_inputFile;
	std::string		_outputFile;

public:
	Application();
	virtual ~Application(){};
	virtual bool OnInit();
	virtual int OnExit();
	virtual int OnRun();

	bool InitializeOgre();
	void setupResources(void);
	void loadResources(void);
	virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};

static const wxCmdLineEntryDesc cmdLineDesc [] =
{
    { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("Displays help on the command line parameters"),
          wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_PARAM, _T(""), _T(""), _T("input file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
	{ wxCMD_LINE_PARAM, _T(""), _T(""), _T("output file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
    { wxCMD_LINE_NONE }
};

#endif
