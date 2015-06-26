#pragma once
#include "DataTypeDefine.h"
#include "afxwin.h"
// DlgChat �Ի���

class DlgChat : public CDialogEx
{
	DECLARE_DYNAMIC(DlgChat)
public:
	DlgChat(IMChat::UserData& user, IMChat::UserData& me, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~DlgChat();

// �Ի�������
	enum { IDD = IDD_DlgChat };

	void AppendChatMsg(IMChat::MsgData* ptrMsg, const std::string* ptrStr);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	//X��ť����,����WM_CLOSE��Ϣ
	afx_msg void OnClose();

	afx_msg LRESULT OnWMUpdateData(WPARAM wParam, LPARAM lParam);
	// ���ɵ���Ϣӳ�亯��;
	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnBnClickedBtnsend();
	CString m_csChatLog;
	
private:
	IMChat::UserData& m_userData;//�Է���Ϣ;
	IMChat::UserData& m_userDataMe;//�Լ�����Ϣ;
public:
	CString m_csChatMessage;
	CEdit m_editLog;
	CEdit m_editChat;
};
