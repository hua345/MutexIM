// DlgLogin.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "DlgLogin.h"
#include "afxdialogex.h"
#include "openssl/md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// DlgLogin �Ի���

IMPLEMENT_DYNAMIC(DlgLogin, CDialogEx)

DlgLogin::DlgLogin(CWnd* pParent /*=NULL*/)
: CDialogEx(DlgLogin::IDD, pParent)
, m_TcpConnection()
, m_dispatcher(std::bind(&DlgLogin::handleUnknownMessage, this, std::placeholders::_1))
, m_codec(std::bind(&SyncProtobufDispatcher::HandleSyncMessage, &m_dispatcher, std::placeholders::_1))
, m_userName(_T(""))
, m_password(_T(""))
, m_pWndParent(NULL)
, m_strLoginAddress("")
, m_nPortLogin(0)
, m_strMsgAddress("")
, m_nPortMsg(0)
, m_strEncrytoSeed("")
{
	// Verify that the version of the library that we linked against is   
	// compatible with the version of the headers we compiled against.   
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	m_pWndParent = pParent;
	m_strLoginAddress = "127.0.0.1";
	m_nPortLogin = 8080;
	//ע����Ϣ������
	m_dispatcher.registerMessageCallback(IM::MsgServResponse::descriptor()
		, std::bind(&DlgLogin::handleMsgServerResponse, this, std::placeholders::_1));
	m_dispatcher.registerMessageCallback(IM::LoginResponse::descriptor()
		, std::bind(&DlgLogin::handleLoginResponse, this, std::placeholders::_1));
	//ע������������Ϣ�ص�����
	m_TcpConnection.setMessageCallback(std::bind(&SyncProtobufCodec::onSyncMessage, &m_codec, std::placeholders::_1));
	//�������ӷ�����

	m_TcpConnection.Connect(m_strLoginAddress, m_nPortLogin);
}

DlgLogin::~DlgLogin()
{

}

void DlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_UserName, m_userName);
	DDX_Text(pDX, IDC_Password, m_password);
}


BEGIN_MESSAGE_MAP(DlgLogin, CDialogEx)
	ON_BN_CLICKED(IDC_BtnLogin, &DlgLogin::OnBnClickedBtnlogin)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


BOOL DlgLogin::PreTranslateMessage(MSG* pMsg)
{
	//�û������е�ʱ�������ע�ⰴ����ENTER����ESC��������ͻ������˳�
	//����Enter��ʱ,������а�ť��û�л�����뽹��
	//��Windows �ͻ��Զ�ȥѰ�ҳ������Դ��ָ����Ĭ�ϰ�ť
	//,OnOK����Ҳ���Զ�������
	//ESC����ԭ��Ҳ����ˣ�����Ĭ�Ϻ�OnCancel����ӳ����һ���
	//ֻ����ESC������Enter����������֤
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = static_cast<int>(pMsg->wParam);
		switch (nVirtKey)
		{
		case VK_RETURN:
		{
						  return FALSE;
		}
		case VK_ESCAPE:
		{
						  return FALSE;
		}
		default:
			break;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
//X��ť����
void DlgLogin::OnClose()
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	::SendMessage(m_pWndParent->GetSafeHwnd(), WM_CLOSE, (WPARAM)0, (LPARAM)0);
	CDialogEx::OnClose();
}
// DlgLogin ��Ϣ�������


void DlgLogin::OnBnClickedBtnlogin()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	doMsgServerRequest();
}
void DlgLogin::handleConnection(const boost::asio::ip::tcp::endpoint& endpoint)
{
	//do nothing
}

void DlgLogin::handleDisconnection(const boost::asio::ip::tcp::endpoint& endpoint)
{
	//do nothing
}

void DlgLogin::handleUnknownMessage( shared_ptr_Msg message)
{
	//do nothing
}
void DlgLogin::handleLoginResponse(shared_ptr_Msg message)
{
	std::shared_ptr<IM::LoginResponse> msg = std::dynamic_pointer_cast<IM::LoginResponse>(message);
	if (IM::RESULTERROR::SUCCESS == msg->result())// 0 -- success, non-0 -- fail reason
	{
		m_userInfo.timeTamp = msg->timetamp();
		m_strEncrytoSeed = msg->sessionid();
		m_userInfo.user_id = msg->userid();
		m_userInfo.name = msg->name();
		m_userInfo.nick_name = msg->nick_name();
		m_userInfo.email = msg->email();
		m_userInfo.position = msg->position();
		m_userInfo.image_url = msg->image_url();
		m_userInfo.sex = msg->sex();
		OnOK();
	}
	else if (IM::RESULTERROR::ONLYONE_ERROR == msg->result())
	{
		MessageBox(_T("���û���PC���Ѿ���½"));
	}
}
void DlgLogin::handleMsgServerResponse( shared_ptr_Msg message)
{
	std::shared_ptr<IM::MsgServResponse> msg = std::dynamic_pointer_cast<IM::MsgServResponse>(message);
	if (IM::RESULTERROR::SUCCESS == msg->result())// 0 -- success, non-0 -- fail reason
	{
		m_nPortMsg = msg->port();
		m_strMsgAddress = msg->address();
	}
	UpdateData(TRUE);
	//CString to char*
#ifdef UNICODE
	CW2A m_strTemp(m_userName.GetBuffer(0));
	CW2A m_strTemp2(m_password.GetBuffer(0));
	std::string usename(m_strTemp);
	std::string password(m_strTemp2);
#else
	//MultiByteToWideChar
	std::string usename(m_userName.GetBuffer(0));
	std::string password(m_password.GetBuffer(0));
#endif
	doLoginRequest(0, usename, password);
}

void DlgLogin::doMsgServerRequest()
{
	UpdateData(TRUE);
	//CString to char*
#ifdef UNICODE
	CW2A m_strTemp(m_userName.GetBuffer(0));
	std::string userid(m_strTemp);
#else
	//MultiByteToWideChar
	std::string userid(m_userName.GetBuffer(0));
#endif
	IM::MsgServerRequest msg;
	msg.set_userid(userid);
	std::shared_ptr<MyBuffer> buf = m_codec.Encode(msg);
	m_TcpConnection.Send(buf);
	m_TcpConnection.SyncRead();
}

void DlgLogin::doLoginRequest(uint32_t onlinestate, const std::string& username, const std::string& password)
{
	IM::LoginRequest msg;
	msg.set_onlinestate(onlinestate);
	msg.set_userid(username);
	//transfer password md5
	unsigned char md[MD5_DIGEST_LENGTH] = {0};
	MD5((const unsigned char*)password.c_str(), password.size(), md);
	msg.set_password(md, MD5_DIGEST_LENGTH);

	std::shared_ptr<MyBuffer> buf = m_codec.Encode(msg);
	m_TcpConnection.Send(buf);
	m_TcpConnection.SyncRead();
}
