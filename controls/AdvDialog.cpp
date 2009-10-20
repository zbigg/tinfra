//
// File:      AdvDialog.cpp
// Id:        $Id$
// Created:   26-01-2006
//
// Copyright(C)  Zbigniew Zagorski 2006. All rights reserved.
// 
// This program is distributed as free software under the
// license in the file "LICENSE", which is included in the distribution.
//

#include <wx/wxprec.h>

#ifndef USE_PCH
#include "wx/textctrl.h"
#include "wx/dialog.h"
#include "wx/sizer.h"
#endif

#include "wx/statbmp.h"
#include "wx/stattext.h"
#include "wx/artprov.h"
#include "AdvDialog.h"

IMPLEMENT_DYNAMIC_CLASS(AdvDialogHeader, wxPanel);
BEGIN_EVENT_TABLE(AdvDialogHeader, wxPanel)
END_EVENT_TABLE();


AdvDialogHeader::AdvDialogHeader()
{
    hintState = HINT_NONE;
}

AdvDialogHeader::AdvDialogHeader(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    hintState = HINT_NONE;
    Create(parent,id,pos,size,style,name);
}

bool AdvDialogHeader::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    if( !wxPanel::Create(parent,id,pos,size,style,name) )
        return false;
    InitControls();
    return true;
}

void AdvDialogHeader::SetImage(const wxBitmap& bmp)
{
    ctlBitmap->SetBitmap(bmp);
}


void AdvDialogHeader::SetHeader(const wxString& header)
{
    ctlHeader->SetLabel(header);
}
void AdvDialogHeader::SetSubHeader(const wxString& subHeader)
{
    ctlSubHeader->SetLabel(subHeader);
}

void AdvDialogHeader::SetMessageState(enum HintState state, const wxString& title)
{
    bool changes = false;
    if( GetMessageText() != title ) {
        ctlMessage->SetLabel(title);
        changes = true;
    }
    if(hintState != state ) {
        wxBitmap resultBmp;
        wxColour fgColour = wxNullColour;
        switch( state ) {
        case HINT_NONE:
            resultBmp = wxNullBitmap;
            break;
        case HINT_INFO:
            resultBmp = infoBitmap;
            break;
        case HINT_WARNING:
            resultBmp = warningBitmap;
            break;
        case HINT_ERROR:
            resultBmp = errorBitmap;
            fgColour = *wxRED;
            break;
        }
        if( fgColour != ctlMessage->GetForegroundColour() ) 
            ctlMessage->SetForegroundColour(fgColour);
        ctlMessageIcon->SetBitmap(resultBmp);
        hintState = state;
        changes = true;
    }
    if( changes ) Layout();
}

wxString AdvDialogHeader::GetMessageText() const
{
    return ctlMessage->GetLabel();
}


static wxBitmap createColourMask(wxBitmap bmp, wxColour const& colour)
{
    wxMask* mask = new wxMask(bmp,colour);
    bmp.SetMask(mask);
    return bmp;
}

static wxBitmap createColourMask(wxBitmap bmp)
{
    wxColour const colour = wxColour(0,0,0);
    return createColourMask(bmp,colour);
}

