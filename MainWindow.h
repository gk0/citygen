#ifndef WXTEST_H
#define WXTEST_H

#include "stdafx.h"
#include "WorldWindow.h"
#include "LogWindow.h"
#include "FileToolBar.h"
#include "EditModeToolBar.h"
#include "SelectModeToolBar.h"

class wxDocManager;

// MainWindow class declaration //
class MainWindow : public wxDocParentFrame, public Ogre::Singleton<MainWindow>, EditModeListener
{
private:
	DECLARE_EVENT_TABLE()

	wxDocManager* mDocManager;

	wxToolBar* mToolBar;
	//wxToolBar* mGraphToolBar;
	wxFrameManager mFrameManager;
	FileToolBar mFileToolBar;
	EditModeToolBar mEditModeToolBar;
	SelectModeToolBar mSelectModeToolBar;
	
	
	LogWindow *mLogWindow;

	long mMouseX, mMouseY;
	bool m_horzText;

public:
	WorldWindow *mWorldWindow;

	MainWindow(wxDocManager* docManager);
	~MainWindow();

	void onQuit(wxCommandEvent &e);
	void onAbout(wxCommandEvent &e);
	void onNodeAdd(wxCommandEvent &e);
	void onNodeDelete(wxCommandEvent &e);


	void onViewModeSelect(wxCommandEvent &e);
	void onNodeModeSelect(wxCommandEvent &e);
	void onEdgeModeSelect(wxCommandEvent &e);

	void CreateToolbar();
	void CreateGraphToolbar();

	void setEditMode(EditModeListener::EditMode mode);

	void enableDocumentToolbars(bool enable=true);

	
	static MainWindow& getSingleton();
	static MainWindow* getSingletonPtr();

};

#endif
