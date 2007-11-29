#include "stdafx.h"
#include "MainWindow.h"
#include "WorldFrame.h"
#include "ViewPropertyPage.h"
#include "NodePropertyPage.h"
#include "RoadPropertyPage.h"
#include "CellPropertyPage.h"

#include "ColladaDoc.h"

#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------
const int ID_TOOLBAR = 500;

#ifdef WIN32
static const long TOOLBAR_STYLE = wxNO_BORDER | wxTB_NODIVIDER | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_NO_TOOLTIPS;
#else
static const long TOOLBAR_STYLE = wxNO_BORDER | wxTB_NODIVIDER | wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT;
#endif

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

	IDM_TOOLSET_VIEW,
	IDM_TOOLSET_NODE,
	IDM_TOOLSET_EDGE,
	IDM_TOOLSET_CELL,

	IDM_NODE_SELECT,
	IDM_NODE_ADD,
	IDM_NODE_DELETE,

	IDM_ROAD_SELECT,
	IDM_ROAD_ADD,
	IDM_ROAD_DELETE,

	IDM_GRAPH_ADD,
	IDM_GRAPH_DEL,

	IDM_VIEW_PRIMARY,
	IDM_VIEW_CELL,
	IDM_VIEW_BOX,
	IDM_VIEW_BUILDING,

	IDM_EXPORT,

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
	#include "bitmaps/all.xpm"
	#include "bitmaps/building.xpm"

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
	EVT_MENU(IDM_EXPORT, MainWindow::onExport)
    EVT_MENU(wxID_SAVE, MainWindow::onSave)
    EVT_MENU(wxID_SAVEAS, MainWindow::onSaveAs)

	EVT_MENU(IDM_VIEW_PRIMARY, MainWindow::onSelectViewPrimary)
	EVT_MENU(IDM_VIEW_CELL, MainWindow::onSelectViewCell)
	EVT_MENU(IDM_VIEW_BOX, MainWindow::onSelectViewBox)
	//EVT_MENU(IDM_VIEW_BUILDING, MainWindow::onSelectViewBuilding)


	EVT_MENU(IDM_TOOLSET_VIEW, MainWindow::onSelectToolsetView)
	EVT_MENU(IDM_TOOLSET_NODE, MainWindow::onSelectToolsetNode)
	EVT_MENU(IDM_TOOLSET_EDGE, MainWindow::onSelectToolsetRoad)
	EVT_MENU(IDM_TOOLSET_CELL, MainWindow::onSelectToolsetCell)

	EVT_MENU(IDM_NODE_SELECT, MainWindow::onSelectNode)
	EVT_MENU(IDM_NODE_ADD, MainWindow::onSelectNodeAdd)
	EVT_MENU(IDM_NODE_DELETE, MainWindow::onSelectNodeDel)

	EVT_MENU(IDM_ROAD_SELECT, MainWindow::onSelectRoad)
	EVT_MENU(IDM_ROAD_ADD, MainWindow::onSelectRoadAdd)
	EVT_MENU(IDM_ROAD_DELETE, MainWindow::onSelectRoadDel)

END_EVENT_TABLE()


