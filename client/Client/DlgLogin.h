#pragma once
#include <stdint.h>
#include <functional>
#include <boost/bind.hpp>
#include <thread>
#include "SSL_TcpClient.h"
#include "SyncDispatcher.hpp"
#include "SyncProtobufCodec.h"
#include "StructData.hpp"
// DlgLogin �Ի���

class DlgLogin : public CDialogEx
{
	DECLARE_DYNAMIC(DlgLogin)

public:
	DlgLogin( CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~DlgLogin();

// �Ի�������
	enum { IDD = IDD_DlgLogin };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
	CWnd* m_pWndParent;//������ָ��
	CString m_userName;//�û���
	CString m_password;//����
private:
	//X��ť����,����WM_CLOSE��Ϣ
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnlogin();
private:
	//�¼�����
	void handleConnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleDisconnection(const boost::asio::ip::tcp::endpoint& endpoint);
	void handleUnknownMessage( shared_ptr_Msg message);
	void handleMsgServerResponse( shared_ptr_Msg message);
	void handleLoginResponse(shared_ptr_Msg message);
	void doMsgServerRequest();
	void doLoginRequest(uint32_t onlinestate, const std::string& username, const std::string& password);
private:
// 	boost::asio::io_service m_service;//�첽����
// 	std::shared_ptr<AsyncTcpClient> m_ptrTcpConnection;//��������
	SSL_TcpClient     m_TcpConnection;			//��������
	SyncProtobufDispatcher     m_dispatcher;	//��������
	SyncProtobufCodec      m_codec;				//���л��ͷ����л�
public:
	std::string m_strLoginAddress;//��¼��������ַ
	uint16_t m_nPortLogin;//��¼�������˿�
	std::string m_strMsgAddress;//��Ϣ��������ַ
	uint16_t m_nPortMsg;//��Ϣ�������˿�
	UseInfoStruct m_userInfo;  // �û���Ϣ
	std::string m_strEncrytoSeed;//��������
};
