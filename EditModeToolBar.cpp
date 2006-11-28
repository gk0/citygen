// Includes 
#include "stdafx.h"
#include "EditModeToolBar.h"


const int IDM_GRAPH_VIEW = wxNewId();
const int IDM_GRAPH_NODE = wxNewId();
const int IDM_GRAPH_EDGE = wxNewId();
const int IDM_GRAPH_CELL = wxNewId();

//Bitmaps
#ifdef USE_XPM_BITMAPS
    #include "bitmaps/view.xpm"
    #include "bitmaps/node.xpm"
    #include "bitmaps/edge.xpm"
    #include "bitmaps/cell.xpm"
#endif // USE_XPM_BITMAPS

// Event Table //
BEGIN_EVENT_TABLE(EditModeToolBar, wxToolBar)
	EVT_MENU(IDM_GRAPH_VIEW, EditModeToolBar::onSelectViewMode)
	EVT_MENU(IDM_GRAPH_NODE, EditModeToolBar::onSelectNodeMode)
	EVT_MENU(IDM_GRAPH_EDGE, EditModeToolBar::onSelectEdgeMode)
	EVT_MENU(IDM_GRAPH_CELL, EditModeToolBar::onSelectCellMode)
END_EVENT_TABLE()


EditModeToolBar::EditModeToolBar(wxWindow* parent, wxWindowID id, long style, bool largeToolbar)
 : wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style),
   m_EditModeListener(0)
{
	// Set up toolbar
	enum
	{
		Tool_view,
		Tool_node,
		Tool_edge,
		Tool_cell,
		Tool_max
	};
	wxBitmap toolBarBitmaps[Tool_max];

	#ifdef USE_XPM_BITMAPS
		#define INIT_TOOL_BMP(bmp) \
			toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp##_xpm)
	#else // !USE_XPM_BITMAPS
		#define INIT_TOOL_BMP(bmp) \
			toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)
	#endif // USE_XPM_BITMAPS/!USE_XPM_BITMAPS

	INIT_TOOL_BMP(view);
	INIT_TOOL_BMP(node);
	INIT_TOOL_BMP(edge);
	INIT_TOOL_BMP(cell);

	int w = toolBarBitmaps[Tool_view].GetWidth(),
		h = toolBarBitmaps[Tool_view].GetHeight();

	if ( largeToolbar )
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
	AddTool(IDM_GRAPH_VIEW, _T("View"), toolBarBitmaps[Tool_view], _T("Select view only mode"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_NODE, _T("Node Edit"), toolBarBitmaps[Tool_node], _T("Select node edit mode"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_EDGE, _T("Road Edit"), toolBarBitmaps[Tool_edge], _T("Select road edit mode"), wxITEM_RADIO);
	AddTool(IDM_GRAPH_CELL, _T("Cell Edit"), toolBarBitmaps[Tool_cell], _T("Select cell edit mode"), wxITEM_RADIO);


	// after adding the buttons to the toolbar, must call Realize() to reflect the changes
	Realize();
}

void EditModeToolBar::onSelectViewMode(wxCommandEvent &e) {
	if(m_EditModeListener) 
		m_EditModeListener->setEditMode(EditModeListener::view);
}

void EditModeToolBar::onSelectNodeMode(wxCommandEvent &e) {
	if(m_EditModeListener) 
		m_EditModeListener->setEditMode(EditModeListener::node);
}

void EditModeToolBar::onSelectEdgeMode(wxCommandEvent &e) {
	if(m_EditModeListener) 
		m_EditModeListener->setEditMode(EditModeListener::edge);
}

void EditModeToolBar::onSelectCellMode(wxCommandEvent &e) {
	if(m_EditModeListener) 
		m_EditModeListener->setEditMode(EditModeListener::cell);
}

void EditModeToolBar::setListener(EditModeListener* listener) {
	m_EditModeListener = listener;
}
