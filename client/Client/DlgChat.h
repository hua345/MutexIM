#pragma once
#include "DataTypeDefine.h"
#include "afxwin.h"
// DlgChat 对话框

class DlgChat : public CDialogEx
{
	DECLARE_DYNAMIC(DlgChat)
public:
	DlgChat(IMChat::UserData& user, IMChat::UserData& me, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgChat();

// 对话框数据
	enum { IDD = IDD_DlgChat };

	void AppendChatMsg(IMChat::MsgData* ptrMsg, const std::string* ptrStr);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	//X按钮处理,重载WM_CLOSE消息
	afx_msg void OnClose();

	afx_msg LRESULT OnWMUpdateData(WPARAM wParam, LPARAM lParam);
	// 生成的消息映射函数;
	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnBnClickedBtnsend();
	CString m_csChatLog;
	
private:
	IMChat::UserData& m_userData;//对方信息;
	IMChat::UserData& m_userDataMe;//自己的信息;
public:
	CString m_csChatMessage;
	CEdit m_editLog;
	CEdit m_editChat;
};
