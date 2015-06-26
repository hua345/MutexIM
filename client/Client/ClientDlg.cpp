
// ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"
#include "Message.pb.h"
#include "DlgLogin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
	, m_service(NULL)
	, m_ptrTcpConnection(new AsyncTcpClient(m_service, this))
	, m_dispatcher(std::bind(&CClientDlg::handleUnknownMessage, this, std::placeholders::_1, std::placeholders::_2))
	, m_codec(std::bind(&ProtobufDispatcher::HandleMessage, &m_dispatcher, std::placeholders::_1, std::placeholders::_2))
	, m_listFriends()
	, m_dlgFriends(m_listFriends, this)
	, m_dlgGroups(m_listGroupInfo, this)
{
	// Verify that the version of the library that we linked against is   
	// compatible with the version of the headers we compiled against.   
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	InitCallback();
	//托盘图标;
	memset(&m_nid, 0, sizeof(m_nid)); // Initialize NOTIFYICONDATA struct
	m_nid.cbSize = sizeof(m_nid);
	m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
}

CClientDlg::~CClientDlg()
{
	//销毁窗口;
	m_mapdlgChat.clear();
	m_mapdlgGroupChat.clear();
	m_dlgFriends.DestroyWindow();
	m_dlgGroups.DestroyWindow();
	//关闭连接;
	m_ptrTcpConnection->Disconnect();
	// Optional:  Delete all global objects allocated by libprotobuf.   
	google::protobuf::ShutdownProtobufLibrary();
	//托盘图标;
	m_nid.hIcon = NULL;
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TabIM, m_tab);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGING, IDC_TabIM, &CClientDlg::OnTcnSelchangingTabim)
	ON_MESSAGE(WM_USE_DlgCHAT, OnWMDlgChat)
	ON_MESSAGE(WM_USE_DlgGROUPCHAT, &CClientDlg::OnWMDlgGroupChat)
	ON_MESSAGE(WM_USE_CloseDlgChat, &CClientDlg::OnWMCloseDlgChat)
	ON_MESSAGE(WM_USE_ChatMessage, &CClientDlg::OnWMChatMessage)
	ON_MESSAGE(WM_TRAYNOTIFY, &CClientDlg::OnTrayNotify)
	ON_MESSAGE(WM_USE_GROUPCHATMESSAGE, &CClientDlg::OnWMGroupChatMessage)

END_MESSAGE_MAP()


// CClientDlg 消息处理程序
BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	//创建工作线程

	//先出现登录框
	DlgLogin m_dlgLogin(this);
	BOOL nResponse = m_dlgLogin.DoModal();
	if (IDOK == nResponse)
	{
		//加密种子;
		m_sessionId = m_dlgLogin.m_strEncrytoSeed;
		//当前用户信息;
		m_userData.set_timestamp(m_dlgLogin.m_userInfo.timeTamp);
		m_userData.set_user_id(m_dlgLogin.m_userInfo.user_id);
		m_userData.set_name(m_dlgLogin.m_userInfo.name);
		m_userData.set_nick_name(m_dlgLogin.m_userInfo.nick_name);
		m_userData.set_email(m_dlgLogin.m_userInfo.email);
		m_userData.set_image_url(m_dlgLogin.m_userInfo.image_url);
		m_userData.set_position(m_dlgLogin.m_userInfo.position);
		m_userData.set_sex(m_dlgLogin.m_userInfo.sex);
		//消息服务器地址;
		m_remoteAddress = m_dlgLogin.m_strMsgAddress;
		m_remotePort = m_dlgLogin.m_nPortMsg;

		//创建与消息服务器的长连接;
		m_ptrTcpConnection->Connect(m_remoteAddress, m_remotePort);
		std::thread th(boost::bind(&boost::asio::io_service::run, &m_service));
		th.detach();

		IMChat::shakeHandleReq  msg;
		time_t nowtime;
		time(&nowtime);

		msg.set_timestamp(nowtime);
		msg.set_userid(m_userData.user_id());
		m_codec.Send(m_ptrTcpConnection, msg);
	}
	else if (IDCANCEL == nResponse)
	{

	}
	//-------------------------------------MFC------------------------------------//
	//设置标题;
#ifdef UNICODE
	CA2W m_strTemp(m_userData.nick_name().c_str());
	CString Title(m_strTemp);
#else
	//MultiByteToWideChar
	CString Title(m_userData.nick_name().c_str());
