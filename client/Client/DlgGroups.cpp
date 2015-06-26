// Groups.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "DlgGroups.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CGroups 对话框

IMPLEMENT_DYNAMIC(CDlgGroups, CDialogEx)

CDlgGroups::CDlgGroups(List_GroupInfo& groupInfo, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGroups::IDD, pParent)
	, m_listGroupInfo(groupInfo)
	, m_ptrCWndParent(pParent)
{

}

CDlgGroups::~CDlgGroups()
{

}


void CDlgGroups::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTGroups, m_listboxGroups);
}


void CDlgGroups::updateGroupMembers()
{
	auto itBegin = m_listGroupInfo.begin();
	int nIndex = -1;
	while (itBegin != m_listGroupInfo.end())
	{
#ifdef UNICODE
		CA2W temp(itBegin->get()->groupname().c_str());
		CString  groupName(temp);
#else
		//MultiByteToWideChar
		CString  groupName(itBegin->get()->groupname().c_str());
#endif
		nIndex = m_listboxGroups.AddString(groupName);
		if (nIndex >= 0)
			m_listboxGroups.SetItemDataPtr(nIndex, itBegin->get());
		++itBegin;
	}
}

BEGIN_MESSAGE_MAP(CDlgGroups, CDialogEx)
	ON_LBN_DBLCLK(IDC_LISTGroups, &CDlgGroups::OnLbnDblclkListgroups)
END_MESSAGE_MAP()


// CGroups 消息处理程序


void CDlgGroups::OnLbnDblclkListgroups()
{
	// TODO:  在此添加控件通知处理程序代码
	IMChat::GroupInfo* pGroupInfo = reinterpret_cast<IMChat::GroupInfo*>(m_listboxGroups.GetItemData(m_listboxGroups.GetCurSel()));
	if (NULL != pGroupInfo )
	{
		assert(NULL != m_ptrCWndParent);
		::SendMessage(m_ptrCWndParent->m_hWnd, WM_USE_DlgGROUPCHAT, (WPARAM)0, (LPARAM)pGroupInfo);
	}
	pGroupInfo = NULL;
}
