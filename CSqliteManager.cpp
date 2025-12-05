#include "pch.h"
#include "CSqliteManager.h"
#include <windows.h>

static void DebugPrint(const std::string& s)
{
	OutputDebugStringA(s.c_str());
}

CSqliteManager::CSqliteManager(const std::wstring& dbPath)
	: m_db(nullptr)
{
	std::string utf8Path = WStringToUtf8(dbPath);
	int rc = sqlite3_open_v2(utf8Path.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
	if (rc != SQLITE_OK) {
		DebugPrint("sqlite3_open_v2 failed: " + std::string(sqlite3_errmsg(m_db ? m_db : nullptr)) + "\n");
		if (m_db) {
			sqlite3_close(m_db);
			m_db = nullptr;
		}
	}
}

CSqliteManager::~CSqliteManager()
{
	if (m_db) {
		sqlite3_close(m_db);
		m_db = nullptr;
	}
}

bool CSqliteManager::IsOpen() const
{
	return m_db != nullptr;
}

bool CSqliteManager::Execute(const std::string& sql)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_db) return false;
	char* err = nullptr;
	int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
	if (rc != SQLITE_OK) {
		std::string msg = "Execute SQL failed: ";
		if (err) {
			msg += err;
			sqlite3_free(err);
		} else {
			msg += sqlite3_errmsg(m_db);
		}
		msg += "\n";
		DebugPrint(msg);
		return false;
	}
	return true;
}

std::vector<std::map<std::string, std::string>> CSqliteManager::Query(const std::string& sql)
{
	std::vector<std::map<std::string, std::string>> rows;
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_db) return rows;

	sqlite3_stmt* stmt = nullptr;
	int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		DebugPrint(std::string("sqlite3_prepare_v2 failed: ") + sqlite3_errmsg(m_db) + "\n");
		return rows;
	}

	int colCount = sqlite3_column_count(stmt);
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		std::map<std::string, std::string> row;
		for (int i = 0; i < colCount; ++i) {
			const char* name = sqlite3_column_name(stmt, i);
			const unsigned char* text = sqlite3_column_text(stmt, i);
			row[name ? name : ""] = text ? reinterpret_cast<const char*>(text) : "";
		}
		rows.push_back(std::move(row));
	}

	if (rc != SQLITE_DONE) {
		DebugPrint(std::string("sqlite3_step error: ") + sqlite3_errmsg(m_db) + "\n");
	}

	sqlite3_finalize(stmt);
	return rows;
}

bool CSqliteManager::CreateSchema()
{
	// 简单表结构：users, income, expense
	const char* sql =
		"BEGIN TRANSACTION;\n"
		"CREATE TABLE IF NOT EXISTS users ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" username TEXT NOT NULL UNIQUE,"
		" display_name TEXT,"
		" created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
		");\n"
		"CREATE TABLE IF NOT EXISTS income ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" user_id INTEGER,"
		" amount REAL NOT NULL,"
		" source TEXT,"
		" note TEXT,"
		" occurred_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
		" FOREIGN KEY(user_id) REFERENCES users(id)"
		");\n"
		"CREATE TABLE IF NOT EXISTS expense ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" user_id INTEGER,"
		" amount REAL NOT NULL,"
		" category TEXT,"
		" note TEXT,"
		" occurred_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
		" FOREIGN KEY(user_id) REFERENCES users(id)"
		");\n"
		"COMMIT;\n";

	return Execute(sql);
}

std::string CSqliteManager::WStringToUtf8(const std::wstring& wstr)
{
	if (wstr.empty()) return {};
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}