#endif
	this->SetWindowText(Title);
	//创建Tab页;
	CRect tabRect; // 标签控件客户区的位置和大小  ;
	m_tab.InsertItem(0, _T("好友"));// 插入第一个标签;
	m_tab.InsertItem(1, _T("群组"));// 插入第二个标签;
	m_dlgFriends.Create(IDD_DlgFriends, &m_tab);// 创建第一个标签页;
	m_dlgGroups.Create(IDD_DlgGroups,&m_tab);// 创建第二个标签页;   
	m_tab.GetClientRect(&tabRect);
	// 调整tabRect，使其覆盖范围适合放置标签页   
	tabRect.left += 1;
	tabRect.right -= 1;
	tabRect.top += 25;
	tabRect.bottom -= 1;
	// 根据调整好的tabRect放置m_dlgFriends子对话框，并设置为显示   
	m_dlgFriends.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
	// 根据调整好的tabRect放置m_dlgGroups子对话框，并设置为隐藏   
	m_dlgGroups.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
	//托盘图标;
	m_nid.hWnd = GetSafeHwnd();
	m_nid.uCallbackMessage = WM_TRAYNOTIFY;
	// Set tray icon and tooltip
	m_nid.hIcon = m_hIcon;
	// Set tray notification tip information
	CString strToolTip = _T("托盘程序");
	_tcsncpy_s(m_nid.szTip, strToolTip, strToolTip.GetLength());
	Shell_NotifyIcon(NIM_ADD, &m_nid);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

LRESULT CClientDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	UINT uMsg = (UINT)lParam;

	switch (uMsg)
	{
	case WM_RBUTTONUP:
	{
		//右键处理
		CMenu menuTray;
		CPoint point;
		int id;
		GetCursorPos(&point);

		menuTray.LoadMenu(IDR_MENU1);
		id = menuTray.GetSubMenu(0)->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
		switch(id){
		case ID_SubMenuExit:
			OnOK();
			break;
		case ID_SubMenuDisplay:
			//窗口前端显示
			SetForegroundWindow();
			ShowWindow(SW_SHOWNORMAL);
			break;
		default:
			break;
		}
		break;
	}
	case WM_LBUTTONDBLCLK:
		SetForegroundWindow();
		ShowWindow(SW_SHOWNORMAL);
		break;
	default:
		break;
	}
	return 0;

}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CClientDlg::handleConnection(const boost::asio::ip::tcp::endpoint& endpoint)
{

}

void CClientDlg::handleDisconnection(const boost::asio::ip::tcp::endpoint& endpoint)
{

}

