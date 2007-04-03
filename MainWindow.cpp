#include "stdafx.h"
#include "MainWindow.h"
#include "WorldFrame.h"

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

	IDM_GRAPH_VIEW,
	IDM_GRAPH_NODE,
	IDM_GRAPH_EDGE,
	IDM_GRAPH_CELL,

	IDM_GRAPH_SELNODE,
	IDM_GRAPH_ADDNODE,
	IDM_GRAPH_DELNODE,

	IDM_GRAPH_SELEDGE,
	IDM_GRAPH_ADDEDGE,
	IDM_GRAPH_DELEDGE,

	IDM_GRAPH_ADD,
	IDM_GRAPH_DEL,

	IDM_VIEW_GRAPH,
	IDM_VIEW_ROADS,

    IDM_OTHER_1,
    IDM_OTHER_2,
    IDM_OTHER_3
};

#ifdef USE_XPM_BITMAPS
    #include "bitmaps/newdoc.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/help.xpm"

    #include "bitmaps/view.xpm"
    #include "bitmaps/node.xpm"
	#include "bitmaps/edge.xpm"
    #include "bitmaps/cell.xpm"

    #include "bitmaps/selnode.xpm"
	#include "bitmaps/addnode.xpm"
	#include "bitmaps/delnode.xpm"
	#include "bitmaps/addedge.xpm"
	#include "bitmaps/deledge.xpm"
	
	#include "bitmaps/graph.xpm"
	#include "bitmaps/roads.xpm"

	#define TOOL_BMP(bmp) wxBitmap(bmp##_xpm)
#else
	#define TOOL_BMP(bmp) wxBITMAP(bmp)
#endif 

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "mondrian.xpm"
#endif


BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW,  MainWindow::onNew)
    EVT_MENU(wxID_OPEN,  MainWindow::onOpen)
    EVT_MENU(wxID_CLOSE, MainWindow::onClose)
    EVT_MENU(wxID_SAVE, MainWindow::onSave)
    EVT_MENU(wxID_SAVEAS, MainWindow::onSaveAs)

	EVT_MENU(IDM_GRAPH_VIEW, MainWindow::onSelectViewMode)
	EVT_MENU(IDM_GRAPH_NODE, MainWindow::onSelectNodeMode)
	EVT_MENU(IDM_GRAPH_EDGE, MainWindow::onSelectRoadMode)
	EVT_MENU(IDM_GRAPH_CELL, MainWindow::onSelectCellMode)

	EVT_MENU(IDM_GRAPH_SELNODE, MainWindow::onSelectNode)
	EVT_MENU(IDM_GRAPH_ADDNODE, MainWindow::onSelectNodeAdd)
	EVT_MENU(IDM_GRAPH_DELNODE, MainWindow::onSelectNodeDel)

END_EVENT_TABLE()


