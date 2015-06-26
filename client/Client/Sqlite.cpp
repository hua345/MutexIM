#include "stdafx.h"
#include "Sqlite.h"

CSqlite::CSqlite()
: m_ptrSqlitedb(NULL)
, m_ptrErrmsg(NULL)
{

}
CSqlite::~CSqlite()
{
	//释放掉azResult 的内存空间
	sqlite3_close(m_ptrSqlitedb); //关闭数据库
	m_ptrSqlitedb = NULL;
}

bool CSqlite::OpenDB(const char *filename)
{
	//打开指定的数据库文件,如果不存在将创建一个同名的数据库文件;
	bool isopened = false;
	int rc = sqlite3_open(filename, &m_ptrSqlitedb);
	if (rc)
	{
		sqlite3_close(m_ptrSqlitedb);
		isopened = false;
	}
	isopened = true;
	return isopened;//只有一个返回点;
}
int CSqlite::Query(const std::string& sqlstr, MyDataTable& table)
{
	//查询数据
	//"SELECT * FROM test ";
	return sqlite3_get_table(m_ptrSqlitedb, sqlstr.c_str(), &table.m_ptrResult, &table.m_rows, &table.m_columns, &m_ptrErrmsg);
}

int CSqlite::ExecuteNonQuery(const std::string& sqlstr)
{
	//创建一个表,如果该表存在，则不创建，并给出提示信息，存储在zErrMsg 中;
	//" CREATE TABLE test(ID INTEGER PRIMARY KEY,AGE INTEGER,LEVEL INTEGER,NAME VARCHAR(12),SALARY REAL);";
	//"INSERT INTO test VALUES(NULL , 1 , 1 , '201205151206', 117.9 );";
	//"DELETE FROM test WHERE AGE = 1 ;";
	return sqlite3_exec(m_ptrSqlitedb, sqlstr.c_str(), 0, 0, &m_ptrErrmsg);
}

MyDataTable::MyDataTable()
: m_ptrResult(NULL)
, m_rows(0)
, m_columns(0)
{

}

MyDataTable::~MyDataTable()
{
	sqlite3_free_table(m_ptrResult);
	m_ptrResult = NULL;
}