void AdvDialogHeader::InitControls()
{
    SetBackgroundColour(*wxWHITE);
    
    wxBitmap x = createColourMask( wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_CMN_DIALOG));
    warningBitmap = createColourMask( wxArtProvider::GetBitmap(wxART_WARNING , wxART_MENU, wxSize(16,16)));
    errorBitmap   = createColourMask( wxArtProvider::GetBitmap(wxART_ERROR , wxART_MENU, wxSize(16,16)));
    infoBitmap   = createColourMask( wxArtProvider::GetBitmap(wxART_INFORMATION , wxART_MENU, wxSize(16,16)));
    
    ctlHeader = new wxStaticText(this,-1,wxEmptyString,wxDefaultPosition,wxDefaultSize, 0);
    ctlSubHeader = new wxStaticText(this,-1,wxEmptyString,wxDefaultPosition,wxDefaultSize, 0);
    ctlBitmap = new wxStaticBitmap(this,-1,x,wxDefaultPosition,wxDefaultSize, 0);
    ctlBitmap->SetForegroundColour(*wxWHITE);
    wxFont f = ctlHeader->GetFont();
    f.SetWeight(wxFONTWEIGHT_BOLD);
    ctlHeader->SetFont(f);
    
    ctlMessage = new wxStaticText(this,-1,wxEmptyString,wxDefaultPosition,wxDefaultSize, 0);	
    ctlMessageIcon = new wxStaticBitmap(this,-1,wxNullBitmap,wxDefaultPosition,wxDefaultSize, 0);
    ctlMessageIcon->SetSize(wxSize(16,16));
    ctlMessageIcon->SetForegroundColour(*wxWHITE);
    ctlMessage->SetLabel(wxEmptyString);
    ctlMessage->SetForegroundColour(*wxGREEN);
    
    wxSizer* sh = new wxBoxSizer(wxHORIZONTAL);
    wxSizer* sx = new wxBoxSizer(wxVERTICAL);
    wxSizer* sm = new wxBoxSizer(wxHORIZONTAL);
    
    sm->Add(ctlMessageIcon, 0, wxALL, 2);
    sm->Add(ctlMessage, 1, wxALL | wxEXPAND| wxALIGN_CENTER_VERTICAL, 3);
    
    sx->Add(ctlHeader, 1, wxBOTTOM | wxEXPAND, 0);
    sx->Add(ctlSubHeader, 1, wxLEFT | wxBOTTOM | wxEXPAND, 8);
    sx->Add(sm, 1, wxLEFT  | wxEXPAND, 8);
    
    sh->Add(sx, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT , 10);
    sh->Add(ctlBitmap, 0, wxEXPAND | wxALL, 8);
    
    SetSizer(sh);
    Layout();
}



///
///
///

IMPLEMENT_DYNAMIC_CLASS(AdvDialog, wxDialog);
BEGIN_EVENT_TABLE(AdvDialog, wxDialog)
END_EVENT_TABLE();

AdvDialog::AdvDialog()
{
    cleanup();
}

AdvDialog::AdvDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    cleanup();
    Create(parent,id,title,pos,size,style,name);
}

void AdvDialog::cleanup()
{
    
}

bool AdvDialog::Create(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    if( !wxDialog::Create(parent,id,title,pos,size, style == 0 ? wxDEFAULT_DIALOG_STYLE: style ,name) )
        return false;
    init();
    return true;
}

wxWindow* AdvDialog::GetContentContainer() const
{
    return contentWindow;
}

void AdvDialog::ReplaceContentContainer(wxWindow* newWindow)
{
    wxSizer* s = GetSizer();
    s->Remove(1);
    GetContentContainer()->Destroy();
    s->Insert(1, newWindow, 1, wxALL | wxEXPAND, 2);
    contentWindow = newWindow;
}

wxWindow* AdvDialog::GetHeaderContainer() const
{
    return headerPanel;
}

void AdvDialog::init()
{
    wxSizer* s = new wxBoxSizer(wxVERTICAL);
    
    contentWindow = new wxPanel(this);
    contentWindow->SetSize(wxSize(200,200));
    
    {
        wxSizer* sc = new wxBoxSizer(wxVERTICAL);
        contentWindow->SetSizer(sc);
    }
    
    headerPanel = new AdvDialogHeader(this,-1);
    s->Add(headerPanel, 0, wxEXPAND, 0);
    s->Add(contentWindow, 1, wxALL | wxEXPAND, 2);
    SetSizer(s);
}

void AdvDialog::SetHeader(const wxString& header)
{
    headerPanel->SetHeader(header);
}
void AdvDialog::SetSubHeader(const wxString& subHeader)
{
    headerPanel->SetSubHeader(subHeader);
}

void AdvDialog::SetMessageState(enum AdvDialogHeader::HintState state, const wxString& title)
{
    headerPanel->SetMessageState(state,title);
}

void AdvDialog::SetHeaderImage(const wxBitmap& bmp)
{
    headerPanel->SetImage(bmp);
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
