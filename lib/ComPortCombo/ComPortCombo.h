#pragma once

#if !defined(__ATLCTRLS_H__)
    #error ComPortCombo.h requires atlctrls.h to be included first
    // You need to have the following WTL headers included to use this control.
    //#include <atlstr.h>
    //#include <atlbase.h>
    //#include <atlapp.h>
    //#include <atlctrls.h>
#endif

#include "EnumDevices.h"

#include <string>

class CEnumDevInfo;

class CComPortCombo : public CComboBox
{
public:
    CComPortCombo()
        : m_bNoneItem(true)           // First item is "<None>"
        , m_bOnlyPhysical(false)      // Include virtual COM ports
        , m_bOnlyPresent(true)        // Only present ports
        , m_strNone(_T("<None>"))
    { }

    // Forward assignment to CComboBox.
    template <typename T>
    CComPortCombo& operator =(T val)
    {
        CComboBox::operator=(val);
        return *this;
    }

    // Fill the combo box.
    bool InitList(int nDefPort = -1)
    {
        // Should be drop-down list without edit field.
        // CBS_DROPDOWNLIST = 3 while CBS_SIMPLE = 1 and CBS_DROPDOWN = 2
        ATLASSERT((GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST);

        m_nDefPort = nDefPort;							// use member var for access by call back function
        if (m_nDefPort < 0)								// when no default port specified
            m_nDefPort = GetPortNum();					//  use the currently selected one upon re-loading
        ResetContent();
        if (m_bNoneItem)
        {
            int nItem = AddString(m_strNone);
            SetItemData(nItem, 0);
        }

        auto bResult = CEnumDevices::EnumSerialPorts(
            [this](CEnumDevInfo const& pInfo) { this->AddItem(pInfo); },
            m_bOnlyPresent);

        // If no port pre-selected and none item used or only one port present, select first item.
        if (bResult && GetCurSel() < 0 && (m_bNoneItem || GetCount() == 1))
            SetCurSel(0);
        return bResult;
    }

    bool InitList(CString lpszPort)
    {
        return InitList(CEnumDevices::GetPortFromName(lpszPort));
    }

    bool IsPhysicalPort() const { return GetCurData() && !(GetCurData() & DATA_VIRTUAL_MASK); }
    bool IsVirtualPort() const { return (GetCurData() & DATA_VIRTUAL_MASK) != 0; }
    int GetPortNum() const { return GetCurData() & DATA_PORT_MASK; }

    bool GetFileName(CString& str) const
    {
        std::wstring s;
        bool const res = CEnumDevices::GetFileName(GetPortNum(), s);
        if (res)
            str = s.c_str();
        return res;
    }

    void ShowNoneItem(bool bSet = true) { m_bNoneItem = bSet; }
    void ShowOnlyPhysical(bool bSet = true) { m_bOnlyPhysical = bSet; }
    void ShowOnlyPresent(bool bSet = true) { m_bOnlyPresent = bSet; }
    void NoneStr(CString s) { m_strNone = s; }

private:
    void AddItem(CEnumDevInfo const& pInfo)
    {
        if (pInfo.m_nPortNum > 0 &&
            (!m_bOnlyPhysical || !(pInfo.m_nPortNum & DATA_VIRTUAL_MASK)))
        {
            int nItem = AddString(pInfo.m_strName.c_str());
            SetItemData(nItem, static_cast<DWORD>(pInfo.m_nPortNum));
            if ((pInfo.m_nPortNum & DATA_PORT_MASK) == m_nDefPort)
                SetCurSel(nItem);
        }
    }

    // Get item data for current selection (port number with additional flags)
    int GetCurData() const
    {
        int const nSel = GetCurSel();
        return nSel >= 0
            ? static_cast<int>(GetItemData(nSel))
            : 0;
    }

    bool    m_bNoneItem;
    bool    m_bOnlyPhysical;
    bool    m_bOnlyPresent;
    int     m_nDefPort;
    CString m_strNone;
};
