#ifndef WXTEST_H
#define WXTEST_H

#include "stdafx.h"
#include "WorldView.h"
#include "LogWindow.h"
#include "FileToolBar.h"
#include "EditModeToolBar.h"
#include "SelectModeToolBar.h"

// MainWindow class declaration //
class MainWindow : public wxFrame, EditModeListener
{
private:
	DECLARE_EVENT_TABLE()

	wxToolBar* mToolBar;
	//wxToolBar* mGraphToolBar;
	wxFrameManager mFrameManager;
	FileToolBar mFileToolBar;
	EditModeToolBar mEditModeToolBar;
	SelectModeToolBar mSelectModeToolBar;
	
	WorldView *mWorldView;
	LogWindow *mLogWindow;

	long mMouseX, mMouseY;
	bool m_horzText;

public:
	MainWindow();
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
};

#endif
