#pragma once

#include "pch.h"
#include "CSqliteManager.h"

//#include <afxwin.h>
//#include <afxcmn.h>
//#include <afxdialogex.h>

class CExpenseEditDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CExpenseEditDlg)
public:
    CExpenseEditDlg(CWnd* pParent = nullptr);
    virtual ~CExpenseEditDlg() = default;

    void SetValues(const CString& amount, const CString& category, const CString& note);
    void GetValues(CString& amount, CString& category, CString& note) const;

protected:
    virtual BOOL OnInitDialog() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;

    CString m_amount;
    CString m_category;
    CString m_note;

    DECLARE_MESSAGE_MAP()
};

class CExpenseDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CExpenseDlg)
public:
    CExpenseDlg(CWnd* pParent = nullptr);
    virtual ~CExpenseDlg() = default;

protected:
    virtual BOOL OnInitDialog() override;
    afx_msg void OnAdd();
    afx_msg void OnEdit();
    afx_msg void OnDelete();
    afx_msg void OnPrint();

    void LoadRecords();
    CString EscapeSql(const CString& input);

    CListCtrl m_listExpense;
    CSqliteManager m_db;

    DECLARE_MESSAGE_MAP()
};