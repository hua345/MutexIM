#include "stdafx.h"
#include "Sqlite.h"

CSqlite::CSqlite()
: m_ptrSqlitedb(NULL)
, m_ptrErrmsg(NULL)
{

}
CSqlite::~CSqlite()
{
	//�ͷŵ�azResult ���ڴ�ռ�
	sqlite3_close(m_ptrSqlitedb); //�ر����ݿ�
	m_ptrSqlitedb = NULL;
}

bool CSqlite::OpenDB(const char *filename)
{
	//��ָ�������ݿ��ļ�,��������ڽ�����һ��ͬ�������ݿ��ļ�;
	bool isopened = false;
	int rc = sqlite3_open(filename, &m_ptrSqlitedb);
	if (rc)
	{
		sqlite3_close(m_ptrSqlitedb);
		isopened = false;
	}
	isopened = true;
	return isopened;//ֻ��һ�����ص�;
}
int CSqlite::Query(const std::string& sqlstr, MyDataTable& table)
{
	//��ѯ����
	//"SELECT * FROM test ";
	return sqlite3_get_table(m_ptrSqlitedb, sqlstr.c_str(), &table.m_ptrResult, &table.m_rows, &table.m_columns, &m_ptrErrmsg);
}

int CSqlite::ExecuteNonQuery(const std::string& sqlstr)
{
	//����һ����,����ñ���ڣ��򲻴�������������ʾ��Ϣ���洢��zErrMsg ��;
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

