#pragma once
#include <stdint.h>
#include <functional>
#include <boost/bind.hpp>
#include <thread>
#include "SSL_TcpClient.h"
#include "SyncDispatcher.hpp"
#include "SyncProtobufCodec.h"
#include "StructData.hpp"
// DlgLogin 对话框

class DlgLogin : public CDialogEx
{
	DECLARE_DYNAMIC(DlgLogin)

public:
	DlgLogin( CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgLogin();

// 对话框数据
	enum { IDD = IDD_DlgLogin };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pWndParent;//父窗口指针
	CString m_userName;//用户名
	CString m_password;//密码
private:
	//X按钮处理,重载WM_CLOSE消息
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnlogin();
private:
	//事件处理
	void handleConnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleDisconnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleUnknownMessage( shared_ptr_Msg message);
	void handleMsgServerResponse( shared_ptr_Msg message);
	void handleLoginResponse(shared_ptr_Msg message);
	void doMsgServerRequest();
	void doLoginRequest(uint32_t onlinestate, const std::string& username, const std::string& password);
private:
// 	boost::asio::io_service m_service;//异步处理
// 	std::shared_ptr<AsyncTcpClient> m_ptrTcpConnection;//网络连接
	SSL_TcpClient     m_TcpConnection;			//网络连接
	SyncProtobufDispatcher     m_dispatcher;	//反射中心
	SyncProtobufCodec      m_codec;				//序列化和反序列化
public:
	std::string m_strLoginAddress;//登录服务器地址
	uint16_t m_nPortLogin;//登录服务器端口
	std::string m_strMsgAddress;//消息服务器地址
	uint16_t m_nPortMsg;//消息服务器端口
	UseInfoStruct m_userInfo;  // 用户信息
	std::string m_strEncrytoSeed;//加密种子
};
