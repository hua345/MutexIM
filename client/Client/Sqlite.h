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

	// �����ݿ�  ;
	bool OpenDB(const char *filename);
	// ��ѯ����  ;��ѯ�������row����,column����;
	int Query(const std::string& sqlstr, MyDataTable& table);
	// ִ��sql���  ;
	int ExecuteNonQuery(const std::string& sqlstr);
private:
	sqlite3 *m_ptrSqlitedb;// ���ݿ��ָ��  ;
	char* m_ptrErrmsg;     // ������Ϣ  ;
};
class MyDataTable
{
public:
	MyDataTable();
	~MyDataTable();
	char **m_ptrResult;  //һάָ�������Ž��;
	int m_rows;	//��ѯ�����������;
	int m_columns;	//��ѯ�����������;
private:
	int m_rowIndex;
};
#endif 

