#pragma once
#include "afxwin.h"
#include "DataTypeDefine.h"
// CFriends �Ի���

class CDlgFriends : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFriends)

public:
	CDlgFriends(List_USERDATA& listUserData, CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgFriends();

// �Ի�������
	enum { IDD = IDD_DlgFriends };

	void updateFriendsList();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	// ���ɵ���Ϣӳ�亯��;
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	List_USERDATA&  m_listUserData;             //������Ϣ�б�
	CListBox m_listboxFriends;					//�����б�
	CWnd* m_ptrCWndParent;						//ClientDlg����ָ��,getParent()����tab�ؼ�ָ��
	afx_msg void OnLbnDblclkListfriends();
};