void CClientDlg::handleUnknownMessage(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{

}

void CClientDlg::handleMsgData(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{
	auto ptrRes = std::dynamic_pointer_cast<IMChat::MsgData>(message);
	if (NULL != ptrRes)
	{
		if (IMChat::MsgData_MSGTYPE_CHAT_TEXT == ptrRes->type() || IMChat::MsgData_MSGTYPE_CHAT_AUDIO == ptrRes->type())
		{
			//如果创建了对话窗口,显示信息内容;
			auto it = m_mapdlgChat.find(ptrRes->fromid());
			if (it != m_mapdlgChat.end())
			{
				//MFC对象不能跨线程访问;
				//it->second->AppendChatMsg(ptrRes);
				string fromidNickName;
				auto itBegin = m_listFriends.begin();
				while (itBegin != m_listFriends.end())
				{
					if (itBegin->get()->user_id() == ptrRes->fromid())
					{
						fromidNickName = itBegin->get()->nick_name();
						break;
					}
				}
				::SendMessage(it->second->GetSafeHwnd(), WM_USE_UPDATE_DATA, (WPARAM)&fromidNickName, (LPARAM)ptrRes.get());
			}
		}
		else if (IMChat::MsgData_MSGTYPE_GROUPCHAT_TEXT == ptrRes->type() || IMChat::MsgData_MSGTYPE_GROUPCHAT_AUDIO == ptrRes->type())
		{
			//如果创建了对话窗口,显示信息内容;
			auto it = m_mapdlgGroupChat.find(ptrRes->toid());
			if (it != m_mapdlgGroupChat.end())
			{
				//MFC对象不能跨线程访问;
				//it->second->AppendChatMsg(ptrRes);
				string fromidNickName;
				auto itBegin = m_listFriends.begin();
				while (itBegin != m_listFriends.end())
				{
					if (itBegin->get()->user_id() == ptrRes->fromid())
					{
						fromidNickName = itBegin->get()->nick_name();
						break;
					}
				}
				::SendMessage(it->second->GetSafeHwnd(), WM_USE_UPDATE_DATA, (WPARAM)&fromidNickName, (LPARAM)ptrRes.get());
			}
		}
	}
}

void CClientDlg::handleUnreadMsgResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{
	auto ptrRes = std::dynamic_pointer_cast<IMChat::UnreadMsgResponse>(message);
	if (NULL != ptrRes && ptrRes->msgcount() == ptrRes->msgdata_size())
	{
		for (int i = 0; i < ptrRes->msgdata_size(); ++i)
		{
			auto ptrMsg = make_shared<IMChat::MsgData>();
			ptrMsg->ParseFromArray(ptrRes->msgdata(i).c_str(), ptrRes->msgdata(i).size());			 
			 m_listUnreadMsg.push_back(std::dynamic_pointer_cast<IMChat::MsgData>(ptrMsg));
		}
	}
}

void CClientDlg::handleShakeResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{
	auto ptrRes = std::dynamic_pointer_cast<IMChat::shakeHandleRes>(message);
	if (0 == ptrRes->result())
	{
		doInitFirstRequest();
	}
	else
	{
		//error handle
	}
}

void CClientDlg::handlePeerAllUserInfoResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{
	auto ptrRes = std::dynamic_pointer_cast<IMChat::PeerAllUserInfoResponse>(message);
	if (NULL != ptrRes && ptrRes->friendscount() == ptrRes->msgdata_size())
	{
		for (int i = 0; i < ptrRes->msgdata_size(); ++i)
		{
			auto ptrFriend = make_shared<USERDATA>(ptrRes->msgdata(i));
			m_listFriends.push_back(ptrFriend);
		}
		//更新好友对话框;
		m_dlgFriends.updateFriendsList();
	}
}

void CClientDlg::handleGroupAllInfoResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{
	auto ptrRes = std::dynamic_pointer_cast<IMChat::GroupAllResponse>(message);
	if (NULL != ptrRes && ptrRes->groupcount() == ptrRes->msgdata_size())
	{
		for (int i = 0; i < ptrRes->msgdata_size(); ++i)
		{
			auto ptrGroup = make_shared<IMChat::GroupInfo>(ptrRes->msgdata(i));
			m_listGroupInfo.push_back(ptrGroup);
		}
		
		if (NULL != m_dlgGroups)
		{
			m_dlgGroups.updateGroupMembers();
		}
	}
}

void CClientDlg::handleGroupMemberResponse(const TcpConnectionPtr& conn, shared_ptr_Msg message)
{
	auto ptrRes = std::dynamic_pointer_cast<IMChat::GroupMemberResponse>(message);
	if (NULL != ptrRes && ptrRes->membercount() == ptrRes->msgdata_size())
	{
		
		m_allGroupMember[ptrRes->groupid()] = make_shared<List_GroupMember>();

		auto it = m_allGroupMember.find(ptrRes->groupid());
		if (it != m_allGroupMember.end())
		{
			for (int i = 0; i < ptrRes->msgdata_size(); i++)
			{
				it->second->push_back(ptrRes->msgdata(i));
			}
			
			auto itDlg = m_mapdlgGroupChat.find(ptrRes->groupid());
			if (itDlg != m_mapdlgGroupChat.end())
			{
				itDlg->second->updateMemberInfo(it->second);
			}
		}
	}
}

void CClientDlg::InitCallback()
{
	//注册消息处理函数
	m_dispatcher.registerMessageCallback(IMChat::MsgData::descriptor()
		, std::bind(&CClientDlg::handleMsgData, this, std::placeholders::_1, std::placeholders::_2));
	//未读消息响应;
	m_dispatcher.registerMessageCallback(IMChat::UnreadMsgResponse::descriptor()
		, std::bind(&CClientDlg::handleUnreadMsgResponse, this, std::placeholders::_1, std::placeholders::_2));
	//回话握手响应;
	m_dispatcher.registerMessageCallback(IMChat::shakeHandleRes::descriptor()
		, std::bind(&CClientDlg::handleShakeResponse, this, std::placeholders::_1, std::placeholders::_2));
	////所有好友信息返回 ;
	m_dispatcher.registerMessageCallback(IMChat::PeerAllUserInfoResponse::descriptor()
		, std::bind(&CClientDlg::handlePeerAllUserInfoResponse, this, std::placeholders::_1, std::placeholders::_2));
	//所有群信息响应;
	m_dispatcher.registerMessageCallback(IMChat::GroupAllResponse::descriptor()
		, std::bind(&CClientDlg::handleGroupAllInfoResponse, this, std::placeholders::_1, std::placeholders::_2));
	//群成员信息响应;
	m_dispatcher.registerMessageCallback(IMChat::GroupMemberResponse::descriptor()
		, std::bind(&CClientDlg::handleGroupMemberResponse, this, std::placeholders::_1, std::placeholders::_2));
	//注册网络连接消息回调函数
	m_ptrTcpConnection->setMessageCallback(std::bind(&ProtobufCodec::onMessage, &m_codec, std::placeholders::_1, std::placeholders::_2));
	m_ptrTcpConnection->setConnectionCallback(std::bind(&CClientDlg::handleConnection, this, std::placeholders::_1));
	m_ptrTcpConnection->setDisconnectedCallback(std::bind(&CClientDlg::handleDisconnection, this, std::placeholders::_1));
}


void CClientDlg::doInitFirstRequest()
{
	time_t nowtime;
	time(&nowtime);
	//所有好友信息请求
	IMChat::PeerAllUserInfoRequest usersInfoReq;
	usersInfoReq.set_userid(m_userData.user_id());
	usersInfoReq.set_timestamp(nowtime);
	m_codec.Send(m_ptrTcpConnection, usersInfoReq);
// 	//好友分组信息请求
// 	IMChat::PeerCategoryRequest msgCategoryReq;
// 	msgCategoryReq.set_timestamp(nowtime);
// 	msgCategoryReq.set_userid(m_userData.user_id());
// 	m_codec.Send(m_ptrTcpConnection, msgCategoryReq);
	// 所有群号请求
	IMChat::GroupAllRequest groupReq;
	groupReq.set_userid(m_userData.user_id());
	groupReq.set_timestamp(nowtime);
	m_codec.Send(m_ptrTcpConnection, groupReq);
	//未读消息请求
	IMChat::UnreadMsgRequest unreadReq;
	unreadReq.set_userid(m_userData.user_id());
	unreadReq.set_timestamp(nowtime);
	m_codec.Send(m_ptrTcpConnection, unreadReq);
}

void CClientDlg::OnTcnSelchangingTabim(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;
	CRect tabRect;    // 标签控件客户区的Rect   

	// 获取标签控件客户区Rect，并对其调整，以适合放置标签页   
	m_tab.GetClientRect(&tabRect);
	tabRect.left += 1;
	tabRect.right -= 1;
	tabRect.top += 25;
	tabRect.bottom -= 1;
	switch (m_tab.GetCurSel())
	{
		// 如果标签控件当前选择标签为“Friends”，则显示m_dlgFriends对话框，隐藏m_dlgGroups对话框   
	case 1:
		m_dlgFriends.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		m_dlgGroups.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;
		// 如果标签控件当前选择标签为“Groups”，则隐藏m_dlgFriends对话框，显示m_dlgGroups对话框   
	case 0:
		m_dlgFriends.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_dlgGroups.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		break;
	default:
		break;
	}
}

LRESULT CClientDlg::OnWMDlgChat(WPARAM wParam, LPARAM lParam)
{

	USERDATA* pUserData = reinterpret_cast<USERDATA*>(lParam);
	if (NULL != pUserData)
	{

		//创建聊天窗口;
		std::shared_ptr<DlgChat> ptrChat(new DlgChat(*pUserData, m_userData));

		m_mapdlgChat[pUserData->user_id()] = ptrChat;//存储窗口智能指针;

		ptrChat->Create(IDD_DlgChat, this);//创建窗口，并传递this窗口指针;

		ptrChat->CenterWindow();//位置居中;

		ptrChat->ShowWindow(SW_SHOW);//显示窗口;
	}
	return 0;
}

LRESULT CClientDlg::OnWMDlgGroupChat(WPARAM wParam, LPARAM lParam)
{
	IMChat::GroupInfo* pGroupInfo = reinterpret_cast<IMChat::GroupInfo*>(lParam);
	if (NULL != pGroupInfo)
	{

		//创建聊天窗口;
		std::shared_ptr<DlgGroupChat> ptrChat(new DlgGroupChat(*pGroupInfo, m_userData));

		m_mapdlgGroupChat[pGroupInfo->groupid()] = ptrChat;//存储窗口智能指针;

		ptrChat->Create(IDD_DlgGroupChat, this);//创建窗口，并传递this窗口指针;

		ptrChat->CenterWindow();//位置居中;

		ptrChat->ShowWindow(SW_SHOW);//显示窗口;
		//查看当前是否已经有群成员信息;
		auto it = m_allGroupMember.find(pGroupInfo->groupid());
		if (it != m_allGroupMember.end())
			ptrChat->updateMemberInfo(it->second);
		//群成员请求;
		IMChat::GroupMembeRequest request;
		request.set_userid(m_userData.user_id());
		request.set_groupid(pGroupInfo->groupid());
		time_t nowtime;
		time(&nowtime);
		request.set_timestamp(static_cast<int64_t>(nowtime));
		m_codec.Send(m_ptrTcpConnection, request);
	}
	return 0;
}

LRESULT CClientDlg::OnWMCloseDlgChat(WPARAM wParam, LPARAM lParam)
{
	if (NULL != lParam)
	{
		const std::string* ptrId = reinterpret_cast<std::string*>(lParam);
		//销毁窗口指针;
		
		auto it = m_mapdlgChat.find(*ptrId);
		if (it != m_mapdlgChat.end())
		{
			it->second->DestroyWindow();
			m_mapdlgChat.erase(it);
		}
		ptrId = NULL;
	}
	return 0;
}

LRESULT CClientDlg::OnWMCloseDlgGroupChat(WPARAM wParam, LPARAM lParam)
{
	if (NULL != lParam)
	{
		const std::string* ptrId = reinterpret_cast<std::string*>(lParam);
		//销毁窗口指针;

		auto it = m_mapdlgGroupChat.find(*ptrId);
		if (it != m_mapdlgGroupChat.end())
		{
			it->second->DestroyWindow();
			m_mapdlgGroupChat.erase(it);
		}
		ptrId = NULL;
	}
	return 0;
}

LRESULT CClientDlg::OnWMChatMessage(WPARAM wParam, LPARAM lParam)
{
	std::string* pStrMessage = reinterpret_cast<std::string*>(wParam);
	std::string*    userId = reinterpret_cast<std::string*>(lParam);
	if (NULL != pStrMessage && NULL != userId)
	{
		time_t nowTime;
		time(&nowTime);
		IMChat::MsgData message;
		message.set_timestamp(nowTime);
		message.set_fromid(m_userData.user_id());
		message.set_msgdata(*pStrMessage);
		message.set_toid(*userId);
		message.set_type(ENUM_MSGTYPE::MsgData_MSGTYPE_CHAT_TEXT);

		m_codec.Send(m_ptrTcpConnection, message);
	}
	return 0;
}

LRESULT CClientDlg::OnWMGroupChatMessage(WPARAM wParam, LPARAM lParam)
{
	std::string* pStrMessage = reinterpret_cast<std::string*>(wParam);
	std::string*    userId = reinterpret_cast<std::string*>(lParam);
	if (NULL != pStrMessage && NULL != userId)
	{
		time_t nowTime;
		time(&nowTime);
		IMChat::MsgData message;
		message.set_timestamp(nowTime);
		message.set_fromid(m_userData.user_id());
		message.set_msgdata(*pStrMessage);
		message.set_toid(*userId);
		message.set_type(ENUM_MSGTYPE::MsgData_MSGTYPE_GROUPCHAT_TEXT);

		m_codec.Send(m_ptrTcpConnection, message);
	}
	return 0;
}

void CClientDlg::InitTestData()
{
// 	m_sessionId = "014006";
// 
// 	std::shared_ptr<USERDATA> ptrUser(new USERDATA());
// 	ptrUser->set_nick_name("Cplusplus");
// 	ptrUser->set_user_id("014006");
// 	m_listFriends.push_back(ptrUser);
// 	std::shared_ptr<USERDATA> ptrUser2(new USERDATA());
// 	ptrUser2->set_nick_name("Programing");
// 	ptrUser2->set_user_id("014007");
// 	m_listFriends.push_back(ptrUser2);
// 	std::shared_ptr<USERDATA> ptrUser3(new USERDATA());
// 	ptrUser3->set_nick_name("Computer Science");
// 	ptrUser3->set_user_id("014008");
// 	m_listFriends.push_back(ptrUser3);
// 	std::shared_ptr<USERDATA> ptrUser4(new USERDATA());
// 	ptrUser4->set_nick_name("Design");
// 	ptrUser4->set_user_id("014009");
// 	m_listFriends.push_back(ptrUser4);
// 	std::shared_ptr<USERDATA> ptrUser5(new USERDATA());
// 	ptrUser5->set_nick_name("hello world");
// 	ptrUser5->set_user_id("014010");
// 	m_listFriends.push_back(ptrUser5);
}