MainWindow::MainWindow(wxWindow* parent)
	: wxFrame(parent, -1, _("citygen"),
	  wxDefaultPosition, wxSize(1024,768), wxDEFAULT_FRAME_STYLE)
{
	// notify wxAUI which frame to use
	_frameManager.SetManagedWindow(this);


	// MENU

	// file menu
	wxMenu *menuFile = new wxMenu();
	menuFile->Append(wxID_NEW, _("&New..."));
    menuFile->Append(wxID_OPEN, _("&Open..."));
    menuFile->Append(wxID_CLOSE, _("&Close"));
	menuFile->AppendSeparator();
	menuFile->Append(IDM_EXPORT, _("&Export"));
	menuFile->AppendSeparator();
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

	// tool menu
	wxMenu *toolGraph = new wxMenu();
	toolGraph->Append(IDM_NODE_SELECT, _("Select Node"), _("Select a node from the graph"));
	toolGraph->Append(IDM_NODE_ADD, _("Add Node"), _("Add a node to the graph"));
	toolGraph->Append(IDM_NODE_DELETE, _("Delete Node"), _("Delete a node from the graph"));
	toolGraph->AppendSeparator();
	toolGraph->Append(IDM_ROAD_SELECT, _("Select Road"), _("Select an road from the graph"));
	toolGraph->Append(IDM_ROAD_ADD, _("Add Road"), _("Add an road to the graph"));
	toolGraph->Append(IDM_ROAD_DELETE, _("Delete Road"), _("Delete an road from the graph"));
	toolGraph->AppendSeparator();
	toolGraph->Append(IDM_VIEW_CELL, _("Select Cell"), _("Select a cell from the graph"));

	// help menu
	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_HELP, _("&About...\tF1"), _("About this program."));

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, _("&File"));
	menuBar->Append(menuView, _("&View"));
	menuBar->Append(menuTemplate, _("&Template"));
	menuBar->Append(toolGraph, _("&Tool"));
	menuBar->Append(menuHelp, _("&Help"));
	SetMenuBar(menuBar);

	
	// TOOLBARS

	// create the file toolbar
	_fileToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
    _fileToolBar->AddTool(wxID_NEW, _("New"), TOOL_BMP(newdoc), _("New file"));
    _fileToolBar->AddTool(wxID_OPEN, _("Open"), TOOL_BMP(open), _("Open file"));
    _fileToolBar->AddTool(wxID_SAVE, _("Save"), TOOL_BMP(save), _("Save file"), wxITEM_CHECK);
    _fileToolBar->AddSeparator();
    _fileToolBar->AddTool(wxID_HELP, _("Help"), TOOL_BMP(help), _("Help button"), wxITEM_CHECK);
	_fileToolBar->Realize();
	_frameManager.AddPane(_fileToolBar,  wxAuiPaneInfo().Name(_("FileTb")).
		Caption(_("File ToolBar")).ToolbarPane().Top().Row(0).Position(0).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	// create a view mode toolbar
	_viewModeToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	//_viewModeToolBar->AddTool(IDM_VIEW_BUILDING, _("View Buildings"), TOOL_BMP(building), _("View detailed buildings"), wxITEM_RADIO);
	_viewModeToolBar->AddTool(IDM_VIEW_BOX, _("View All"), TOOL_BMP(all), _("View buildings and all roads"), wxITEM_RADIO);
	_viewModeToolBar->AddTool(IDM_VIEW_CELL, _("View Roads"), TOOL_BMP(roads), _("View all roads"), wxITEM_RADIO);
	_viewModeToolBar->AddTool(IDM_VIEW_PRIMARY, _("View Primary"), TOOL_BMP(graph), _("View primary roads only"), wxITEM_RADIO);
	_viewModeToolBar->Realize();
	_viewModeToolBar->Enable(false);

	_frameManager.AddPane(_viewModeToolBar,  wxAuiPaneInfo().Name(_("ViewTb")).
		Caption(_("View Mode ToolBar")).ToolbarPane().Top().Row(0).Position(1).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	// create an edit mode toolbar
	_toolsetModeToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	_toolsetModeToolBar->AddTool(IDM_TOOLSET_VIEW, _("View"), TOOL_BMP(view), _("Select view only mode"), wxITEM_RADIO);
	_toolsetModeToolBar->AddTool(IDM_TOOLSET_NODE, _("Node Edit"), TOOL_BMP(node), _("Select node edit mode"), wxITEM_RADIO);
	_toolsetModeToolBar->AddTool(IDM_TOOLSET_EDGE, _("Road Edit"), TOOL_BMP(edge), _("Select road edit mode"), wxITEM_RADIO);
	_toolsetModeToolBar->AddTool(IDM_TOOLSET_CELL, _("Cell Edit"), TOOL_BMP(cell), _("Select cell edit mode"), wxITEM_RADIO);
	_toolsetModeToolBar->Realize();
	_toolsetModeToolBar->Enable(false);

	_frameManager.AddPane(_toolsetModeToolBar,  wxAuiPaneInfo().Name(_("ToolsetModeTb")).
		Caption(_("Edit Mode ToolBar")).ToolbarPane().Top().Row(0).Position(1).LeftDockable(false).
		RightDockable(false).Resizable(false)); 

	_nodeEditToolBar = 0;
	_roadEditToolBar = 0;
	_toolsetMode = view;

	// WINDOW PANES

	// world frame
	_worldFrame = new WorldFrame(this);
	_frameManager.AddPane(_worldFrame, wxCENTER, _("Ogre Pane"));

	// property manager
	_propertyGridManager = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxSize(260,200));
	_frameManager.AddPane(_propertyGridManager, wxRIGHT, wxT("Property Inspector"));

	// create pages
	_viewPropertyPage = new ViewPropertyPage(_worldFrame);
	_nodePropertyPage =  new NodePropertyPage(_worldFrame);
	_roadPropertyPage = new RoadPropertyPage(_worldFrame);
	_cellPropertyPage = new CellPropertyPage(_worldFrame);

	// Add pages to property inspector
	_propertyGridManager->AddPage(wxT("View Properties"), wxNullBitmap, _viewPropertyPage);
	_propertyGridManager->AddPage(wxT("Node Properties"), wxNullBitmap, _nodePropertyPage);
	_propertyGridManager->AddPage(wxT("Road Properties"), wxNullBitmap, _roadPropertyPage);
	_propertyGridManager->AddPage(wxT("Cell Properties"), wxNullBitmap, _cellPropertyPage);

	// log frame
	_logFrame = new LogFrame(this, wxSize(1000, 100));
	_frameManager.AddPane(_logFrame, wxBOTTOM, _("Log"));

	// tell the manager to "commit" all the changes just made
	_frameManager.Update();

	// set the frame icon
	SetIcon(wxICON(mondrian));

	// init doc stuff
	modify(false);
}

