#ifndef  __INCLUDE_SQL_LITE_H 
#define  __INCLUDE_SQL_LITE_H  

#pragma once
#include <string>
#include <sqlite/sqlite3.h>
#pragma comment(lib, "sqlite3.lib")

class MyDataTable;
class CSqlite;

class CSqlite
{
public:
	CSqlite();
	virtual ~CSqlite();

	// 打开数据库  ;
	bool OpenDB(const char *filename);
	// 查询操作  ;查询结果集的row行数,column列数;
	int Query(const std::string& sqlstr, MyDataTable& table);
	// 执行sql语句  ;
	int ExecuteNonQuery(const std::string& sqlstr);
private:
	sqlite3 *m_ptrSqlitedb;// 数据库的指针  ;
	char* m_ptrErrmsg;     // 错误信息  ;
};
class MyDataTable
{
public:
	MyDataTable();
	~MyDataTable();
	char **m_ptrResult;  //一维指针数组存放结果;
	int m_rows;	//查询结果集的行数;
	int m_columns;	//查询结果集的列数;
private:
	int m_rowIndex;
};
#endif 

