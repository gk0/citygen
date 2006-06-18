// Includes //
#include "stdafx.h"
#include "MainWindow.h"
#include "OgreView.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------
const int ID_TOOLBAR = 500;

static const long TOOLBAR_STYLE = wxNO_BORDER | wxTB_NODIVIDER | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT/* | wxTB_HORZ_LAYOUT*/;

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
    IDM_TOGGLE_HORIZONTAL_TEXT,
    IDM_TOGGLE_ANOTHER_TOOLBAR,
    IDM_CHANGE_TOOLTIP,
    IDM_SHOW_TEXT,
    IDM_SHOW_ICONS,
    IDM_SHOW_BOTH,

	IDM_GRAPH_VIEW,
	IDM_GRAPH_NODE,
	IDM_GRAPH_EDGE,
	IDM_GRAPH_ADD,
	IDM_GRAPH_DEL,
	IDM_GRAPH_ADDNODE,
	IDM_GRAPH_DELNODE,
	IDM_GRAPH_ADDEDGE,
	IDM_GRAPH_DELEDGE,

    IDM_OTHER_1,
    IDM_OTHER_2,
    IDM_OTHER_3
};

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "mondrian.xpm"
#endif

#if USE_XPM_BITMAPS
    #include "bitmaps/newdoc.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/cut.xpm"
    #include "bitmaps/preview.xpm"  // paste XPM
    #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS


// Event Table //
BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_EXIT, MainWindow::OnQuit)

	EVT_MENU(wxID_HELP, MainWindow::OnAbout)

	EVT_MENU(IDM_GRAPH_VIEW, MainWindow::OnViewModeSelect)
	EVT_MENU(IDM_GRAPH_NODE, MainWindow::OnNodeModeSelect)
	EVT_MENU(IDM_GRAPH_EDGE, MainWindow::OnEdgeModeSelect)

	//EVT_MENU(IDM_GRAPH_ADDNODE, MainWindow::OnNodeAdd)
	EVT_MENU(IDM_GRAPH_DELNODE, MainWindow::OnNodeDelete)

END_EVENT_TABLE()

void MainWindow::OnNodeDelete(wxCommandEvent &e) {
	//mOgreWindow->deleteSelectedNode();
	mOgreWindow->update();
}

void MainWindow::OnViewModeSelect(wxCommandEvent &e) {
	//mOgreWindow->setMode(normal);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDNODE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELNODE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDEDGE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELEDGE, false);
}
void MainWindow::OnNodeModeSelect(wxCommandEvent &e) {
	//mOgreWindow->setMode(node);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDNODE, true);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELNODE, true);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDEDGE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELEDGE, false);
}
void MainWindow::OnEdgeModeSelect(wxCommandEvent &e) {
	//mOgreWindow->setMode(edge);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDEDGE, true);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELEDGE, true);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDNODE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELNODE, false);
}