void MainWindow::init()
{
	_worldFrame->init();
}

MainWindow::~MainWindow()
{
	// deinitialize the frame manager
	_frameManager.UnInit();
	delete _worldFrame;
}

void MainWindow::updateOgre()
{
	_worldFrame->update();
}

void MainWindow::onNew(wxCommandEvent &e)
{
	if(!onSaveModified())
        return;

	setFilename(_(""));
	_worldFrame->onNewDoc();
	_viewModeToolBar->Enable(true);
	_toolsetModeToolBar->Enable(true);
	modify(true);
}

void MainWindow::onOpen(wxCommandEvent &e)
{
	if(!onSaveModified())
        return;

	wxString filename = wxFileSelector(_("Choose a file to open"), _(""), _(""),  
		_(""), _("Citygen XML files (*.cgx)|*.cgx|GIF files (*.gif)|*.gif"));

	doOpen(filename);
}

void MainWindow::doOpen(const wxString& filename)
{
	if(!filename.empty())
	{
		// TODO
		TiXmlDocument doc;
		if(doc.LoadFile(_C(filename)))
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
			_worldFrame->loadXML(worldRoot);

			//
			setFilename(filename);
			_savedYet = true;
			modify(false);
			_viewModeToolBar->Enable(true);
			_toolsetModeToolBar->Enable(true);
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

	_viewModeToolBar->Enable(false);
	_toolsetModeToolBar->Enable(false);
	_worldFrame->onCloseDoc();
}

void MainWindow::onExport(wxCommandEvent &e)
{
	//TODO: file save as
	wxString filename = wxFileSelector(_("Choose a file to export"), _(""), _(""),  
		_(""), _("COLLADA files (*.dae)|*.dae"), wxSAVE);
	if(!filename.empty())
	{
		doExport(filename); //return
	}
	//return false;
}

void MainWindow::onSave(wxCommandEvent &e)
{
	save();
	_fileToolBar->ToggleTool(wxID_SAVE, false);
}

void MainWindow::onSaveAs(wxCommandEvent &e)
{
	saveAs();
	_fileToolBar->ToggleTool(wxID_SAVE, false);
}

bool MainWindow::save()
{
    if (!isModified() && _savedYet)
        return true;

    if (_docFile.empty() || !_savedYet)
        return saveAs();

    return doSave(_docFile);
}

bool MainWindow::saveAs()
{
	//TODO: file save as !!!CAUSES a memory leak; is within wxFileSelector
	wxString filename = wxFileSelector(_("Choose a file to save"), _(""), _(""),  
		_(""), _("Citygen XML files (*.cgx)|*.cgx|GIF files (*.gif)|*.gif"), wxSAVE);
	if(!filename.empty())
	{
		return doSave(filename);
	}
	return false;
}

bool MainWindow::doExport(const wxString &file)
{
	//FCDocument doc;  
	//_worldFrame->exportScene(doc);

	// Save and load this document.
	//if(!FCollada::SaveDocument(&doc, file))
	std::string fileStr(_C(file));
#ifdef WIN32
	replace(fileStr.begin(), fileStr.end(), '\\', '/');
#endif
	ColladaDoc doc(fileStr);
	_worldFrame->exportScene(doc);
	if(!doc.save())
	{
		(void)wxMessageBox(_("Sorry, could not save this file."), _("File error"),
			wxOK | wxICON_EXCLAMATION, this);
		return false;
	}
	return true;
}

bool MainWindow::doSave(const wxString &file)
{
	//TODO
	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "yes");  
	doc.LinkEndChild(decl);  
	doc.LinkEndChild(_worldFrame->saveXML());
 
	if(doc.SaveFile(file.mb_str(wxCSConv(wxLocale::GetSystemEncoding()))))
	{
		setFilename(file);
		modify(false);
		_savedYet = true;
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
        wxString title(wxFileNameFromPath(_docFile));

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
	_docFile = file;

	if(file.empty())
		SetTitle(wxTheApp->GetAppName());
	else
		SetTitle(wxFileNameFromPath(_docFile) + _(" - ") + wxTheApp->GetAppName());
}

void MainWindow::onSelectViewPrimary(wxCommandEvent &e)
{
	_worldFrame->setViewMode(view_primary);
}

void MainWindow::onSelectViewCell(wxCommandEvent &e)
{
	_worldFrame->setViewMode(view_cell);
}

void MainWindow::onSelectViewBox(wxCommandEvent &e)
{
	_worldFrame->setViewMode(view_box);
}

void MainWindow::onSelectViewBuilding(wxCommandEvent &e)
{
	_worldFrame->setViewMode(view_building);
}

void MainWindow::onSelectToolsetView(wxCommandEvent &e)
{
	_toolsetMode = view;
	onChangeToolsetMode();
}

void MainWindow::onSelectToolsetNode(wxCommandEvent &e)
{
	_toolsetMode = node;
	onChangeToolsetMode();
}

void MainWindow::onSelectToolsetRoad(wxCommandEvent &e)
{
	_toolsetMode = road;
	onChangeToolsetMode();
}

void MainWindow::onSelectToolsetCell(wxCommandEvent &e)
{
	_toolsetMode = cell;
	onChangeToolsetMode();
}

void MainWindow::onChangeToolsetMode()
{
	// change world frame edit mode
	_worldFrame->setToolsetMode(_toolsetMode);

	// create toolbars if not created and detach if created
	_frameManager.DetachPane(_fileToolBar);
	_frameManager.DetachPane(_viewModeToolBar);
	_frameManager.DetachPane(_toolsetModeToolBar);

	// attach required toolbars
	wxAuiPaneInfo toolbarSettings = wxAuiPaneInfo().ToolbarPane().Top().Row(0).
					LeftDockable(false).RightDockable(false).Resizable(false);
	_frameManager.AddPane(_fileToolBar,  toolbarSettings.Name(_("FileToolBar")).
							Caption(_("File ToolBar")).Position(0)); 
	_frameManager.AddPane(_viewModeToolBar, toolbarSettings.Name(_("ViewModeTb")).
							Caption(_("View Mode ToolBar")).Position(1)); 
	_frameManager.AddPane(_toolsetModeToolBar, toolbarSettings.Name(_("ToolsetModeTb")).
							Caption(_("Edit Mode ToolBar")).Position(2));

	// destroy all special mode toolbars
	if(_nodeEditToolBar)
	{
		_frameManager.DetachPane(_nodeEditToolBar);
		delete _nodeEditToolBar;
		_nodeEditToolBar = 0;
	}
	if(_roadEditToolBar)
	{
		_frameManager.DetachPane(_roadEditToolBar);
		delete _roadEditToolBar;
		_roadEditToolBar = 0;
	}

	// create the right edit mode toolbar
	switch(_toolsetMode)
	{
	case view:
		// no custom view toolbar
		_worldFrame->setActiveTool(viewTool);
		break;
	case node:
		initNodeEdit();
		_worldFrame->setActiveTool(selNode);
		break;
	case road:
		initRoadEdit();
		_worldFrame->setActiveTool(selRoad);
		break;
	case cell:
		_worldFrame->setActiveTool(selCell);
		break;
	}

	_propertyGridManager->SelectPage(_toolsetMode);


	// update frame manager
	_frameManager.Update();
}

void MainWindow::initNodeEdit()
{
	// create a toolbar and add them to it
	_nodeEditToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	_nodeEditToolBar->AddTool(IDM_NODE_SELECT, _("Select Node"), TOOL_BMP(selnode), _("Select node"), wxITEM_RADIO);
	_nodeEditToolBar->AddTool(IDM_NODE_ADD, _("Add Node"), TOOL_BMP(addnode), _("Add node"), wxITEM_RADIO);
	_nodeEditToolBar->AddTool(IDM_NODE_DELETE, _("Delete Node"), TOOL_BMP(delnode), _("Delete node"), wxITEM_RADIO);
	_nodeEditToolBar->Realize();

	_frameManager.AddPane(_nodeEditToolBar,  wxAuiPaneInfo().Name(_("NodeEditTb")).
		Caption(_("Node Edit ToolBar")).ToolbarPane().Top().Row(0).Position(3).LeftDockable(false).
		RightDockable(false).Resizable(false)); 
}

void MainWindow::initRoadEdit()
{
	// create a toolbar and add them to it
	_roadEditToolBar = new wxToolBar(this, wxNewId(), wxDefaultPosition, wxDefaultSize, TOOLBAR_STYLE);
	_roadEditToolBar->AddTool(IDM_ROAD_SELECT, _("Select Road"), TOOL_BMP(selnode), _("Select road"), wxITEM_RADIO);
	_roadEditToolBar->AddTool(IDM_ROAD_ADD, _("Add Road"), TOOL_BMP(addedge), _("Add road"), wxITEM_RADIO);
	_roadEditToolBar->AddTool(IDM_ROAD_DELETE, _("Delete Road"), TOOL_BMP(deledge), _("Delete road"), wxITEM_RADIO);
	_roadEditToolBar->Realize();

	_frameManager.AddPane(_roadEditToolBar,  wxAuiPaneInfo().Name(_("RoadEditTb")).
		Caption(_("Road Edit ToolBar")).ToolbarPane().Top().Row(0).Position(4).LeftDockable(false).
		RightDockable(false).Resizable(false));
}

void MainWindow::onSelectNode(wxCommandEvent &e)
{
	_worldFrame->setActiveTool(selNode);
}

void MainWindow::onSelectNodeAdd(wxCommandEvent &e)
{
	_worldFrame->setActiveTool(addNode);
}

void MainWindow::onSelectNodeDel(wxCommandEvent &e)
{
	_worldFrame->setActiveTool(delNode);
}

void MainWindow::onSelectRoad(wxCommandEvent &e)
{
	_worldFrame->setActiveTool(selRoad);
}

void MainWindow::onSelectRoadAdd(wxCommandEvent &e)
{
	_worldFrame->setActiveTool(addRoad);
}

void MainWindow::onSelectRoadDel(wxCommandEvent &e)
{
	_worldFrame->setActiveTool(delRoad);
}


void MainWindow::updateProperties()
{
	switch(_toolsetMode)
	{
	case view:
		_viewPropertyPage->update();
		break;
	case node:
		_nodePropertyPage->update();
		break;
	case road:
		_roadPropertyPage->update();
		break;
	case cell:
		_cellPropertyPage->update();
		break;
	}
}

void MainWindow::modify(bool m) 
{ 
	_modified = m;
	_fileToolBar->EnableTool(wxID_SAVE, m);
}

bool MainWindow::isModified() 
{ 
	return _modified;
}
