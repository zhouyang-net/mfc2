#include "pch.h"
#include "IncomeDlg.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CIncomeEditDlg, CDialogEx)
IMPLEMENT_DYNAMIC(CIncomeDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CIncomeEditDlg, CDialogEx)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CIncomeDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_ADD_INCOME, &CIncomeDlg::OnAdd)
    ON_BN_CLICKED(IDC_BTN_EDIT_INCOME, &CIncomeDlg::OnEdit)
    ON_BN_CLICKED(IDC_BTN_DELETE_INCOME, &CIncomeDlg::OnDelete)
    ON_BN_CLICKED(IDC_BTN_PRINT_INCOME, &CIncomeDlg::OnPrint)
END_MESSAGE_MAP()

CIncomeEditDlg::CIncomeEditDlg(CWnd* pParent)
    : CDialogEx(IDD_INCOME_EDIT_DLG, pParent)
    , m_amount(_T(""))
    , m_source(_T(""))
    , m_note(_T(""))
{
}

BOOL CIncomeEditDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CIncomeEditDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_AMOUNT, m_amount);
    DDX_Text(pDX, IDC_EDIT_SOURCE, m_source);
    DDX_Text(pDX, IDC_EDIT_NOTE, m_note);
}

void CIncomeEditDlg::SetValues(const CString& amount, const CString& source, const CString& note)
{
    m_amount = amount;
    m_source = source;
    m_note = note;
}

void CIncomeEditDlg::GetValues(CString& amount, CString& source, CString& note) const
{
    amount = m_amount;
    source = m_source;
    note = m_note;
}

/* CIncomeDlg */

CIncomeDlg::CIncomeDlg(CWnd* pParent)
    : CDialogEx(IDD_INCOME_DLG, pParent)
    , m_db(L"finance.db")
{
}

BOOL CIncomeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_listIncome.SubclassDlgItem(IDC_LIST_INCOME, this);
    m_listIncome.InsertColumn(0, L"ID", LVCFMT_LEFT, 40);
    m_listIncome.InsertColumn(1, L"金额", LVCFMT_LEFT, 80);
    m_listIncome.InsertColumn(2, L"来源", LVCFMT_LEFT, 120);
    m_listIncome.InsertColumn(3, L"备注", LVCFMT_LEFT, 120);
    m_listIncome.InsertColumn(4, L"时间", LVCFMT_LEFT, 150);

    if (m_db.IsOpen()) {
        m_db.CreateSchema();
        LoadRecords();
    } else {
        AfxMessageBox(L"未能打开数据库，请检查 sqlite 配置。");
    }

    return TRUE;
}

void CIncomeDlg::LoadRecords()
{
    m_listIncome.DeleteAllItems();
    auto rows = m_db.Query("SELECT id, amount, source, note, occurred_at FROM income ORDER BY occurred_at DESC;");
    int idx = 0;
    for (auto& r : rows) {
        CString id = CA2W(r.at("id").c_str());
        CString amount = CA2W(r.at("amount").c_str());
        CString source = CA2W(r.at("source").c_str());
        CString note = CA2W(r.at("note").c_str());
        CString at = CA2W(r.at("occurred_at").c_str());

        idx = m_listIncome.InsertItem(idx, id);
        m_listIncome.SetItemText(idx, 1, amount);
        m_listIncome.SetItemText(idx, 2, source);
        m_listIncome.SetItemText(idx, 3, note);
        m_listIncome.SetItemText(idx, 4, at);
        ++idx;
    }
}

CString CIncomeDlg::EscapeSql(const CString& input)
{
    CString out = input;
    out.Replace(L"'", L"''");
    return out;
}

void CIncomeDlg::OnAdd()
{
    CIncomeEditDlg dlg(this);
    if (dlg.DoModal() == IDOK) {
        CString amount, source, note;
        dlg.GetValues(amount, source, note);
        double d = _ttof(amount);
        CString escSource = EscapeSql(source);
        CString escNote = EscapeSql(note);

        CString sql;
        sql.Format(L"INSERT INTO income (user_id, amount, source, note, occurred_at) VALUES (NULL, %f, '%s', '%s', datetime('now'));",
            d, escSource.GetString(), escNote.GetString());

        // convert to UTF-8 std::string
        CT2A utf8(sql, CP_UTF8);
        if (m_db.Execute(std::string(utf8))) {
            LoadRecords();
        } else {
            AfxMessageBox(L"插入失败。");
        }
    }
}

