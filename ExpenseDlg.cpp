#include "pch.h"
#include "ExpenseDlg.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CExpenseEditDlg, CDialogEx)
IMPLEMENT_DYNAMIC(CExpenseDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CExpenseEditDlg, CDialogEx)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CExpenseDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_ADD_EXPENSE, &CExpenseDlg::OnAdd)
    ON_BN_CLICKED(IDC_BTN_EDIT_EXPENSE, &CExpenseDlg::OnEdit)
    ON_BN_CLICKED(IDC_BTN_DELETE_EXPENSE, &CExpenseDlg::OnDelete)
    ON_BN_CLICKED(IDC_BTN_PRINT_EXPENSE, &CExpenseDlg::OnPrint)
END_MESSAGE_MAP()

CExpenseEditDlg::CExpenseEditDlg(CWnd* pParent)
    : CDialogEx(IDD_EXPENSE_EDIT_DLG, pParent)
    , m_amount(_T(""))
    , m_category(_T(""))
    , m_note(_T(""))
{
}

BOOL CExpenseEditDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CExpenseEditDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_AMOUNT, m_amount);
    DDX_Text(pDX, IDC_EDIT_CATEGORY, m_category);
    DDX_Text(pDX, IDC_EDIT_NOTE, m_note);
}

void CExpenseEditDlg::SetValues(const CString& amount, const CString& category, const CString& note)
{
    m_amount = amount;
    m_category = category;
    m_note = note;
}

void CExpenseEditDlg::GetValues(CString& amount, CString& category, CString& note) const
{
    amount = m_amount;
    category = m_category;
    note = m_note;
}

/* CExpenseDlg */

CExpenseDlg::CExpenseDlg(CWnd* pParent)
    : CDialogEx(IDD_EXPENSE_DLG, pParent)
    , m_db(L"finance.db")
{
}

BOOL CExpenseDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_listExpense.SubclassDlgItem(IDC_LIST_EXPENSE, this);
    m_listExpense.InsertColumn(0, L"ID", LVCFMT_LEFT, 40);
    m_listExpense.InsertColumn(1, L"金额", LVCFMT_LEFT, 80);
    m_listExpense.InsertColumn(2, L"类别", LVCFMT_LEFT, 120);
    m_listExpense.InsertColumn(3, L"备注", LVCFMT_LEFT, 120);
    m_listExpense.InsertColumn(4, L"时间", LVCFMT_LEFT, 150);

    if (m_db.IsOpen()) {
        m_db.CreateSchema();
        LoadRecords();
    } else {
        AfxMessageBox(L"未能打开数据库，请检查 sqlite 配置。");
    }

    return TRUE;
}

void CExpenseDlg::LoadRecords()
{
    m_listExpense.DeleteAllItems();
    auto rows = m_db.Query("SELECT id, amount, category, note, occurred_at FROM expense ORDER BY occurred_at DESC;");
    int idx = 0;
    for (auto& r : rows) {
        CString id = CA2W(r.at("id").c_str());
        CString amount = CA2W(r.at("amount").c_str());
        CString category = CA2W(r.at("category").c_str());
        CString note = CA2W(r.at("note").c_str());
        CString at = CA2W(r.at("occurred_at").c_str());

        idx = m_listExpense.InsertItem(idx, id);
        m_listExpense.SetItemText(idx, 1, amount);
        m_listExpense.SetItemText(idx, 2, category);
        m_listExpense.SetItemText(idx, 3, note);
        m_listExpense.SetItemText(idx, 4, at);
        ++idx;
    }
}

CString CExpenseDlg::EscapeSql(const CString& input)
{
    CString out = input;
    out.Replace(L"'", L"''");
    return out;
}

void CExpenseDlg::OnAdd()
{
    CExpenseEditDlg dlg(this);
    if (dlg.DoModal() == IDOK) {
        CString amount, category, note;
        dlg.GetValues(amount, category, note);
        double d = _ttof(amount);
        CString escCategory = EscapeSql(category);
        CString escNote = EscapeSql(note);

        CString sql;
        sql.Format(L"INSERT INTO expense (user_id, amount, category, note, occurred_at) VALUES (NULL, %f, '%s', '%s', datetime('now'));",
            d, escCategory.GetString(), escNote.GetString());

        CT2A utf8(sql, CP_UTF8);
        if (m_db.Execute(std::string(utf8))) {
            LoadRecords();
        } else {
            AfxMessageBox(L"插入失败。");
        }
    }
}

