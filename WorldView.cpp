// Includes 
#include "stdafx.h"
#include "WorldView.h"
#include "MainWindow.h"

// Namespace
using namespace Ogre;


//IMPLEMENT_CLASS0(WorldView)
IMPLEMENT_DYNAMIC_CLASS(WorldView, wxView)

// For drawing lines in a canvas
float xpos = -1;
float ypos = -1;

BEGIN_EVENT_TABLE(WorldView, wxView)
    //EVT_MENU(DOODLE_CUT, WorldView::OnCut)
END_EVENT_TABLE()

WorldView::WorldView()
{
	mFrame = (wxFrame *) NULL;
}

WorldView::~WorldView()
{
}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool WorldView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{   
    // Single-window mode
	mFrame = MainWindow::getSingletonPtr();
    canvas = MainWindow::getSingletonPtr()->mWorldCanvas;
    canvas->view = this;
	canvas->prepare();
	//canvas->docSynch();
	//canvas->Update();
	MainWindow::getSingletonPtr()->enableDocumentToolbars(true);
    
    // Associate the appropriate frame with this view.
    SetFrame(mFrame);
    
    // Make sure the document manager knows that this is the
    // current view.
    Activate(true);
    
    // Initialize the edit menu Undo and Redo items
    //doc->GetCommandProcessor()->SetEditMenu(((MyFrame *)frame)->editMenu);
    //doc->GetCommandProcessor()->Initialize();  
    
    return true;
}


// Sneakily gets used for default print/preview
// as well as drawing on the screen.
void WorldView::OnDraw(wxDC *dc)
{
}

void WorldView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
	if (canvas)
		canvas->loadDoc();
}

// Clean up windows used for displaying the view.
bool WorldView::OnClose(bool deleteWindow)
{
    if (!GetDocument()->Close())
        return false;

	MainWindow::getSingletonPtr()->enableDocumentToolbars(false);
    
    canvas->clear();
	canvas->Update();
    canvas->view = 0;
    canvas = 0;
    
    wxString s(wxTheApp->GetAppName());
    if (mFrame)
		mFrame->SetTitle(s);
    
    SetFrame((wxFrame *)0);
    
	Activate(false);
    
    return true;
}
/*
//void WorldView::OnCut(wxCommandEvent& WXUNUSED(event) )
//{
//    DrawingDocument *doc = (DrawingDocument *)GetDocument();
//    doc->GetCommandProcessor()->Submit(new DrawingCommand(_T("Cut Last Segment"), DOODLE_CUT, doc, (DoodleSegment *) NULL));
//}
*/
