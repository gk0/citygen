#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "stdafx.h"
#include <wx/app.h>
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

public:
	Application();
	virtual ~Application(){};
	virtual bool OnInit();
	virtual int OnExit();

	bool InitializeOgre();
	void setupResources(void);
	void loadResources(void);
};

#endif
