#ifndef WXTEST_H
#define WXTEST_H

#include "stdafx.h"
#include "WorldView.h"
#include "LogWindow.h"

// MainWindow class declaration //
class MainWindow : public wxFrame
{
private:
	DECLARE_EVENT_TABLE()

	wxToolBar* mToolBar;
	wxToolBar* mGraphToolBar;
	wxFrameManager mFrameManager;
	
	OgreView *mOgreWindow;
	LogWindow *mLogWindow;

	long mMouseX, mMouseY;
	bool m_smallToolbar, m_horzText;

public:
	MainWindow();
	~MainWindow();

	void OnQuit(wxCommandEvent &e);
	void OnAbout(wxCommandEvent &e);
	void OnNodeAdd(wxCommandEvent &e);
	void OnNodeDelete(wxCommandEvent &e);


	void OnViewModeSelect(wxCommandEvent &e);
	void OnNodeModeSelect(wxCommandEvent &e);
	void OnEdgeModeSelect(wxCommandEvent &e);

	void CreateToolbar();
	void CreateGraphToolbar();
};

#endif
