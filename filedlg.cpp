#include <wx/wx.h>
#include <wx/dataview.h>


class FileSystemTreeModel: public wxDataViewModel {
    //
    // wxDataViewModel contract
    //
    virtual unsigned int 	GetChildren (const wxDataViewItem &item, wxDataViewItemArray &children) const
    {
        return 0;
    }
    
    virtual unsigned int 	GetColumnCount () const
    {
        return 1;
    }
    
    virtual wxString 	GetColumnType (unsigned int col) const
    {
        return wxVariant(wxT("")).GetType();
    }
    
    virtual void 	GetValue (wxVariant &variant, const wxDataViewItem &item, unsigned int col) const
    {
        wxASSERT(col == 0);
        
        wxString itemFull = getItem(item);
        
        wxFileName itemFileName(itemFull);
        wxString result = itemFileName.GetPath();
        
        variant = wxVariant(result);
    } 

    virtual wxDataViewItem 	GetParent (const wxDataViewItem &item) const
    {
        std::string item_full = getItem(item);
        std::string item_dirname = tinfra::path::dirname(item_full);
        
        parent_id = searchOrCreateItem(item_dirname);
    }
    
    virtual bool 	IsContainer (const wxDataViewItem &item) const
    {
        std::string item_full = getItem(item);
        
        return fs.is_dir(item_full);
    }
    virtual bool 	SetValue (const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
    {
        return false;
    }
    
        // implementation details
    std::string getItem(const wxDataViewItem &item) const 
    {
        void* vid = item.GetId();
        int i = reinterpret_cast<int>(vid);
        
        wxASSERT(i < entries.size());
        wxASSERT(i > 0);
        
        return entries[i];
    }
private:
    std::vector<std::string> entries;
};

class FileSystemBrowser: public wxPanel {
public:
    FileSystemBrowser(wxWindow* parent):
        wxPanel(parent, -1)
    {
        initControls();
    }
    
private:
    
    void initControls()
    
    {
        
    }
};

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
