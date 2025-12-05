#pragma once

#include "pch.h"
#include "CSqliteManager.h"

class CIncomeEditDlg : public CDialogEx
{
public:
    CIncomeEditDlg(CWnd* pParent = nullptr); 
    virtual ~CIncomeEditDlg() = default;

    void SetValues(const CString& amount, const CString& source, const CString& note);
    void GetValues(CString& amount, CString& source, CString& note) const;

protected:
    virtual BOOL OnInitDialog() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;

    CString m_amount;
    CString m_source;
    CString m_note;

    DECLARE_MESSAGE_MAP()
};

class CIncomeDlg : public CDialogEx
{
public:
    CIncomeDlg(CWnd* pParent = nullptr);
    virtual ~CIncomeDlg() = default;

protected:
    virtual BOOL OnInitDialog() override;
    afx_msg void OnAdd();
    afx_msg void OnEdit();
    afx_msg void OnDelete();
    afx_msg void OnPrint();

    void LoadRecords();
    CString EscapeSql(const CString& input);

    CListCtrl m_listIncome;
    CSqliteManager m_db;

    DECLARE_MESSAGE_MAP()
};