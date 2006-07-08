// Includes 
#include "stdafx.h"
#include "SelectModeToolBar.h"

const int IDM_GRAPH_SELNODE = wxNewId();
const int IDM_GRAPH_ADDNODE = wxNewId();
const int IDM_GRAPH_DELNODE = wxNewId();
const int IDM_GRAPH_SELEDGE = wxNewId();
const int IDM_GRAPH_ADDEDGE = wxNewId();
const int IDM_GRAPH_DELEDGE = wxNewId();


// Event Table //
BEGIN_EVENT_TABLE(SelectModeToolBar, wxToolBar)
	EVT_MENU(IDM_GRAPH_SELNODE, SelectModeToolBar::onSelectSelMode)
	EVT_MENU(IDM_GRAPH_ADDNODE, SelectModeToolBar::onSelectAddMode)
	EVT_MENU(IDM_GRAPH_DELNODE, SelectModeToolBar::onSelectDelMode)
	EVT_MENU(IDM_GRAPH_SELEDGE, SelectModeToolBar::onSelectSelMode)
	EVT_MENU(IDM_GRAPH_ADDEDGE, SelectModeToolBar::onSelectAddMode)
	EVT_MENU(IDM_GRAPH_DELEDGE, SelectModeToolBar::onSelectDelMode)
END_EVENT_TABLE()


SelectModeToolBar::SelectModeToolBar(wxWindow* parent, wxWindowID id, long style, bool largeToolbar)
 : wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style),
   mSelectModeListener(0)
{
	mLargeToolbar = largeToolbar;

	init();
	setEditMode(view);
}

void SelectModeToolBar::setEditMode(EditModeListener::EditMode mode) 
{
	//Remove What we don't want
	//if(mode != view) {
	//	if(FindById(IDM_GRAPH_SELEDGE)) RemoveTool(IDM_GRAPH_SELEDGE);
	//	if(FindById(IDM_GRAPH_ADDEDGE)) RemoveTool(IDM_GRAPH_ADDEDGE);
	//	if(FindById(IDM_GRAPH_DELEDGE)) RemoveTool(IDM_GRAPH_DELEDGE);
	//	if(FindById(IDM_GRAPH_SELNODE)) RemoveTool(IDM_GRAPH_SELNODE);
	//	if(FindById(IDM_GRAPH_ADDNODE)) RemoveTool(IDM_GRAPH_ADDNODE);
	//	if(FindById(IDM_GRAPH_DELNODE)) RemoveTool(IDM_GRAPH_DELNODE);
	//	init(mode);
	//}else{
	//	if(FindById(IDM_GRAPH_SELEDGE)) EnableTool(IDM_GRAPH_SELEDGE, false);
	//	if(FindById(IDM_GRAPH_ADDEDGE)) EnableTool(IDM_GRAPH_ADDEDGE, false);
	//	if(FindById(IDM_GRAPH_DELEDGE)) EnableTool(IDM_GRAPH_DELEDGE, false);
	//	if(FindById(IDM_GRAPH_SELNODE)) EnableTool(IDM_GRAPH_SELNODE, false);
	//	if(FindById(IDM_GRAPH_ADDNODE)) EnableTool(IDM_GRAPH_ADDNODE, false);
	//	if(FindById(IDM_GRAPH_DELNODE)) EnableTool(IDM_GRAPH_DELNODE, false);
	//}
	bool viewNode = (mode == node);
	if(FindById(IDM_GRAPH_SELNODE)) EnableTool(IDM_GRAPH_SELNODE, viewNode);
	if(FindById(IDM_GRAPH_ADDNODE)) EnableTool(IDM_GRAPH_ADDNODE, viewNode);
	if(FindById(IDM_GRAPH_DELNODE)) EnableTool(IDM_GRAPH_DELNODE, viewNode);

	bool viewEdge = (mode == edge);
	if(FindById(IDM_GRAPH_SELEDGE)) EnableTool(IDM_GRAPH_SELEDGE, viewEdge);
	if(FindById(IDM_GRAPH_ADDEDGE)) EnableTool(IDM_GRAPH_ADDEDGE, viewEdge);
	if(FindById(IDM_GRAPH_DELEDGE)) EnableTool(IDM_GRAPH_DELEDGE, viewEdge);
}