void CIncomeDlg::OnEdit()
{
    POSITION pos = m_listIncome.GetFirstSelectedItemPosition();
    if (!pos) { AfxMessageBox(L"请选择一条记录进行编辑。"); return; }
    int nItem = m_listIncome.GetNextSelectedItem(pos);
    CString id = m_listIncome.GetItemText(nItem, 0);
    CString amount = m_listIncome.GetItemText(nItem, 1);
    CString source = m_listIncome.GetItemText(nItem, 2);
    CString note = m_listIncome.GetItemText(nItem, 3);

    CIncomeEditDlg dlg(this);
    dlg.SetValues(amount, source, note);
    if (dlg.DoModal() == IDOK) {
        CString newAmount, newSource, newNote;
        dlg.GetValues(newAmount, newSource, newNote);
        double d = _ttof(newAmount);
        CString escSource = EscapeSql(newSource);
        CString escNote = EscapeSql(newNote);
        CString sql;
        sql.Format(L"UPDATE income SET amount=%f, source='%s', note='%s' WHERE id=%s;",
            d, escSource.GetString(), escNote.GetString(), id.GetString());
        CT2A utf8(sql, CP_UTF8);
        if (m_db.Execute(std::string(utf8))) {
            LoadRecords();
        } else {
            AfxMessageBox(L"更新失败。");
        }
    }
}

void CIncomeDlg::OnDelete()
{
    POSITION pos = m_listIncome.GetFirstSelectedItemPosition();
    if (!pos) { AfxMessageBox(L"请选择一条记录进行删除。"); return; }
    int nItem = m_listIncome.GetNextSelectedItem(pos);
    CString id = m_listIncome.GetItemText(nItem, 0);

    if (AfxMessageBox(L"确认删除所选记录？", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        CString sql;
        sql.Format(L"DELETE FROM income WHERE id=%s;", id.GetString());
        CT2A utf8(sql, CP_UTF8);
        if (m_db.Execute(std::string(utf8))) {
            LoadRecords();
        } else {
            AfxMessageBox(L"删除失败。");
        }
    }
}

void CIncomeDlg::OnPrint()
{
    // 导出为 CSV 并用默认程序打开（可由用户打印）
    CString tmpPath;
    TCHAR buf[MAX_PATH];
    GetTempPath(MAX_PATH, buf);
    tmpPath = buf;
    tmpPath += L"income_export.csv";

    auto rows = m_db.Query("SELECT id, amount, source, note, occurred_at FROM income ORDER BY occurred_at DESC;");
    CStdioFile file;
    if (!file.Open(tmpPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) {
        AfxMessageBox(L"无法创建导出文件。");
        return;
    }

    file.WriteString(L"id,amount,source,note,occurred_at\r\n");
    for (auto& r : rows) {
        CString line;
        CString id = CA2W(r.at("id").c_str());
        CString amount = CA2W(r.at("amount").c_str());
        CString source = CA2W(r.at("source").c_str());
        CString note = CA2W(r.at("note").c_str());
        CString at = CA2W(r.at("occurred_at").c_str());

        // 简单 CSV 转义（双引号）
        auto csvEsc = [](const CString& s)->CString {
            CString t = s;
            t.Replace(L"\"", L"\"\"");
            return L"\"" + t + L"\"";
        };

        line.Format(L"%s,%s,%s,%s,%s\r\n",
            csvEsc(id).GetString(),
            csvEsc(amount).GetString(),
            csvEsc(source).GetString(),
            csvEsc(note).GetString(),
            csvEsc(at).GetString());

        file.WriteString(line);
    }
    file.Close();

    ::ShellExecute(nullptr, L"open", tmpPath, nullptr, nullptr, SW_SHOWNORMAL);
}