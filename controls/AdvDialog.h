/*
 * File:      AdvDialog.h
 * Id:        $Id$
 * Created:   26-01-2006
 *
 * Copyright(C)  Zbigniew Zagorski 2006. All rights reserved.
 * 
 * This program is distributed as free software under the
 * license in the file "LICENSE", which is included in the distribution.
 */

#ifndef __advdialog_h__
#define __advdialog_h__

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/statbmp.h>

class AdvDialogHeader: public wxPanel {
public:
	enum HintState {
		HINT_NONE,
		HINT_INFO,
		HINT_WARNING,
		HINT_ERROR
	};
	AdvDialogHeader();

	AdvDialogHeader(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("advDialogBox"));

	bool Create(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("advDialogBox"));
	
        void SetImage(const wxBitmap& bmp);

	void SetHeader(const wxString& header);
	void SetSubHeader(const wxString& subHeader);

	void SetMessageState(enum HintState state, const wxString& title);	

	wxString GetMessageText() const;
	// TODO: add rest of getters and setters
protected:
	void InitControls();

private:
	wxStaticText*	ctlHeader;
	wxStaticText*	ctlSubHeader;
	wxStaticBitmap* ctlBitmap;

	wxStaticText*   ctlMessage;
	wxStaticBitmap* ctlMessageIcon;

	HintState       hintState;

public: // TODO: should be accessed by accessor methods
	wxBitmap		warningBitmap;
	wxBitmap		errorBitmap;
	wxBitmap		infoBitmap;

	DECLARE_DYNAMIC_CLASS(AdvDialogHeader);
	DECLARE_EVENT_TABLE();
};

class AdvDialog: public wxDialog {
public:
	AdvDialog();

	AdvDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("advDialogBox"));

	bool Create(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("advDialogBox"));

        void      ReplaceContentContainer(wxWindow* newWindow);
	wxWindow* GetContentContainer() const;
	//wxWindow* GetFooterContainer() const;
	wxWindow* GetHeaderContainer() const;

        void SetHeaderImage(const wxBitmap& bmp);
	void SetHeader(const wxString& header);
	void SetSubHeader(const wxString& subHeader);

	void SetMessageState(enum AdvDialogHeader::HintState state, const wxString& title);	

protected:

	void init();
private:
	void cleanup();

	wxWindow* contentWindow;

	AdvDialogHeader* headerPanel;

public:
	DECLARE_DYNAMIC_CLASS(AdvDialog);
	DECLARE_EVENT_TABLE();

};

#endif // __advdialog_h__

