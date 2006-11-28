// Includes //
#include "stdafx.h"
#include "MainWindow.h"
//#include "wxOgre.h"


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
MainWindow::MainWindow(wxDocManager* docManager)
  : wxDocParentFrame(docManager, NULL, wxID_ANY, wxT("CityGen"), wxDefaultPosition, wxSize(800, 600), 
	wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxTHICK_FRAME | wxSYSTEM_MENU  | wxCAPTION | wxCLOSE_BOX),
    mFileToolBar(this, wxNewId(), TOOLBAR_STYLE),
	mEditModeToolBar(this, wxNewId(), TOOLBAR_STYLE),
	mSelectModeToolBar(this, wxNewId(), TOOLBAR_STYLE)
{
    m_horzText = false;

	mDocManager = docManager;

	mEditModeToolBar.setListener(this);

	SetIcon(wxICON(mondrian));

	mFrameManager.SetFrame(this);

	ViewPropertyPage* viewPage = new ViewPropertyPage();
	NodePropertyPage* nodePage = new NodePropertyPage();	
	RoadPropertyPage* roadPage = new RoadPropertyPage();
	CellPropertyPage* cellPage = new CellPropertyPage();


	// No Ogre app until it's given to us.
	mWorldCanvas = new WorldCanvas(0, this, nodePage, cellPage);
	mFrameManager.AddPane(mWorldCanvas, wxCENTER, wxT("Render Window"));

	// Log Window
	mLogWindow = new LogWindow(this);
	mFrameManager.AddPane(mLogWindow, wxPaneInfo().
						Name(wxT("LogWindow")).Caption(wxT("Log Window")).Bottom().
						LeftDockable(true).RightDockable(true).Resizable(true));

	/*
	 wxPropertyGridManager::wxPropertyGridManager  	(   	wxWindow *   	 parent,
		wxWindowID  	id = wxID_ANY,
		const wxPoint &  	pos = wxDefaultPosition,
		const wxSize &  	size = wxDefaultSize,
		long  	style = (0),
		const wxChar *  	name = wxPropertyGridManagerNameStr
	)  
	*/
	mPropertyGridManager = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxSize(180,200));
	mFrameManager.AddPane(mPropertyGridManager, wxPaneInfo().
						Name(wxT("PropertyInspector")).Caption(wxT("Property Inspector")).Right().
						LeftDockable(true).RightDockable(true).Resizable(true));

	// Add pages to property inspector
	mPropertyGridManager->AddPage(wxT("View Properties"), wxNullBitmap, viewPage);
	mPropertyGridManager->AddPage(wxT("Node Properties"), wxNullBitmap, nodePage);
	mPropertyGridManager->AddPage(wxT("Road Properties"), wxNullBitmap, roadPage);
	mPropertyGridManager->AddPage(wxT("Cell Properties"), wxNullBitmap, cellPage);


	// Menu
	wxMenu *menuFile = new wxMenu();
	menuFile->Append(wxID_NEW, _T("&New..."));
    menuFile->Append(wxID_OPEN, _T("&Open..."));
    menuFile->Append(wxID_CLOSE, _T("&Close"));
    menuFile->Append(wxID_SAVE, _T("&Save"));
    menuFile->Append(wxID_SAVEAS, _T("Save &As..."));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_PRINT, _T("&Print..."));
    menuFile->Append(wxID_PRINT_SETUP, _T("Print &Setup..."));
    menuFile->Append(wxID_PREVIEW, _T("Print Pre&view"));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT, _T("E&xit:\tAlt-X"), _T("Exit this program."));

	// A nice touch: a history of files visited. Use this menu.
    mDocManager->FileHistoryUseMenu(menuFile);

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

	mSelectModeToolBar.setListener((SelectModeListener*) mWorldCanvas);
	mFrameManager.AddPane(&mSelectModeToolBar, wxPaneInfo().
				Name(wxT("SelectModeToolBar")).Caption(wxT("Node Edit ToolBar")).
				ToolbarPane().Top().
				LeftDockable(true).RightDockable(true).Resizable(false));

	enableDocumentToolbars(false);

	mFrameManager.Update();

	// Status Bar
	CreateStatusBar(2);
	SetStatusText(_T("Primary Road Network Mode."));

	//mWorldCanvas->prepare();
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
	mPropertyGridManager->SelectPage(mode);
	mWorldCanvas->setEditMode(mode);
	mSelectModeToolBar.setEditMode(mode);

	//Update the Toolbar Size & FrameManager
	mFrameManager.GetPane(&mSelectModeToolBar).BestSize(mSelectModeToolBar.GetBestSize());
	mFrameManager.Update();
}

template<> MainWindow* Ogre::Singleton<MainWindow>::ms_Singleton = 0;
MainWindow& MainWindow::getSingleton()
{
	return ( *ms_Singleton );
}

MainWindow* MainWindow::getSingletonPtr()
{
	return ms_Singleton;
}

void MainWindow::enableDocumentToolbars(bool enable) {
	mSelectModeToolBar.Enable(enable);
	mEditModeToolBar.Enable(enable);
}

void MainWindow::updateOgre()
{
	mWorldCanvas->Update();
}
