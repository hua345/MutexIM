#pragma once
#include <memory>
#include <list>
#include "DataTypeDefine.h"
#include "afxwin.h"

// DlgGroupChat �Ի���

class DlgGroupChat : public CDialogEx
{
	DECLARE_DYNAMIC(DlgGroupChat)

public:
	DlgGroupChat(IMChat::GroupInfo& group, USERDATA& me, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~DlgGroupChat();

	void AppendChatMsg(IMChat::MsgData* ptrMsg, const std::string* ptrStr);

	void updateMemberInfo(std::shared_ptr<List_GroupMember> ptrMemberList);
// �Ի�������
	enum { IDD = IDD_DlgGroupChat };
	//X��ť����,����WM_CLOSE��Ϣ
	afx_msg void OnClose();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();


	afx_msg LRESULT OnWMUpdateData(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	IMChat::GroupInfo& m_groupInfo;//Ⱥ��Ϣ;
	std::shared_ptr<List_GroupMember>  m_ptrMember; //Ⱥ��Ա��Ϣ;

	USERDATA&          m_userDataMe;//�Լ�����Ϣ;
public:
	CString m_csChatLog;	//��Ϣ��¼;
	CString m_csGroupMessage;	//��Ϣ�༭��;
	afx_msg void OnBnClickedBtnsend();
	CListBox m_listboxMembers;//��Ա�б�;
	CEdit m_editLog;
};
