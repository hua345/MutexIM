// DemoServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "Server.h"
#include <signal.h>

BOOL WINAPI ConsoleHandler(DWORD msgType);
//事件循环;
boost::asio::io_service g_service_;

void exitFunc(){
	g_service_.stop();
};

int main(int argc, char* argv[])
{
	//注册控制台事件;
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	// Verify that the version of the library that we linked against is   
	// compatible with the version of the headers we compiled against.   
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	try
	{	
		if (argc != 3)
		{
			std::cerr << "Usage: chat_server <host> <port>\n";
			system("PAUSE");
			return -1;
		}
		Server server_(g_service_, argv[1], argv[2]);

// 		string address("127.0.0.1");
// 		string port("8081");
// 		Server server_(g_service_, address, port);
		


		g_service_.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	// Optional:  Delete all global objects allocated by libprotobuf.   
	google::protobuf::ShutdownProtobufLibrary();

	system("PAUSE");
	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD msgType)
{
	if (msgType == CTRL_C_EVENT)
	{
		printf("Ctrl-C!\n");
		exitFunc();
		printf("exit success!\n");
		return TRUE;
	}
	else if (msgType == CTRL_CLOSE_EVENT)
	{
		printf("Close console window!\n");

		exitFunc();
		/* Note: The system gives you very limited time to exit in this condition */
		return TRUE;
	}
	return TRUE;
	/*
	Other messages:
	CTRL_BREAK_EVENT         Ctrl-Break pressed
	CTRL_LOGOFF_EVENT        User log off
	CTRL_SHUTDOWN_EVENT      System shutdown
	*/
}