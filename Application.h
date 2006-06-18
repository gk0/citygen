#include "stdafx.h"
#include "MainWindow.h"

// Application class declaration //
class Application : public wxApp
{
private:
	Ogre::Root *mRoot;
	MainWindow *mWindow;

public:
	virtual bool OnInit();
	virtual int OnExit();

	bool InitializeOgre();
	void setupResources(void);
	void loadResources(void);
};