MainWindow::MainWindow(wxWindow* parent)
	: wxFrame(parent, -1, _("Citygen"),
	  wxDefaultPosition, wxSize(800,600), wxDEFAULT_FRAME_STYLE)
{
	// notify wxAUI which frame to use
	mFrameManager.SetManagedWindow(this);


	// MENU

	// file menu
	wxMenu *menuFile = new wxMenu();
	menuFile->Append(wxID_NEW, _("&New..."));
    menuFile->Append(wxID_OPEN, _("&Open..."));
    menuFile->Append(wxID_CLOSE, _("&Close"));
    menuFile->Append(wxID_SAVE, _("&Save"));
    menuFile->Append(wxID_SAVEAS, _("Save &As..."));
	menuFile->AppendSeparator();
	//menuFile->Append(wxID_PRINT, _("&Print..."));
    //menuFile->Append(wxID_PRINT_SETUP, _("Print &Setup..."));
    //menuFile->Append(wxID_PREVIEW, _("Print Pre&view"));
	//menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT, _("E&xit:\tAlt-X"), _("Exit this program."));

	// view menu
	wxMenu *menuView = new wxMenu();
	//menuView->AppendCheckItem(IDM_TOGGLE_TOOLBAR, _("Hide &toolbar"), _("Show or hide the toolbar"));

	wxMenu *menuTemplate = new wxMenu();

	// graph menu
	wxMenu *menuGraph = new wxMenu();
	menuGraph->Append(IDM_GRAPH_ADDNODE, _("Add Node"), _("Add a node to the graph"));
	menuGraph->Append(IDM_GRAPH_DELNODE, _("Delete Node"), _("Delete a node from the graph"));
	menuGraph->AppendSeparator();
	menuGraph->Append(IDM_GRAPH_ADDEDGE, _("Add Edge"), _("Add an edge to the graph"));
	menuGraph->Append(IDM_GRAPH_DELEDGE, _("Delete Edge"), _("Delete an edge from the graph"));

	// help menu
	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_HELP, _("&About...\tF1"), _("About this program."));

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, _("&File"));
	menuBar->Append(menuView, _("&View"));
	menuBar->Append(menuTemplate, _("&Template"));
	menuBar->Append(menuGraph, _("&Graph"));
	menuBar->Append(menuHelp, _("&Help"));
	SetMenuBar(menuBar);

	
	// TOOLBARS

	// create the file toolbar
	mFileToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
    mFileToolBar->AddTool(wxID_NEW, _("New"), TOOL_BMP(newdoc), _("New file"));
    mFileToolBar->AddTool(wxID_OPEN, _("Open"), TOOL_BMP(open), _("Open file"));
    mFileToolBar->AddTool(wxID_SAVE, _("Save"), TOOL_BMP(save), _("Toggle button 1"), wxITEM_CHECK);
    mFileToolBar->AddSeparator();
    mFileToolBar->AddTool(wxID_HELP, _("Help"), TOOL_BMP(help), _("Help button"), wxITEM_CHECK);
	mFileToolBar->Realize();
	mFrameManager.AddPane(mFileToolBar,  wxAuiPaneInfo().Name(_("FileTb")).
		Caption(_("File ToolBar")).ToolbarPane().Top().Row(0).Position(0).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	// create a view mode toolbar
	mViewModeToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	mViewModeToolBar->AddTool(IDM_VIEW_GRAPH, _("View Graph"), TOOL_BMP(graph), _("Select view graph mode"), wxITEM_RADIO);
	mViewModeToolBar->AddTool(IDM_VIEW_ROADS, _("View Roads"), TOOL_BMP(roads), _("Select roads graph mode"), wxITEM_RADIO);
	mViewModeToolBar->Realize();
	mViewModeToolBar->Enable(false);

	mFrameManager.AddPane(mViewModeToolBar,  wxAuiPaneInfo().Name(_("ViewTb")).
		Caption(_("View Mode ToolBar")).ToolbarPane().Top().Row(0).Position(1).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	// create an edit mode toolbar
	mEditModeToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	mEditModeToolBar->AddTool(IDM_GRAPH_VIEW, _("View"), TOOL_BMP(view), _("Select view only mode"), wxITEM_RADIO);
	mEditModeToolBar->AddTool(IDM_GRAPH_NODE, _("Node Edit"), TOOL_BMP(node), _("Select node edit mode"), wxITEM_RADIO);
	mEditModeToolBar->AddTool(IDM_GRAPH_EDGE, _("Road Edit"), TOOL_BMP(edge), _("Select road edit mode"), wxITEM_RADIO);
	mEditModeToolBar->AddTool(IDM_GRAPH_CELL, _("Cell Edit"), TOOL_BMP(cell), _("Select cell edit mode"), wxITEM_RADIO);
	mEditModeToolBar->Realize();
	mEditModeToolBar->Enable(false);

	mFrameManager.AddPane(mEditModeToolBar,  wxAuiPaneInfo().Name(_("EditModeTb")).
		Caption(_("Edit Mode ToolBar")).ToolbarPane().Top().Row(0).Position(1).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	mNodeEditToolBar = 0;
	mRoadEditToolBar = 0;


	// WINDOW PANES

	// world frame
	mWorldFrame = new WorldFrame(this);
	mFrameManager.AddPane(mWorldFrame, wxCENTER, _("Ogre Pane"));

	// log frame
	mLogFrame = new LogFrame(this);
	mFrameManager.AddPane(mLogFrame, wxBOTTOM, _("Log"));

	// tell the manager to "commit" all the changes just made
	mFrameManager.Update();

	// set the frame icon
	SetIcon(wxICON(mondrian));

	// init doc stuff
	modify(false);

	//this->onNew(wxCommandEvent());
}

MainWindow::~MainWindow()
{
	// deinitialize the frame manager
	mFrameManager.UnInit();
	delete mWorldFrame;
}

void MainWindow::updateOgre()
{
	mWorldFrame->update();
}

void MainWindow::onNew(wxCommandEvent &e)
{
	if(!onSaveModified())
        return;

	mWorldFrame->onNewDoc();
	mViewModeToolBar->Enable(true);
	mEditModeToolBar->Enable(true);
}

void MainWindow::onOpen(wxCommandEvent &e)
{
	if(!onSaveModified())
        return;

	wxString filename = wxFileSelector(_("Choose a file to open"), _(""), _(""),  
		_(""), _("Citygen XML files (*.cgx)|*.cgx|GIF files (*.gif)|*.gif"));
	if(!filename.empty())
	{
		// TODO
		TiXmlDocument doc;
		if(doc.LoadFile(filename.mb_str(wxCSConv(wxLocale::GetSystemEncoding())), TIXML_DEFAULT_ENCODING))
		{
			TiXmlHandle hDoc(&doc);
			TiXmlElement* pElem = hDoc.FirstChildElement("WorldDocument").Element();

			// should always have a valid root but handle gracefully if it does
			if(!pElem)
			{
				(void)wxMessageBox(_("Sorry, could not find XML root."), _("File error"), 
					wxOK | wxICON_EXCLAMATION, this);
				return;
			}

			// load data
			const TiXmlHandle worldRoot(pElem);
			mWorldFrame->loadXML(worldRoot);

			//
			setFilename(filename);
			mSavedYet = true;
			modify(false);
			mViewModeToolBar->Enable(true);
			mEditModeToolBar->Enable(true);
		}
		else
		{
			(void)wxMessageBox(_("Sorry, could not open this file."), _("File error"), 
				wxOK | wxICON_EXCLAMATION, this);
		}
	}
}

void MainWindow::onClose(wxCommandEvent &e)
{
	if(!onSaveModified())
        return;

	setFilename(_(""));

	mViewModeToolBar->Enable(false);
	mEditModeToolBar->Enable(false);
	mWorldFrame->onCloseDoc();
}

void MainWindow::onSave(wxCommandEvent &e)
{
	save();
}

void MainWindow::onSaveAs(wxCommandEvent &e)
{
	saveAs();
}

bool MainWindow::save()
{
    if (!isModified() && mSavedYet)
        return true;

    if ( mDocFile.empty() || !mSavedYet )
        return saveAs();

    return doSave(mDocFile);
}

bool MainWindow::saveAs()
{
	//TODO: file save as
	wxString filename = wxFileSelector(_("Choose a file to save"), _(""), _(""),  
		_(""), _("Citygen XML files (*.cgx)|*.cgx|GIF files (*.gif)|*.gif"));
	if(!filename.empty())
	{
		return doSave(filename);
	}
	return false;
}

bool MainWindow::doSave(const wxString &file)
{
	//TODO
	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "yes" );  
	doc.LinkEndChild(decl);  
	doc.LinkEndChild(mWorldFrame->saveXML());
 
	if(doc.SaveFile(file.mb_str(wxCSConv(wxLocale::GetSystemEncoding()))))
	{
		setFilename(file);
		modify(false);
		mSavedYet = true;
		return true;
	}
	else
	{
		(void)wxMessageBox(_("Sorry, could not save this file."), _("File error"),
			wxOK | wxICON_EXCLAMATION, this);
		return false;
	}
}

// true if safe to close
bool MainWindow::onSaveModified()
{
	//TODO: save modified
    if(isModified())
    {
        wxString title(wxFileNameFromPath(mDocFile));

        wxString msgTitle;
        if (!wxTheApp->GetAppName().empty())
            msgTitle = wxTheApp->GetAppName();
        else
            msgTitle = wxString(_("Warning"));

        wxString prompt;
        prompt.Printf(_("Do you want to save changes to document %s?"),
                (const wxChar *)title);
        int res = wxMessageBox(prompt, msgTitle,
                wxYES_NO|wxCANCEL|wxICON_QUESTION, this);
        if (res == wxNO)
        {
            modify(false);
            return true;
        }
        else if (res == wxYES)
            return save();
        else if (res == wxCANCEL)
            return false;
    }
    return true;
}

void MainWindow::setFilename(const wxString &file)
{
	mDocFile = file;

	if(file.empty())
		SetTitle(wxTheApp->GetAppName());
	else
		SetTitle(wxFileNameFromPath(mDocFile) + _(" - ") + wxTheApp->GetAppName());
}

void MainWindow::onSelectViewMode(wxCommandEvent &e)
{
	mEditMode = view;
	onChangeEditMode();
}

void MainWindow::onSelectNodeMode(wxCommandEvent &e)
{
	mEditMode = node;
	onChangeEditMode();
}

void MainWindow::onSelectRoadMode(wxCommandEvent &e)
{
	mEditMode = road;
	onChangeEditMode();
}

void MainWindow::onSelectCellMode(wxCommandEvent &e)
{
	mEditMode = cell;
	onChangeEditMode();
}

void MainWindow::onChangeEditMode()
{
	// change world frame edit mode
	mWorldFrame->setEditMode(mEditMode);

	// create toolbars if not created and detach if created
	mFrameManager.DetachPane(mFileToolBar);
	mFrameManager.DetachPane(mViewModeToolBar);
	mFrameManager.DetachPane(mEditModeToolBar);

	// attach required toolbars
	wxAuiPaneInfo toolbarSettings = wxAuiPaneInfo().ToolbarPane().Top().Row(0).
					LeftDockable(false).RightDockable(false).Resizable(false);
	mFrameManager.AddPane(mFileToolBar,  toolbarSettings.Name(_("FileToolBar")).
							Caption(_("File ToolBar")).Position(0)); 
	mFrameManager.AddPane(mViewModeToolBar, toolbarSettings.Name(_("ViewModeTb")).
							Caption(_("View Mode ToolBar")).Position(1)); 
	mFrameManager.AddPane(mEditModeToolBar, toolbarSettings.Name(_("EditModeTb")).
							Caption(_("Edit Mode ToolBar")).Position(2));

	// destroy all special mode toolbars
	if(mNodeEditToolBar)
	{
		mFrameManager.DetachPane(mNodeEditToolBar);
		delete mNodeEditToolBar;
		mNodeEditToolBar = 0;
	}
	if(mRoadEditToolBar)
	{
		mFrameManager.DetachPane(mRoadEditToolBar);
		delete mRoadEditToolBar;
		mRoadEditToolBar = 0;
	}

	// create the right edit mode toolbar
	switch(mEditMode)
	{
	case view:
		// no custom view toolbar
		mWorldFrame->setActiveTool(viewTool);
		break;
	case node:
		initNodeEdit();
		break;
	case road:
		initRoadEdit();
		break;
	case cell:
		break;
	}

	// update frame manager
	mFrameManager.Update();
}

void MainWindow::initNodeEdit()
{
	// create a toolbar and add them to it
	mNodeEditToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	mNodeEditToolBar->AddTool(IDM_GRAPH_SELNODE, _("Select Node"), TOOL_BMP(select), _("Select node"), wxITEM_RADIO);
	mNodeEditToolBar->AddTool(IDM_GRAPH_ADDNODE, _("Add Node"), TOOL_BMP(addnode), _("Add node"), wxITEM_RADIO);
	mNodeEditToolBar->AddTool(IDM_GRAPH_DELNODE, _("Delete Node"), TOOL_BMP(delnode), _("Delete node"), wxITEM_RADIO);
	mNodeEditToolBar->Realize();

	mFrameManager.AddPane(mNodeEditToolBar,  wxAuiPaneInfo().Name(_("NodeEditTb")).
		Caption(_("Node Edit ToolBar")).ToolbarPane().Top().Row(0).Position(3).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	mWorldFrame->setActiveTool(selNode);
}

void MainWindow::initRoadEdit()
{
	// create a toolbar and add them to it
	mRoadEditToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	mRoadEditToolBar->AddTool(IDM_GRAPH_SELEDGE, _("Select Road"), TOOL_BMP(select), _("Select road"), wxITEM_RADIO);
	mRoadEditToolBar->AddTool(IDM_GRAPH_ADDEDGE, _("Add Road"), TOOL_BMP(addedge), _("Add road"), wxITEM_RADIO);
	mRoadEditToolBar->AddTool(IDM_GRAPH_DELEDGE, _("Delete Road"), TOOL_BMP(deledge), _("Delete road"), wxITEM_RADIO);
	mRoadEditToolBar->Realize();

	mFrameManager.AddPane(mRoadEditToolBar,  wxAuiPaneInfo().Name(_("RoadEditTb")).
		Caption(_("Road Edit ToolBar")).ToolbarPane().Top().Row(0).Position(4).LeftDockable(false).
		RightDockable(false).Resizable(false)); 
}

void MainWindow::onSelectNode(wxCommandEvent &e)
{
	mWorldFrame->setActiveTool(selNode);
}

void MainWindow::onSelectNodeAdd(wxCommandEvent &e)
{
	mWorldFrame->setActiveTool(addNode);
}

void MainWindow::onSelectNodeDel(wxCommandEvent &e)
{
	mWorldFrame->setActiveTool(delNode);
}
