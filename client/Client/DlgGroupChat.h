#pragma once
#include <memory>
#include <list>
#include "DataTypeDefine.h"
#include "afxwin.h"

// DlgGroupChat 对话框

class DlgGroupChat : public CDialogEx
{
	DECLARE_DYNAMIC(DlgGroupChat)

public:
	DlgGroupChat(IMChat::GroupInfo& group, USERDATA& me, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgGroupChat();

	void AppendChatMsg(IMChat::MsgData* ptrMsg, const std::string* ptrStr);

	void updateMemberInfo(std::shared_ptr<List_GroupMember> ptrMemberList);
// 对话框数据
	enum { IDD = IDD_DlgGroupChat };
	//X按钮处理,重载WM_CLOSE消息
	afx_msg void OnClose();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();


	afx_msg LRESULT OnWMUpdateData(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	IMChat::GroupInfo& m_groupInfo;//群信息;
	std::shared_ptr<List_GroupMember>  m_ptrMember; //群成员信息;

	USERDATA&          m_userDataMe;//自己的信息;
public:
	CString m_csChatLog;	//消息记录;
	CString m_csGroupMessage;	//消息编辑框;
	afx_msg void OnBnClickedBtnsend();
	CListBox m_listboxMembers;//成员列表;
	CEdit m_editLog;
};
