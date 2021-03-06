//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ 生成的包含文件。
// 供 Client.rc 使用
//
#define IDM_ABOUTBOX                    0x0010
#define IDD_ABOUTBOX                    100
#define IDS_ABOUTBOX                    101
#define IDD_CLIENT_DIALOG               102
#define IDR_MAINFRAME                   128
#define IDD_DlgLogin                    129
#define IDB_Friends_Sel                 133
#define IDB_Friends_Sel2                134
#define IDB_Friends_unSel               135
#define IDD_DlgFriends                  136
#define IDD_DlgGroups                   137
#define IDD_DlgChat                     138
#define IDD_DIALOG1                     139
#define IDD_DlgGroupChat                139
#define IDR_MENU1                       140
#define IDC_UserName                    1000
#define IDC_Password                    1001
#define IDC_BtnLogin                    1002
#define IDC_TabIM                       1005
#define IDC_LIST1                       1007
#define IDC_ListFriends                 1007
#define IDC_ListMember                  1007
#define IDC_LISTGroups                  1007
#define IDC_EditChat                    1008
#define IDC_EditChatLog                 1011
#define IDC_BtnSend                     1012
#define IDC_EditGroupChatLog            1013
#define IDC_EditGroupChat               1014
#define ID_SubMenuDisplay               1124
#define ID_SubMenuExit                  1125

//User Define Message
#define WM_USE_DlgCHAT             WM_USER + 200		//打开私聊窗口;
#define WM_USE_DlgGROUPCHAT        WM_USER + 201		//打开群聊窗口;
#define WM_USE_CloseDlgChat		   WM_USER + 202		//关闭私聊窗口;
#define WM_USE_CloseDlgGroupChat   WM_USER + 203		//关闭群聊窗口;
#define WM_USE_LoginData		   WM_USER + 204		//登录时向主窗口传递数据;

#define WM_USE_ChatMessage		   WM_USER + 300		//发送聊天消息;
#define WM_USE_GROUPCHATMESSAGE	   WM_USER + 301		//发送群聊消息;
#define WM_USE_FILES				WM_USER + 302		//发送文件;
#define WM_USE_VIDEO				WM_USER + 303       //视频聊天;
#define WM_USE_AUDIO				WM_USER + 304		//音频聊天;
#define WM_USE_RECVMESSAGE          WM_USER + 305       //向子窗口发送已接收消息;

#define WM_TRAYNOTIFY              WM_USER  + 400   //托盘图标;
#define WM_USE_UPDATE_DATA			WM_USER + 500   //UpdateData ,MFC对象不能跨线程访问造成的
// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        141
#define _APS_NEXT_COMMAND_VALUE         32779
#define _APS_NEXT_CONTROL_VALUE         1015
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
