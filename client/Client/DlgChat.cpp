// DlgChat.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "DlgChat.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// DlgChat 对话框

IMPLEMENT_DYNAMIC(DlgChat, CDialogEx)

DlgChat::DlgChat(USERDATA& user, USERDATA& me, CWnd* pParent /*=NULL*/)
: CDialogEx(DlgChat::IDD, pParent)
, m_userData(user)
, m_userDataMe(me)
, m_csChatLog(_T(""))
, m_csChatMessage(_T(""))
{

}

DlgChat::~DlgChat()
{
}

void DlgChat::AppendChatMsg(IMChat::MsgData* ptrMsg, const std::string* ptrStr)
{
	CString csTime;
	time_t msgTime = ptrMsg->timestamp();
	struct tm msgTm;
	localtime_s(&msgTm, &msgTime);
	csTime.Format(_T("%d-%d-%d %d:%d:%d"), msgTm.tm_year + 1900, msgTm.tm_mon + 1, msgTm.tm_mday, msgTm.tm_hour, msgTm.tm_min, msgTm.tm_sec);
#ifdef UNICODE
	CA2W dataTemp(ptrMsg->msgdata().c_str());//ptrMsg->msgdata().c_str()
	CString csContent(dataTemp);
	CA2W nickNameTemp(ptrStr->c_str());//ptrMsg->msgdata().c_str()
	CString nickName(nickNameTemp);
#else
	//MultiByteToWideChar
	CString csContent(ptrMsg->msgdata().c_str());
	CString nickName(ptrStr->c_str());
#endif
	m_csChatLog.Append(nickName);
	m_csChatLog.Append(_T("  "));
	m_csChatLog.Append(csTime);
	m_csChatLog.Append(_T("\r\n"));
	m_csChatLog.Append(csContent);
	m_csChatLog.Append(_T("\r\n"));
	//滚动条自动下拉;
	::PostMessage(m_editLog.GetSafeHwnd(), WM_VSCROLL, SB_BOTTOM, 0);
	UpdateData(FALSE);
}

void DlgChat::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EditChatLog, m_csChatLog);
	DDX_Text(pDX, IDC_EditChat, m_csChatMessage);
	DDX_Control(pDX, IDC_EditChatLog, m_editLog);
	DDX_Control(pDX, IDC_EditChat, m_editChat);
}


BOOL DlgChat::OnInitDialog()
{
	CDialogEx::OnInitDialog();

#ifdef UNICODE
	CA2W m_strTemp(m_userData.nick_name().c_str());
	CString Title(m_strTemp);
#else
	//MultiByteToWideChar
	CString Title(m_userData.nick_name().c_str());
#endif
	this->SetWindowText(Title);
	
	return TRUE;
}

BEGIN_MESSAGE_MAP(DlgChat, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BtnSend, &DlgChat::OnBnClickedBtnsend)
	ON_MESSAGE(WM_USE_UPDATE_DATA, &DlgChat::OnWMUpdateData)
END_MESSAGE_MAP()

void DlgChat::OnClose()
{
	::SendMessage(GetParent()->m_hWnd, WM_USE_CloseDlgChat, (WPARAM)0, (LPARAM)&m_userData.user_id());
	CDialogEx::OnClose();
}


LRESULT DlgChat::OnWMUpdateData(WPARAM wParam, LPARAM lParam)
{
	std::string* nickName = reinterpret_cast<std::string*>(wParam);

	IMChat::MsgData* ptrMsg = reinterpret_cast<IMChat::MsgData*>(lParam);
	if (NULL != ptrMsg)
	{
		AppendChatMsg(ptrMsg, nickName);
	}

	return 0;
}

// DlgChat 消息处理程序


void DlgChat::OnBnClickedBtnsend()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!m_csChatMessage.IsEmpty())
	{
		//显示自己发送的消息;
		SYSTEMTIME curtime;
		GetLocalTime(&curtime);
		CString csTime;
		csTime.Format(_T("%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d"),
		curtime.wYear, curtime.wMonth + 1, curtime.wDay,
		curtime.wHour, curtime.wMinute, curtime.wSecond);


#ifdef UNICODE
		CW2A m_strTemp(m_csChatMessage.GetBuffer(0));
		std::string strMessage(m_strTemp);
#else
		//MultiByteToWideChar
		std::string strMessage(m_csChatMessage.GetBuffer(0));
#endif
#ifdef UNICODE
		CA2W nicknameTemp(m_userDataMe.nick_name().c_str());
		CString csNickName(nicknameTemp);
#else
		//MultiByteToWideChar
		CString csNickName(m_userData.nick_name().c_str());
#endif
		m_csChatLog.Append(csNickName);
		m_csChatLog.Append(csTime);
		m_csChatLog.Append(_T("\r\n"));
		m_csChatLog.Append(m_csChatMessage);
		m_csChatLog.Append(_T("\r\n"));

		m_csChatMessage.SetString(_T(""));
		UpdateData(FALSE);
		//滚动条自动下拉;
		::PostMessage(m_editLog.GetSafeHwnd(), WM_VSCROLL, SB_BOTTOM, 0);
		::SendMessage(GetParent()->m_hWnd, WM_USE_ChatMessage, (WPARAM)&strMessage, (LPARAM)&m_userData.user_id());
	}
	else
	{
		MessageBox(_T("不能发送空消息"), _T("提示"));
	}
}
