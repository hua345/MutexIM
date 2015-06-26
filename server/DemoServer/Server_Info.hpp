#ifndef SERVER_INFO_HPP
#define SERVER_INFO_HPP

#include <string>
#include <stdint.h>
#include <list>
#include <memory>
typedef struct
{
	std::string m_address;	//µØÖ·
	uint16_t m_port;		//¼àÌý¶Ë¿Ú
}Struct_info;

class Server_Info
{
public:
	Server_Info()
	{
		//login server
		m_loginServer.m_address = "127.0.0.1";
		m_loginServer.m_port = 8080;
		//message server
		auto ptr_server = std::make_shared<Struct_info>();
		ptr_server->m_address = "127.0.0.1";
		ptr_server->m_port = 8081;
		m_list_msgServer.push_back(ptr_server);

		ptr_server = std::make_shared<Struct_info>();
		ptr_server->m_address = "127.0.0.1";
		ptr_server->m_port = 8082;
		m_list_msgServer.push_back(ptr_server);
		//index
		m_itMsgServer = m_list_msgServer.begin();
	};
	~Server_Info(){};


	const std::list<std::shared_ptr<Struct_info>>& get_list_msgServer(){ return m_list_msgServer; }
	const Struct_info& get_loginServer(){ return m_loginServer; }
	Struct_info get_freeMsgServer()
	{
		if (m_list_msgServer.end() != m_itMsgServer)
		{
			const Struct_info& server = **m_itMsgServer;
			++m_itMsgServer;
			return server;
		}
		else
		{
			m_itMsgServer = m_list_msgServer.begin();
			const Struct_info& server = **m_itMsgServer;
			++m_itMsgServer;
			return server;
		}
	}
private:
	std::list<std::shared_ptr<Struct_info>> m_list_msgServer;
	
	Struct_info m_loginServer;

	std::list<std::shared_ptr<Struct_info>>::iterator m_itMsgServer;
};

#endif	//SERVER_INFO_HPP