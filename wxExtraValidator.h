#include <wx/valgen.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>

class IOnValueChangeByValidator
{
public:
    virtual void OnValueChange(wxGenericValidator *pV,wxWindowBase *pW) = 0;
};

class WXDLLEXPORT wxIntValidator: public wxGenericValidator
{
    DECLARE_CLASS(wxIntValidator)
public:
    wxIntValidator(int* val,IOnValueChangeByValidator 
*pIOnValueChangeByValidator = NULL)
        :wxGenericValidator(val),
        m_pIOnValueChangeByValidator(pIOnValueChangeByValidator)
    {
    }
    wxIntValidator(const wxIntValidator& copyFrom)
        :wxGenericValidator(copyFrom.m_pInt)
    {
        Copy(copyFrom);
    }   
    bool Copy(const wxIntValidator& val)
    {
        wxGenericValidator::Copy(val);
        m_pIOnValueChangeByValidator = val.m_pIOnValueChangeByValidator;
        return true;
    }
    virtual wxObject *Clone() const { return new wxIntValidator(*this); }

    // Called to transfer data to the variable
    virtual bool TransferFromWindow()
    {
        int Int = 0;
        if(m_pInt)
            Int = *m_pInt;
        bool bRes = wxGenericValidator::TransferFromWindow();
        if(bRes && m_pInt && m_pIOnValueChangeByValidator)
        {
            if(*m_pInt != Int)
                
m_pIOnValueChangeByValidator->OnValueChange(this,m_validatorWindow);
        }
        return bRes;
    }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *  WXUNUSED(parent))
    {
#if wxUSE_SPINCTRL && !defined(__WXMOTIF__)
        if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl)) )
        {
            wxSpinCtrl* pControl = (wxSpinCtrl*) m_validatorWindow;
            return pControl && 
IsValid(pControl->GetValue(),pControl->GetMin(),pControl->GetMax());
        } else
#endif
#if wxUSE_SPINBTN
        if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinButton)) )
        {
            wxSpinButton* pControl = (wxSpinButton*) m_validatorWindow;
            return pControl && IsValid(pControl->GetValue(), 
pControl->GetMin(),pControl->GetMax());
        } else
#endif
        {
        }
        return true;
    }
protected:
    bool IsValid(int val,int min,int max) const
    {
        if(val >= min && val <= max)
            return true;
        return false;
    }

    IOnValueChangeByValidator *m_pIOnValueChangeByValidator;
};

class WXDLLEXPORT wxBoolValidator: public wxGenericValidator
{
    DECLARE_CLASS(wxBoolValidator)
public:
    wxBoolValidator(bool* val,IOnValueChangeByValidator 
*pIOnValueChangeByValidator = NULL)
        :wxGenericValidator(val),
        m_pIOnValueChangeByValidator(pIOnValueChangeByValidator)
    {
    }
    wxBoolValidator(const wxBoolValidator& copyFrom)
        :wxGenericValidator(copyFrom.m_pBool)
    {
        Copy(copyFrom);
    }   
    bool Copy(const wxBoolValidator& val)
    {
        wxGenericValidator::Copy(val);
        m_pIOnValueChangeByValidator = val.m_pIOnValueChangeByValidator;
        return true;
    }
    virtual wxObject *Clone() const { return new wxBoolValidator(*this); }

    // Called to transfer data to the variable
    virtual bool TransferFromWindow()
    {
        bool Bool = 0;
        if(m_pBool)
            Bool = *m_pBool;
        bool bRes = wxGenericValidator::TransferFromWindow();
        if(bRes && m_pBool && m_pIOnValueChangeByValidator)
        {
            if(*m_pBool != Bool)
                
m_pIOnValueChangeByValidator->OnValueChange(this,m_validatorWindow);
        }
        return bRes;
    }

protected:
    IOnValueChangeByValidator *m_pIOnValueChangeByValidator;
};