void CExpenseDlg::OnEdit()
{
    POSITION pos = m_listExpense.GetFirstSelectedItemPosition();
    if (!pos) { AfxMessageBox(L"请选择一条记录进行编辑。"); return; }
    int nItem = m_listExpense.GetNextSelectedItem(pos);
    CString id = m_listExpense.GetItemText(nItem, 0);
    CString amount = m_listExpense.GetItemText(nItem, 1);
    CString category = m_listExpense.GetItemText(nItem, 2);
    CString note = m_listExpense.GetItemText(nItem, 3);

    CExpenseEditDlg dlg(this);
    dlg.SetValues(amount, category, note);
    if (dlg.DoModal() == IDOK) {
        CString newAmount, newCategory, newNote;
        dlg.GetValues(newAmount, newCategory, newNote);
        double d = _ttof(newAmount);
        CString escCategory = EscapeSql(newCategory);
        CString escNote = EscapeSql(newNote);
        CString sql;
        sql.Format(L"UPDATE expense SET amount=%f, category='%s', note='%s' WHERE id=%s;",
            d, escCategory.GetString(), escNote.GetString(), id.GetString());
        CT2A utf8(sql, CP_UTF8);
        if (m_db.Execute(std::string(utf8))) {
            LoadRecords();
        } else {
            AfxMessageBox(L"更新失败。");
        }
    }
}

void CExpenseDlg::OnDelete()
{
    POSITION pos = m_listExpense.GetFirstSelectedItemPosition();
    if (!pos) { AfxMessageBox(L"请选择一条记录进行删除。"); return; }
    int nItem = m_listExpense.GetNextSelectedItem(pos);
    CString id = m_listExpense.GetItemText(nItem, 0);

    if (AfxMessageBox(L"确认删除所选记录？", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        CString sql;
        sql.Format(L"DELETE FROM expense WHERE id=%s;", id.GetString());
        CT2A utf8(sql, CP_UTF8);
        if (m_db.Execute(std::string(utf8))) {
            LoadRecords();
        } else {
            AfxMessageBox(L"删除失败。");
        }
    }
}

void CExpenseDlg::OnPrint()
{
    CString tmpPath;
    TCHAR buf[MAX_PATH];
    GetTempPath(MAX_PATH, buf);
    tmpPath = buf;
    tmpPath += L"expense_export.csv";

    auto rows = m_db.Query("SELECT id, amount, category, note, occurred_at FROM expense ORDER BY occurred_at DESC;");
    CStdioFile file;
    if (!file.Open(tmpPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) {
        AfxMessageBox(L"无法创建导出文件。");
        return;
    }

    file.WriteString(L"id,amount,category,note,occurred_at\r\n");
    for (auto& r : rows) {
        CString line;
        CString id = CA2W(r.at("id").c_str());
        CString amount = CA2W(r.at("amount").c_str());
        CString category = CA2W(r.at("category").c_str());
        CString note = CA2W(r.at("note").c_str());
        CString at = CA2W(r.at("occurred_at").c_str());

        auto csvEsc = [](const CString& s)->CString {
            CString t = s;
            t.Replace(L"\"", L"\"\"");
            return L"\"" + t + L"\"";
        };

        line.Format(L"%s,%s,%s,%s,%s\r\n",
            csvEsc(id).GetString(),
            csvEsc(amount).GetString(),
            csvEsc(category).GetString(),
            csvEsc(note).GetString(),
            csvEsc(at).GetString());

        file.WriteString(line);
    }
    file.Close();

    ::ShellExecute(nullptr, L"open", tmpPath, nullptr, nullptr, SW_SHOWNORMAL);
}

// 修复 C2509 错误：为 CExpenseEditDlg 类添加 _GetBaseClass 声明
class CExpenseEditDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CExpenseEditDlg)
public:
    // ... 其他成员声明 ...

protected:
    // 添加此声明以匹配 MFC 宏要求
    static CRuntimeClass* PASCAL _GetBaseClass();
};

// 修复 VCR001: 未找到"_GetBaseClass"的函数定义。
// 为 CExpenseEditDlg 类补充 _GetBaseClass 的定义
CRuntimeClass* PASCAL CExpenseEditDlg::_GetBaseClass()
{
    return RUNTIME_CLASS(CDialogEx);
}