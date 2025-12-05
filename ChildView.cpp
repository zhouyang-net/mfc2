// ChildView.cpp: CChildView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "mfc2.h"
#include "ChildView.h"
#include "CSqliteManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 全局静态数据库管理实例（示例用途）
static CSqliteManager g_db(L"finance.db");

// CChildView

CChildView::CChildView()
{
	// 第一次构造时尝试创建 schema
	if (g_db.IsOpen()) {
		g_db.CreateSchema();
	}
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CChildView 消息处理程序

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	// 演示：查询 income 与 expense 的记录数并显示
	CString text;
	if (g_db.IsOpen()) {
		auto rowsIncome = g_db.Query("SELECT COUNT(*) AS cnt FROM income;");
		auto rowsExpense = g_db.Query("SELECT COUNT(*) AS cnt FROM expense;");

		std::string incCnt = "0";
		std::string expCnt = "0";
		if (!rowsIncome.empty()) {
			auto it = rowsIncome[0].find("cnt");
			if (it != rowsIncome[0].end()) incCnt = it->second;
		}
		if (!rowsExpense.empty()) {
			auto it = rowsExpense[0].find("cnt");
			if (it != rowsExpense[0].end()) expCnt = it->second;
		}

		text.Format(L"收入记录数: %S\r\n支出记录数: %S", incCnt.c_str(), expCnt.c_str());
	} else {
		text = L"数据库未打开，请检查 sqlite3 配置。";
	}

	dc.TextOutW(10, 10, text);

	// 不要为绘制消息而调用 CWnd::OnPaint()
}

