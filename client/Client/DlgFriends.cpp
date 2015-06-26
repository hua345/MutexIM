// Friends.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "DlgFriends.h"
#include "afxdialogex.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// CFriends 对话框

IMPLEMENT_DYNAMIC(CDlgFriends, CDialogEx)

CDlgFriends::CDlgFriends(List_USERDATA& listUserData, CWnd* pParent/*=NULL*/)
	: CDialogEx(CDlgFriends::IDD, pParent)
	, m_listUserData(listUserData)
	, m_ptrCWndParent(pParent)
{

}

CDlgFriends::~CDlgFriends()
{

}

void CDlgFriends::updateFriendsList()
{
	auto itBegin = m_listUserData.begin();
	int nIndex = -1;
	while (itBegin != m_listUserData.end())
	{
#ifdef UNICODE
		CA2W temp(itBegin->get()->nick_name().c_str());
		CString  userId(temp);
#else
		//MultiByteToWideChar
		CString  userId(itBegin->get()->nick_name().c_str()); 
#endif
		nIndex = m_listboxFriends.AddString(userId);
		if (nIndex >= 0)
			m_listboxFriends.SetItemDataPtr(nIndex, itBegin->get());
		++itBegin;
	}
}

void CDlgFriends::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ListFriends, m_listboxFriends);
}


BEGIN_MESSAGE_MAP(CDlgFriends, CDialogEx)
	ON_LBN_DBLCLK(IDC_ListFriends, &CDlgFriends::OnLbnDblclkListfriends)
END_MESSAGE_MAP()

BOOL CDlgFriends::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	updateFriendsList();
	return TRUE;
}


// CFriends 消息处理程序


void CDlgFriends::OnLbnDblclkListfriends()
{
	// TODO:  在此添加控件通知处理程序代码
	USERDATA* pUserData = reinterpret_cast<USERDATA*>(m_listboxFriends.GetItemData(m_listboxFriends.GetCurSel()));
	if (NULL != pUserData)
	{
		::SendMessage(m_ptrCWndParent->m_hWnd, WM_USE_DlgCHAT, (WPARAM)0, (LPARAM)pUserData);
	}	
	pUserData = NULL;
}
