#pragma once

#include "pch.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "sqlite3.h"

// 简单的 SQLite 管理类，封装打开/关闭、执行查询与建表
class CSqliteManager
{
public:
	CSqliteManager(const std::wstring& dbPath = L"finance.db");
	~CSqliteManager();

	bool IsOpen() const;
	// 执行不返回行的 SQL（CREATE、INSERT、UPDATE、DELETE 等）
	bool Execute(const std::string& sql);
	// 执行查询并以 vector<map<column, value>> 返回结果
	std::vector<std::map<std::string, std::string>> Query(const std::string& sql);
	// 创建默认表（users, income, expense）
	bool CreateSchema();

private:
	sqlite3* m_db;
	std::mutex m_mutex;

	// 将宽字符串转换为 UTF-8
	std::string WStringToUtf8(const std::wstring& wstr);
};