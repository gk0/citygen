// Includes 
#include "stdafx.h"
#include "FileToolBar.h"

//Bitmaps
#ifdef USE_XPM_BITMAPS
    #include "bitmaps/newdoc.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
//    #include "bitmaps/copy.xpm"
//    #include "bitmaps/cut.xpm"
//    #include "bitmaps/preview.xpm"  // paste XPM
//   #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS

// Event Table //
BEGIN_EVENT_TABLE(FileToolBar, wxToolBar)
END_EVENT_TABLE()


FileToolBar::FileToolBar(wxWindow* parent, wxWindowID id, long style, bool largeToolbar)
 : wxToolBar(parent, id, wxDefaultPosition, wxDefaultSize, style)
{
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

#ifdef USE_XPM_BITMAPS
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

    int w = toolBarBitmaps[Tool_newdoc].GetWidth();
    int h = toolBarBitmaps[Tool_newdoc].GetHeight();

    if (largeToolbar)
    {
        w *= 2;
        h *= 2;

        for ( size_t n = Tool_newdoc; n < WXSIZEOF(toolBarBitmaps); n++ ){
            toolBarBitmaps[n] =
                wxBitmap(toolBarBitmaps[n].ConvertToImage().Scale(w, h));
        }
        SetToolBitmapSize(wxSize(w, h));
    }

    AddTool(wxID_NEW, _T("New"), toolBarBitmaps[Tool_newdoc], _T("New file"));
    AddTool(wxID_OPEN, _T("Open"), toolBarBitmaps[Tool_open], _T("Open file"));
    AddTool(wxID_SAVE, _T("Save"), toolBarBitmaps[Tool_save], _T("Toggle button 1"), wxITEM_CHECK);
/*  AddTool(wxID_COPY, _T("Copy"), toolBarBitmaps[Tool_copy], _T("Toggle button 2"), wxITEM_CHECK);
    AddTool(wxID_CUT, _T("Cut"), toolBarBitmaps[Tool_cut], _T("Toggle/Untoggle help button"));
    AddTool(wxID_PASTE, _T("Paste"), toolBarBitmaps[Tool_paste], _T("Paste"));
	AddTool(wxID_PRINT, _T("Print"), toolBarBitmaps[Tool_print],
                         _T("Delete this tool. This is a very long tooltip to test whether it does the right thing when the tooltip is more than Windows can cope with."));
*/
    AddSeparator();
    AddTool(wxID_HELP, _T("Help"), toolBarBitmaps[Tool_help], _T("Help button"), wxITEM_CHECK);

    // after adding the buttons to the toolbar, must call Realize() to reflect the changes
    Realize();
}