void SelectModeToolBar::init()
{
	// Set up toolbar
	enum
	{
		Tool_select,
		Tool_addnode,
		Tool_delnode,
		Tool_addedge,
		Tool_deledge,
		Tool_max
	};
	wxBitmap toolBarBitmaps[Tool_max];

	#if USE_XPM_BITMAPS
		#define INIT_TOOL_BMP(bmp) \
			toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp##_xpm)
	#else // !USE_XPM_BITMAPS
		#define INIT_TOOL_BMP(bmp) \
			toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)
	#endif // USE_XPM_BITMAPS/!USE_XPM_BITMAPS

	INIT_TOOL_BMP(select);
	INIT_TOOL_BMP(addnode);
	INIT_TOOL_BMP(delnode);
	INIT_TOOL_BMP(addedge);
	INIT_TOOL_BMP(deledge);

	int w = toolBarBitmaps[0].GetWidth(),
		h = toolBarBitmaps[0].GetHeight();

	if ( mLargeToolbar )
	{
		w *= 2;
		h *= 2;

		for ( size_t n = 0; n < WXSIZEOF(toolBarBitmaps); n++ )
		{
			toolBarBitmaps[n] =
				wxBitmap(toolBarBitmaps[n].ConvertToImage().Scale(w, h));
		}
		SetToolBitmapSize(wxSize(w, h));
	}
	//if(mode == node) {
	//	AddTool(IDM_GRAPH_SELNODE, _T("Select Node"), toolBarBitmaps[Tool_select], _T("Select node"), wxITEM_RADIO);
	//	AddTool(IDM_GRAPH_ADDNODE, _T("Add Node"), toolBarBitmaps[Tool_addnode], _T("Add node"), wxITEM_RADIO);
	//	AddTool(IDM_GRAPH_DELNODE, _T("Delete Node"), toolBarBitmaps[Tool_delnode], _T("Delete node"), wxITEM_RADIO);
	//}
	//if(mode == edge) {
	//	AddTool(IDM_GRAPH_SELEDGE, _T("Select Road"), toolBarBitmaps[Tool_select], _T("Select road"), wxITEM_RADIO);
	//	AddTool(IDM_GRAPH_ADDEDGE, _T("Add Road"), toolBarBitmaps[Tool_deledge], _T("Add road"), wxITEM_RADIO);
	//	AddTool(IDM_GRAPH_DELEDGE, _T("Delete Road"), toolBarBitmaps[Tool_deledge], _T("Delete road"), wxITEM_RADIO);
	//}
	AddTool(IDM_GRAPH_SELNODE, _T("Select Node"), toolBarBitmaps[Tool_select], _T("Select node"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_ADDNODE, _T("Add Node"), toolBarBitmaps[Tool_addnode], _T("Add node"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_DELNODE, _T("Delete Node"), toolBarBitmaps[Tool_delnode], _T("Delete node"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_SELEDGE, _T("Select Road"), toolBarBitmaps[Tool_select], _T("Select road"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_ADDEDGE, _T("Add Road"), toolBarBitmaps[Tool_deledge], _T("Add road"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_DELEDGE, _T("Delete Road"), toolBarBitmaps[Tool_deledge], _T("Delete road"), wxITEM_RADIO);
	Realize();
}


void SelectModeToolBar::setListener(SelectModeListener* listener) {
	mSelectModeListener = listener;
}

void SelectModeToolBar::onSelectSelMode(wxCommandEvent &e) {
	if(mSelectModeListener) 
		mSelectModeListener->setSelectMode(SelectModeListener::sel);
}
void SelectModeToolBar::onSelectAddMode(wxCommandEvent &e) {
	if(mSelectModeListener) 
		mSelectModeListener->setSelectMode(SelectModeListener::add);
}
void SelectModeToolBar::onSelectDelMode(wxCommandEvent &e) {
	if(mSelectModeListener) 
		mSelectModeListener->setSelectMode(SelectModeListener::del);
}