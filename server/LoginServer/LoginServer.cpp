// LoginServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"

int main(int argc, char* argv[])
{
	// Verify that the version of the library that we linked against is   
	// compatible with the version of the headers we compiled against.   
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	try
	{
 		boost::asio::io_service m_ioservice;
		if (argc != 3)
		{
			std::cerr << "Usage: chat_server <host> <port>\n";
			system("PAUSE");
			return -1;
		}
		Server server_(m_ioservice, argv[1], argv[2]);

// 		string serverAddress("127.0.0.1");
// 		string serverPort("8080");
// 		Server server_(m_ioservice, serverAddress, serverPort);


		m_ioservice.run();
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
