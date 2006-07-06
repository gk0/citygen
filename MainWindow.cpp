// Includes //
#include "stdafx.h"
#include "MainWindow.h"
#include "OgreView.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------
const int ID_TOOLBAR = 500;

static const long TOOLBAR_STYLE = wxNO_BORDER | wxTB_NODIVIDER | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT /* | wxTB_HORZ_LAYOUT*/;

enum
{
    IDM_TOGGLETOOLBARSIZE = 200,
    IDM_TOGGLETOOLBARORIENT,
    IDM_TOGGLETOOLBARROWS,
    IDM_TOGGLECUSTOMDISABLED,
    IDM_ENABLEPRINT,
    IDM_DELETEPRINT,
    IDM_INSERTPRINT,
    IDM_TOGGLEHELP,
    IDM_TOGGLE_TOOLBAR,
    IDM_TOGGLE_HORIZonTAL_TEXT,
    IDM_TOGGLE_ANOTHER_TOOLBAR,
    IDM_CHANGE_TOOLTIP,
    IDM_SHOW_TEXT,
    IDM_SHOW_IConS,
    IDM_SHOW_BOTH,

	IDM_GRAPH_ADDNODE,
	IDM_GRAPH_DELNODE,

	IDM_GRAPH_ADD,
	IDM_GRAPH_DEL,
	IDM_GRAPH_ADDEDGE,
	IDM_GRAPH_DELEDGE,

    IDM_OTHER_1,
    IDM_OTHER_2,
    IDM_OTHER_3
};

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "mondrian.xpm"
#endif


// Event Table //
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_EXIT, MainWindow::onQuit)
	EVT_MENU(wxID_HELP, MainWindow::onAbout)
	//EVT_MENU(IDM_GRAPH_VIEW, MainWindow::onViewModeSelect)
	//EVT_MENU(IDM_GRAPH_NODE, MainWindow::onNodeModeSelect)
	//EVT_MENU(IDM_GRAPH_EDGE, MainWindow::onEdgeModeSelect)
	//EVT_MENU(IDM_GRAPH_ADDNODE, MainWindow::onNodeAdd)
	//EVT_MENU(IDM_GRAPH_DELNODE, MainWindow::onNodeDelete)
END_EVENT_TABLE()



// Frame Constructor
MainWindow::MainWindow()
  : wxFrame( NULL, wxID_ANY, "CityGen", wxDefaultPosition, wxSize( 640, 480 ),
	  wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxTHICK_FRAME | wxSYSTEM_MENU  | wxCAPTION | wxCLOSE_BOX),
    mFileToolBar(this, wxNewId(), TOOLBAR_STYLE),
	mEditModeToolBar(this, wxNewId(), TOOLBAR_STYLE),
	mSelectModeToolBar(this, wxNewId(), TOOLBAR_STYLE)
{
    m_horzText = false;

	mEditModeToolBar.setListener(this);

	SetIcon(wxICON(mondrian));

	mFrameManager.SetFrame(this);

	// No Ogre app until it's given to us.
	mWorldView = new WorldView(this);
	mFrameManager.AddPane(mWorldView, wxCENTER, wxT("Render Window"));

	// Log Window
	mLogWindow = new LogWindow(this);
	mFrameManager.AddPane(mLogWindow, wxPaneInfo().
						Name(wxT("LogWindow")).Caption(wxT("Log Window")).Bottom().
						LeftDockable(false).RightDockable(false).Resizable(true));

	// Menu
	wxMenu *menuFile = new wxMenu();
	menuFile->Append(wxID_EXIT, _T("E&xit:\tAlt-X"), _T("Exit this program."));

	wxMenu *menuView = new wxMenu();
	//menuView->AppendCheckItem(IDM_TOGGLE_TOOLBAR, _T("Hide &toolbar"), _T("Show or hide the toolbar"));

	wxMenu *menuTemplate = new wxMenu();

	wxMenu *menuGraph = new wxMenu();
	menuGraph->Append(IDM_GRAPH_ADDNODE, _T("Add Node"), _T("Add a node to the graph"));
	menuGraph->Append(IDM_GRAPH_DELNODE, _T("Delete Node"), _T("Delete a node from the graph"));
	menuGraph->AppendSeparator();
	menuGraph->Append(IDM_GRAPH_ADDEDGE, _T("Add Edge"), _T("Add an edge to the graph"));
	menuGraph->Append(IDM_GRAPH_DELEDGE, _T("Delete Edge"), _T("Delete an edge from the graph"));


	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_HELP, _T("&About...\tF1"), _T("About this program."));

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, _T("&File"));
	menuBar->Append(menuView, _T("&View"));
	menuBar->Append(menuTemplate, _T("&Template"));
	menuBar->Append(menuGraph, _T("&Graph"));
	menuBar->Append(menuHelp, _T("&Help"));
	SetMenuBar(menuBar);
	
	//Toolbar
	mFrameManager.AddPane(&mFileToolBar, wxPaneInfo().
					Name(wxT("FileToolBar")).Caption(wxT("File ToolBar")).
					ToolbarPane().Top().
					LeftDockable(false).RightDockable(false).Resizable(false));

	//Toolbar
	mFrameManager.AddPane(&mEditModeToolBar, wxPaneInfo().
					Name(wxT("EditModeToolBar")).Caption(wxT("Edit Mode ToolBar")).
					ToolbarPane().Top().
					LeftDockable(true).RightDockable(true).Resizable(false));

	mSelectModeToolBar.setListener((SelectModeListener*) mWorldView);
	mFrameManager.AddPane(&mSelectModeToolBar, wxPaneInfo().
				Name(wxT("SelectModeToolBar")).Caption(wxT("Node Edit ToolBar")).
				ToolbarPane().Top().
				LeftDockable(true).RightDockable(true).Resizable(false));

	mFrameManager.Update();

	// Status Bar
	CreateStatusBar(2);
	SetStatusText(_T("Primary Road Network Mode."));
}

// Frame Destructor
MainWindow::~MainWindow()
{
	// deinitialize the frame manager
    mFrameManager.UnInit();
}

// on Quit
void MainWindow::onQuit(wxCommandEvent &e)
{
	Close(TRUE);
}

// on About
void MainWindow::onAbout(wxCommandEvent &e)
{
    wxString msg;
    msg.Printf( _T("This is the About dialog of the CityGen 5 the sequel.\n"));

    wxMessageBox(msg, _T("About wxWidgets/OGRE Integration Demo"), wxOK | wxICON_INFORMATION, this);
}

void MainWindow::setEditMode(EditModeListener::EditMode mode) 
{
	mWorldView->setEditMode(mode);
	mSelectModeToolBar.setEditMode(mode);

	//Update the Toolbar Size & FrameManager
	mFrameManager.GetPane(&mSelectModeToolBar).BestSize(mSelectModeToolBar.GetBestSize());
	mFrameManager.Update();
}
