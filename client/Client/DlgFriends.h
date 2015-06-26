#pragma once
#include "afxwin.h"
#include "DataTypeDefine.h"
// CFriends 对话框

class CDlgFriends : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFriends)

public:
	CDlgFriends(List_USERDATA& listUserData, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgFriends();

// 对话框数据
	enum { IDD = IDD_DlgFriends };

	void updateFriendsList();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	// 生成的消息映射函数;
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	List_USERDATA&  m_listUserData;             //好友信息列表
	CListBox m_listboxFriends;					//好友列表
	CWnd* m_ptrCWndParent;						//ClientDlg窗口指针,getParent()返回tab控件指针
	afx_msg void OnLbnDblclkListfriends();
};
