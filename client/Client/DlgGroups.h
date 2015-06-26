#pragma once
#include "afxwin.h"
#include "DataTypeDefine.h"

// CGroups 对话框

class CDlgGroups : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGroups)

public:
	CDlgGroups(List_GroupInfo& groupInfo , CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgGroups();
// 对话框数据
	enum { IDD = IDD_DlgGroups };
	void updateGroupMembers();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	List_GroupInfo&   m_listGroupInfo;
	CWnd* m_ptrCWndParent;						//ClientDlg窗口指针,getParent()返回tab控件指针
	CListBox m_listboxGroups;
public:
	afx_msg void OnLbnDblclkListgroups();
};
