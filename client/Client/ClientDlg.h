
// ClientDlg.h : 头文件
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

// CClientDlg 对话框;
using namespace std;
class CClientDlg : public CDialogEx
{
	typedef std::map<std::string, std::shared_ptr<DlgChat>> MAP_DlgChat;			//std::pair<userId, ChatDlg>
	typedef MAP_DlgChat::iterator    MAP_DlgChat_It;
	typedef std::map<std::string, std::shared_ptr<DlgGroupChat>> MAP_DlgGroupChat;	//std::pair<userId, groupChatDlg>
	typedef MAP_DlgGroupChat::iterator    MAP_DlgGroupChat_It;
// 构造;
public:
	CClientDlg(CWnd* pParent = NULL);	// 标准构造函数;
	virtual ~CClientDlg();
// 对话框数据;
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持;


// 实现;
protected:
	HICON m_hIcon;
	// 生成的消息映射函数;
	virtual BOOL OnInitDialog();
	//托盘图标;
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTcnSelchangingTabim(NMHDR *pNMHDR, LRESULT *pResult);
	//自定义消息的处理函数声明;
	afx_msg LRESULT OnWMDlgChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMDlgGroupChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMCloseDlgChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMCloseDlgGroupChat(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMChatMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWMGroupChatMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	//事件处理;
	void handleConnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleDisconnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleUnknownMessage(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	//收到聊天消息从服务器;
	void handleMsgData(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handleUnreadMsgResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handleShakeResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handlePeerAllUserInfoResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void handleGroupAllInfoResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	//收到群成员信息从服务器;
	void handleGroupMemberResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message);
	void InitCallback();
	//请求初始化数据;
	void doInitFirstRequest();
private:
	//网络消息有关;
	boost::asio::io_service m_service;//异步处理;

	std::shared_ptr<AsyncTcpClient> m_ptrTcpConnection;//网络连接;

	ProtobufDispatcher m_dispatcher;//反射中心;

	ProtobufCodec m_codec;//序列化和反序列化;
private:
	//窗口控件有关;
	CTabCtrl		m_tab;						//tab控件;

	MAP_DlgChat		m_mapdlgChat;			//聊天窗口;

	MAP_DlgGroupChat m_mapdlgGroupChat;		//群聊窗口;

	NOTIFYICONDATA m_nid;
	void InitTestData();
private:
	//数据
	List_USERDATA	m_listFriends;				//好友信息列表;

	List_MSGDATA    m_listUnreadMsg;			//未读消息列表;

	CDlgFriends		m_dlgFriends;			//好友列表Dialog;

	List_GroupInfo  m_listGroupInfo;		//群组信息列表;

	MAP_GROUPMEMBER		m_allGroupMember;				//所有群的信息;


	CDlgGroups		m_dlgGroups;				//群组列表Dialog;

	USERDATA		m_userData;				//用户信息;

	std::string      m_sessionId;			//加密种子;

	string			m_remoteAddress;			//消息服务器地址;

	uint16_t     m_remotePort;			//消息服务器端口;
};