// Frame Constructor
MainWindow::MainWindow() : wxFrame( NULL, wxID_ANY, "CityGen", wxDefaultPosition,
								wxSize( 640, 480 ),
								wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxMAXIMIZE_BOX |
								wxTHICK_FRAME   | wxSYSTEM_MENU  | wxCAPTION | wxCLOSE_BOX)
{
	m_smallToolbar = true;
    m_horzText = false;

	SetIcon(wxICON(mondrian));

	mFrameManager.SetFrame(this);

	// No Ogre app until it's given to us.
	mOgreWindow = new OgreView(this);
	mFrameManager.AddPane(mOgreWindow, wxCENTER, wxT("Render Window"));

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
	CreateToolbar();
	mFrameManager.AddPane(mToolBar, wxPaneInfo().
					Name(wxT("ToolsToolBar")).Caption(wxT("Tools")).
					ToolbarPane().Top().
					LeftDockable(false).RightDockable(false).Resizable(false));

		//Toolbar
	CreateGraphToolbar();
	mFrameManager.AddPane(mGraphToolBar, wxPaneInfo().
					Name(wxT("GraphToolBar")).Caption(wxT("Primary Road Graph Tools")).
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

// On Quit
void MainWindow::OnQuit(wxCommandEvent &WXUNUSED(e))
{
	Close(TRUE);
}

// On About
void MainWindow::OnAbout(wxCommandEvent &WXUNUSED(e))
{
    wxString msg;
    msg.Printf( _T("This is the About dialog of the CityGen 5 the sequel.\n"));

    wxMessageBox(msg, _T("About wxWidgets/OGRE Integration Demo"), wxOK | wxICON_INFORMATION, this);
}

void MainWindow::CreateToolbar()
{
    // delete and recreate the toolbar
	//wxToolBarBase* mToolBar = GetToolBar();
    
	long style = mToolBar ? mToolBar->GetWindowStyle() : TOOLBAR_STYLE;
    delete mToolBar;

    SetToolBar(NULL);

	style &= ~wxTB_HORZ_LAYOUT;
    if ( style & wxTB_TEXT && !(style & wxTB_NOICONS) && m_horzText )
       style |= wxTB_HORZ_LAYOUT;

	mToolBar = new wxToolBar(this, ID_TOOLBAR, wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);

    // Set up toolbar
    enum
    {
        Tool_newdoc,
        Tool_open,
        Tool_save,
        /*Tool_copy,
        Tool_cut,
        Tool_paste,
        Tool_print,*/
        Tool_help,
        Tool_Max
    };

    wxBitmap toolBarBitmaps[Tool_Max];

#if USE_XPM_BITMAPS
    #define INIT_TOOL_BMP(bmp) \
        toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp##_xpm)
#else // !USE_XPM_BITMAPS
    #define INIT_TOOL_BMP(bmp) \
        toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)
#endif // USE_XPM_BITMAPS/!USE_XPM_BITMAPS

    INIT_TOOL_BMP(newdoc);
    INIT_TOOL_BMP(open);
    INIT_TOOL_BMP(save);
    /*INIT_TOOL_BMP(copy);
    INIT_TOOL_BMP(cut);
    INIT_TOOL_BMP(paste);
    INIT_TOOL_BMP(print);*/
    INIT_TOOL_BMP(help);

    int w = toolBarBitmaps[Tool_newdoc].GetWidth(),
        h = toolBarBitmaps[Tool_newdoc].GetHeight();

    if ( !m_smallToolbar )
    {
        w *= 2;
        h *= 2;

        for ( size_t n = Tool_newdoc; n < WXSIZEOF(toolBarBitmaps); n++ )
        {
            toolBarBitmaps[n] =
                wxBitmap(toolBarBitmaps[n].ConvertToImage().Scale(w, h));
        }

        mToolBar->SetToolBitmapSize(wxSize(w, h));
    }

    mToolBar->AddTool(wxID_NEW, _T("New"), toolBarBitmaps[Tool_newdoc], _T("New file"));
    mToolBar->AddTool(wxID_OPEN, _T("Open"), toolBarBitmaps[Tool_open], _T("Open file"));
    mToolBar->AddTool(wxID_SAVE, _T("Save"), toolBarBitmaps[Tool_save], _T("Toggle button 1"), wxITEM_CHECK);
/*    mToolBar->AddTool(wxID_COPY, _T("Copy"), toolBarBitmaps[Tool_copy], _T("Toggle button 2"), wxITEM_CHECK);
    mToolBar->AddTool(wxID_CUT, _T("Cut"), toolBarBitmaps[Tool_cut], _T("Toggle/Untoggle help button"));
    mToolBar->AddTool(wxID_PASTE, _T("Paste"), toolBarBitmaps[Tool_paste], _T("Paste"));
	mToolBar->AddTool(wxID_PRINT, _T("Print"), toolBarBitmaps[Tool_print],
                         _T("Delete this tool. This is a very long tooltip to test whether it does the right thing when the tooltip is more than Windows can cope with."));
*/
    mToolBar->AddSeparator();
    mToolBar->AddTool(wxID_HELP, _T("Help"), toolBarBitmaps[Tool_help], _T("Help button"), wxITEM_CHECK);

    // after adding the buttons to the toolbar, must call Realize() to reflect the changes
    mToolBar->Realize();
}

void MainWindow::CreateGraphToolbar()
{
    // delete and recreate the toolbar
	//wxToolBarBase* mGraphToolBar = GetToolBar();
    
	long style = mGraphToolBar ? mGraphToolBar->GetWindowStyle() : TOOLBAR_STYLE;
    delete mGraphToolBar;

    SetToolBar(NULL);

	style &= ~wxTB_HORZ_LAYOUT;
    if ( style & wxTB_TEXT && !(style & wxTB_NOICONS) && m_horzText )
       style |= wxTB_HORZ_LAYOUT;

	mGraphToolBar = new wxToolBar(this, ID_TOOLBAR, wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);

    // Set up toolbar
    enum
    {
		Tool_view,
		Tool_node,
		Tool_edge,
        Tool_addnode,
        Tool_delnode,
        Tool_addedge,
        Tool_deledge,
        Tool_Max
    };

    wxBitmap toolBarBitmaps[Tool_Max];

#if USE_XPM_BITMAPS
    #define INIT_TOOL_BMP(bmp) \
        toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp##_xpm)
#else // !USE_XPM_BITMAPS
    #define INIT_TOOL_BMP(bmp) \
        toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)
#endif // USE_XPM_BITMAPS/!USE_XPM_BITMAPS

	INIT_TOOL_BMP(view);
	INIT_TOOL_BMP(node);
	INIT_TOOL_BMP(edge);
    INIT_TOOL_BMP(addnode);
    INIT_TOOL_BMP(delnode);
    INIT_TOOL_BMP(addedge);
    INIT_TOOL_BMP(deledge);

    int w = toolBarBitmaps[Tool_addnode].GetWidth(),
        h = toolBarBitmaps[Tool_addnode].GetHeight();

    if ( !m_smallToolbar )
    {
        w *= 2;
        h *= 2;

        for ( size_t n = Tool_addnode; n < WXSIZEOF(toolBarBitmaps); n++ )
        {
            toolBarBitmaps[n] =
                wxBitmap(toolBarBitmaps[n].ConvertToImage().Scale(w, h));
        }

        mGraphToolBar->SetToolBitmapSize(wxSize(w, h));
    }
	mGraphToolBar->AddTool(IDM_GRAPH_VIEW, _T("View"), toolBarBitmaps[Tool_view], _T("Select view only mode"), wxITEM_RADIO);
    mGraphToolBar->AddTool(IDM_GRAPH_NODE, _T("Node Edit"), toolBarBitmaps[Tool_node], _T("Select node edit mode"), wxITEM_RADIO);
	mGraphToolBar->AddTool(IDM_GRAPH_EDGE, _T("Road Edit"), toolBarBitmaps[Tool_edge], _T("Select road edit mode"), wxITEM_RADIO);
	mGraphToolBar->AddSeparator();
	mGraphToolBar->AddTool(IDM_GRAPH_ADDNODE, _T("Add Node"), toolBarBitmaps[Tool_addnode], _T("Add node"));
    mGraphToolBar->AddTool(IDM_GRAPH_DELNODE, _T("Delete Node"), toolBarBitmaps[Tool_delnode], _T("Delete node"));
	mGraphToolBar->AddSeparator();
	mGraphToolBar->AddTool(IDM_GRAPH_ADDEDGE, _T("Add Road"), toolBarBitmaps[Tool_deledge], _T("Add road"));
    mGraphToolBar->AddTool(IDM_GRAPH_DELEDGE, _T("Delete Road"), toolBarBitmaps[Tool_deledge], _T("Delete road"));

	//
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDNODE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELNODE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_ADDEDGE, false);
	mGraphToolBar->EnableTool(IDM_GRAPH_DELEDGE, false);

    // after adding the buttons to the toolbar, must call Realize() to reflect the changes
    mGraphToolBar->Realize();
}