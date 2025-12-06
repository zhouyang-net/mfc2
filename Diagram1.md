```mermaid
graph LR
  User["用户 (GUI 操作)"]
  
  subgraph "应用: `mfc2`"
    App["`Cmfc2App`\\n(应用入口，Init/Exit)"]
    MDI["`CMainFrame`\\n(MDI 框架，菜单/工具栏)"]
    View["`CChildView`\\n(主视图，绘制/消息处理)"]
    Dialogs["对话框集合\\n(`IncomeDlg`, `ExpenseDlg`, `*_Edit_Dlg`)"]
    MsgMap["消息映射\\n(OnManageIncome/OnManageExpense/OnPaint)"]
    DBMgr["`CSqliteManager`\\n(SQLite 封装，CreateSchema/Query)"]
  end

  subgraph "外部/资源"
    DBFile["`finance.db` (SQLite 文件)"]
    SQLiteLib["`sqlite3` 库"]
    Res["`resource.h` / 资源(rc)"]
    PCH["`pch.h` / 框架头"]
  end

  User -->|菜单 / 界面操作| MDI
  User -->|直接交互| View

  App --> MDI
  App --> View
  MDI --> MsgMap
  View --> MsgMap
  MsgMap -->|命令/消息| Dialogs
  MsgMap -->|绘制/显示| View
  Dialogs -->|增/删/查/改| DBMgr
  View -->|示例查询| DBMgr

  DBMgr --> DBFile
  DBMgr --> SQLiteLib
  MDI --> Res
  App --> PCH
  ```
  