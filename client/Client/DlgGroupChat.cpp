// DlgGroupChat.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "DlgGroupChat.h"
#include "afxdialogex.h"


// DlgGroupChat 对话框

IMPLEMENT_DYNAMIC(DlgGroupChat, CDialogEx)

DlgGroupChat::DlgGroupChat(IMChat::GroupInfo& groupInfo, USERDATA& me,  CWnd* pParent /*=NULL*/)
	: CDialogEx(DlgGroupChat::IDD, pParent)
	, m_groupInfo(groupInfo)
	, m_userDataMe(me)
	, m_csChatLog(_T(""))
	, m_csGroupMessage(_T(""))
	, m_ptrMember(NULL)
{

}

DlgGroupChat::~DlgGroupChat()
{
}

void DlgGroupChat::AppendChatMsg(IMChat::MsgData* ptrMsg, const std::string* ptrStr)
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

void DlgGroupChat::updateMemberInfo(std::shared_ptr<List_GroupMember> ptrMemberList)
{
	//更新列表信息;
	//删除以前的信息;
	m_listboxMembers.ResetContent();
	//添加信息;
	auto itBegin = ptrMemberList->begin();
	int nIndex = -1;
	while (itBegin != ptrMemberList->end())
	{
#ifdef UNICODE
		CA2W temp(itBegin->nick_name().c_str());
		CString  userId(temp);
#else
		//MultiByteToWideChar
		CString  userId(itBegin->nick_name().c_str());
#endif
		nIndex = m_listboxMembers.AddString(userId);
		if (nIndex >= 0)
			m_listboxMembers.SetItemDataPtr(nIndex, &*itBegin);

		++itBegin;
	}
}

void DlgGroupChat::OnClose()
{
	::SendMessage(GetParent()->m_hWnd, WM_USE_CloseDlgGroupChat, (WPARAM)0, (LPARAM)&m_groupInfo.groupid());
	CDialogEx::OnClose();
}

void DlgGroupChat::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EditGroupChatLog, m_csChatLog);
	DDX_Text(pDX, IDC_EditGroupChat, m_csGroupMessage);
	DDX_Control(pDX, IDC_ListMember, m_listboxMembers);
	DDX_Control(pDX, IDC_EditGroupChatLog, m_editLog);
}


BOOL DlgGroupChat::OnInitDialog()
{
	CDialogEx::OnInitDialog();

#ifdef UNICODE
	CA2W m_strTemp(m_groupInfo.groupname().c_str());
	CString Title(m_strTemp);
#else
	//MultiByteToWideChar
	CString Title(m_groupInfo.groupname().c_str());
#endif
	this->SetWindowText(Title);

	return TRUE;
}

LRESULT DlgGroupChat::OnWMUpdateData(WPARAM wParam, LPARAM lParam)
{
	std::string* nickName = reinterpret_cast<std::string*>(wParam);

	IMChat::MsgData* ptrMsg = reinterpret_cast<IMChat::MsgData*>(lParam);
	if (NULL != ptrMsg)
	{
		AppendChatMsg(ptrMsg, nickName);
	}

	return 0;
}

BEGIN_MESSAGE_MAP(DlgGroupChat, CDialogEx)
	ON_BN_CLICKED(IDC_BtnSend, &DlgGroupChat::OnBnClickedBtnsend)
	ON_MESSAGE(WM_USE_UPDATE_DATA, &DlgGroupChat::OnWMUpdateData)
END_MESSAGE_MAP()


// DlgGroupChat 消息处理程序


void DlgGroupChat::OnBnClickedBtnsend()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!m_csGroupMessage.IsEmpty())
	{
		//显示自己发送的消息;
		SYSTEMTIME curtime;
		GetLocalTime(&curtime);
		CString csTime;
		csTime.Format(_T("%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d"),
			curtime.wYear, curtime.wMonth + 1, curtime.wDay,
			curtime.wHour, curtime.wMinute, curtime.wSecond);


#ifdef UNICODE
		CW2A m_strTemp(m_csGroupMessage.GetBuffer(0));
		std::string strMessage(m_strTemp);
#else
		//MultiByteToWideChar
		std::string strMessage(m_csChatMessage.GetBuffer(0));
#endif
#ifdef UNICODE
		CA2W nickNameTemp(m_userDataMe.nick_name().c_str());
		CString csNickName(nickNameTemp);
#else
		//MultiByteToWideChar
		CString csNickName(m_userData.nick_name().c_str());
#endif
		m_csChatLog.Append(csNickName);
		m_csChatLog.Append(csTime);
		m_csChatLog.Append(_T("\r\n"));
		m_csChatLog.Append(m_csGroupMessage);
		m_csChatLog.Append(_T("\r\n"));

		m_csGroupMessage.SetString(_T(""));
		UpdateData(FALSE);
		//滚动条自动下拉;
		::PostMessage(m_editLog.GetSafeHwnd(), WM_VSCROLL, SB_BOTTOM, 0);

		::SendMessage(GetParent()->m_hWnd, WM_USE_GROUPCHATMESSAGE, (WPARAM)&strMessage, (LPARAM)&m_groupInfo.groupid());
	}
	else
	{
		MessageBox(_T("不能发送空消息"), _T("提示"));
	}	
}
