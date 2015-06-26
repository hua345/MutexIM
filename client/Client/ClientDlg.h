
// ClientDlg.h : ͷ�ļ�
//

#pragma once
#include <thread>
#include "AsyncTcpClient.h"
#include "ProtobufCodec.h"
#include "ProtobufDispatcher.hpp"
#include "afxwin.h"
#include "afxcmn.h"
#include "DlgGroups.h"
#include "DlgFriends.h"
#include "DlgChat.h"
#include "DlgGroupChat.h"
#include "DataTypeDefine.h"

#include "Sqlite.h"

// CClientDlg �Ի���;
using namespace std;
class CClientDlg : public CDialogEx
{
	typedef std::map<std::string, std::shared_ptr<DlgChat>> MAP_DlgChat;			//std::pair<userId, ChatDlg>
	typedef MAP_DlgChat::iterator    MAP_DlgChat_It;
	typedef std::map<std::string, std::shared_ptr<DlgGroupChat>> MAP_DlgGroupChat;	//std::pair<userId, groupChatDlg>
	typedef MAP_DlgGroupChat::iterator    MAP_DlgGroupChat_It;
// ����;
public:
	CClientDlg(CWnd* pParent = NULL);	// ��׼���캯��;
	virtual ~CClientDlg();
// �Ի�������;
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��;


// ʵ��;
protected:
	HICON m_hIcon;
	// ���ɵ���Ϣӳ�亯��;
	virtual BOOL OnInitDialog();
	//����ͼ��;
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTcnSelchangingTabim(NMHDR *pNMHDR, LRESULT *pResult);
	//�Զ�����Ϣ�Ĵ���������;
	afx_msg LRESULT OnWMDlgChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMDlgGroupChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMCloseDlgChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMCloseDlgGroupChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMChatMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMGroupChatMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	//�¼�����;
	void handleConnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleDisconnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleUnknownMessage(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	//�յ�������Ϣ�ӷ�����;
	void handleMsgData(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handleUnreadMsgResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handleShakeResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handlePeerAllUserInfoResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handleGroupAllInfoResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	//�յ�Ⱥ��Ա��Ϣ�ӷ�����;
	void handleGroupMemberResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void InitCallback();
	//�����ʼ������;
	void doInitFirstRequest();
private:
	//������Ϣ�й�;
	boost::asio::io_service m_service;//�첽����;

	std::shared_ptr<AsyncTcpClient> m_ptrTcpConnection;//��������;

	ProtobufDispatcher m_dispatcher;//��������;

	ProtobufCodec m_codec;//���л��ͷ����л�;
private:
	//���ڿؼ��й�;
	CTabCtrl		m_tab;						//tab�ؼ�;

	MAP_DlgChat		m_mapdlgChat;			//���촰��;

	MAP_DlgGroupChat m_mapdlgGroupChat;		//Ⱥ�Ĵ���;

	NOTIFYICONDATA m_nid;
	void InitTestData();
private:
	//����
	List_USERDATA	m_listFriends;				//������Ϣ�б�;

	List_MSGDATA    m_listUnreadMsg;			//δ����Ϣ�б�;

	CDlgFriends		m_dlgFriends;			//�����б�Dialog;

	List_GroupInfo  m_listGroupInfo;		//Ⱥ����Ϣ�б�;

	MAP_GROUPMEMBER		m_allGroupMember;				//����Ⱥ����Ϣ;


	CDlgGroups		m_dlgGroups;				//Ⱥ���б�Dialog;

	USERDATA		m_userData;				//�û���Ϣ;

	std::string      m_sessionId;			//��������;

	string			m_remoteAddress;			//��Ϣ��������ַ;

	uint16_t     m_remotePort;			//��Ϣ�������˿�;
};
