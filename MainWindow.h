#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "stdafx.h"
#include "LogFrame.h"

class WorldFrame;

class MainWindow : public wxFrame
{
public:
	enum EditMode {
		view,
		node,
		road,
		cell
	};
	enum ActiveTool {
		viewTool,
		selNode,
		addNode,
		delNode,
		selRoad,
		addRoad,
		delRoad,
		selCell
	};

private:
	wxAuiManager mFrameManager;
	WorldFrame *mWorldFrame;
	LogFrame *mLogFrame;

	// edit mode
	EditMode mEditMode;
	ActiveTool mActiveTool;

	wxToolBar *mFileToolBar, *mViewModeToolBar, *mEditModeToolBar, 
		*mNodeEditToolBar, *mRoadEditToolBar;

	// doc variables
	wxString mDocFile;
    bool mSavedYet;
	bool mModified;

private:
	void setFilename(const wxString &file);
	bool save();
	bool saveAs();
	bool doSave(const wxString &file);
	bool onSaveModified();
	void initEditModeToolBar();
	void onChangeEditMode();
	void initNodeEdit();
	void initRoadEdit();
	void initCellEdit();

protected:
	void onOpen(wxCommandEvent &e);
	void onClose(wxCommandEvent &e);
	void onNew(wxCommandEvent &e);
	void onSave(wxCommandEvent &e);
	void onSaveAs(wxCommandEvent &e);

	void onSelectViewMode(wxCommandEvent &e);
	void onSelectNodeMode(wxCommandEvent &e);
	void onSelectRoadMode(wxCommandEvent &e);
	void onSelectCellMode(wxCommandEvent &e);

	void onSelectNode(wxCommandEvent &e);
	void onSelectNodeAdd(wxCommandEvent &e);
	void onSelectNodeDel(wxCommandEvent &e);

	DECLARE_EVENT_TABLE();

public:
	MainWindow(wxWindow* parent);
	~MainWindow();

	void updateOgre();
	void modify(bool m) { mModified = m; }
	bool isModified() { return mModified; }

};

#endif //__MainWindow_H__
