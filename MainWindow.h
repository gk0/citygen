#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "stdafx.h"
#include "LogFrame.h"

class WorldFrame;
class ViewPropertyPage;
class NodePropertyPage;
class RoadPropertyPage;
class CellPropertyPage;

class MainWindow : public wxFrame
{
public:
	enum ViewMode {
		view_primary,
		view_cell,
		view_box,
		view_building
	};

	enum ToolsetMode {
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
		/*addRoad,
		delRoad,*/
		selCell
	};

private:
	wxAuiManager	_frameManager;
	WorldFrame*		_worldFrame;
	LogFrame*		_logFrame;

	// edit mode
	ToolsetMode		_toolsetMode;
	ActiveTool		_activeTool;

	wxToolBar		*_fileToolBar, *_viewModeToolBar, *_toolsetModeToolBar, 
					*_nodeEditToolBar, *_roadEditToolBar;

	wxPropertyGridManager*	_propertyGridManager;
	ViewPropertyPage*		_viewPropertyPage;
	NodePropertyPage*		_nodePropertyPage;
	RoadPropertyPage*		_roadPropertyPage;
	CellPropertyPage*		_cellPropertyPage;

	// doc variables
	wxString		_docFile;
    bool			_savedYet;
	bool			_modified;

private:
	void setFilename(const wxString &file);
	bool save();
	bool saveAs();
	void doOpen(const wxString &file);
	bool doSave(const wxString &file);
	bool onSaveModified();
	void initToolsetModeToolBar();
	void onChangeToolsetMode();
	void initNodeEdit();
	void initRoadEdit();
	void initCellEdit();

protected:
	void onOpen(wxCommandEvent &e);
	void onClose(wxCommandEvent &e);
	void onNew(wxCommandEvent &e);
	void onSave(wxCommandEvent &e);
	void onSaveAs(wxCommandEvent &e);

	void onSelectViewPrimary(wxCommandEvent &e);
	void onSelectViewCell(wxCommandEvent &e);
	void onSelectViewBox(wxCommandEvent &e);
	void onSelectViewBuilding(wxCommandEvent &e);

	void onSelectToolsetView(wxCommandEvent &e);
	void onSelectToolsetNode(wxCommandEvent &e);
	void onSelectToolsetRoad(wxCommandEvent &e);
	void onSelectToolsetCell(wxCommandEvent &e);

	void onSelectNode(wxCommandEvent &e);
	void onSelectNodeAdd(wxCommandEvent &e);
	void onSelectNodeDel(wxCommandEvent &e);

	DECLARE_EVENT_TABLE();

public:
	MainWindow(wxWindow* parent);
	~MainWindow();

	void init();
	void updateOgre();
	void modify(bool m);
	bool isModified();
	void updateProperties();
	void openFile(const wxString& filename) { doOpen(filename); }

};

#endif //__MainWindow_H__
