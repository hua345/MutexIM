#pragma once
#include "afxwin.h"
#include "DataTypeDefine.h"

// CGroups �Ի���

class CDlgGroups : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGroups)

public:
	CDlgGroups(List_GroupInfo& groupInfo , CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgGroups();
// �Ի�������
	enum { IDD = IDD_DlgGroups };
	void updateGroupMembers();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	List_GroupInfo&   m_listGroupInfo;
	CWnd* m_ptrCWndParent;						//ClientDlg����ָ��,getParent()����tab�ؼ�ָ��
	CListBox m_listboxGroups;
public:
	afx_msg void OnLbnDblclkListgroups();
};
