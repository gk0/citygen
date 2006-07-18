// Includes 
#include "stdafx.h"
#include "WorldView.h"
#include "MainWindow.h"

// Namespace
using namespace Ogre;


IMPLEMENT_DYNAMIC_CLASS(WorldView, wxView)

// For drawing lines in a canvas
float xpos = -1;
float ypos = -1;

BEGIN_EVENT_TABLE(WorldView, wxView)
    //EVT_MENU(DOODLE_CUT, WorldView::OnCut)
END_EVENT_TABLE()

WorldView::~WorldView()
{
	int i = 0;
}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool WorldView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{   
    // Single-window mode
	frame = MainWindow::getSingletonPtr();
    canvas = MainWindow::getSingletonPtr()->mWorldWindow;
    canvas->view = this;
	canvas->prepare();
	canvas->Update();
	MainWindow::getSingletonPtr()->enableDocumentToolbars(true);
    
    // Associate the appropriate frame with this view.
    SetFrame(frame);
    
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
    /*dc->SetFont(*wxNORMAL_FONT);
    dc->SetPen(*wxBLACK_PEN);
    
    wxList::compatibility_iterator node = ((DrawingDocument *)GetDocument())->GetDoodleSegments().GetFirst();
    while (node)
    {
        DoodleSegment *seg = (DoodleSegment *)node->GetData();
        seg->Draw(dc);
        node = node->GetNext();
    }*/
}

void WorldView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
	if (canvas) {
		//clear it and sync it with doc
        canvas->Refresh();
	}
}

// Clean up windows used for displaying the view.
bool WorldView::OnClose(bool deleteWindow)
{
    if (!GetDocument()->Close())
        return false;

	MainWindow::getSingletonPtr()->enableDocumentToolbars(false);
    
    //Lod help me i can't get it to draw a background like it should.
    canvas->clear();
	//canvas->ClearBackground();
	//canvas->Refresh();
	canvas->Update();
    canvas->view = 0;
    canvas = 0;
    
    wxString s(wxTheApp->GetAppName());
    if (frame)
        frame->SetTitle(s);
    
    SetFrame((wxFrame *)0);
    
    Activate(false);
    
    return true;
}

//void WorldView::OnCut(wxCommandEvent& WXUNUSED(event) )
//{
//    DrawingDocument *doc = (DrawingDocument *)GetDocument();
//    doc->GetCommandProcessor()->Submit(new DrawingCommand(_T("Cut Last Segment"), DOODLE_CUT, doc, (DoodleSegment *) NULL));
//}
