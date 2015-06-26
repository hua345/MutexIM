#ifndef SYNC_TCPCLIENT_HPP
#define SYNC_TCPCLIENT_HPP

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <google/protobuf/message.h>
#include "MyBuffer.h"

#pragma comment(lib, "libeay32.lib")  
#pragma comment(lib, "ssleay32.lib")  
class SSL_TcpClient
{
	typedef boost::asio::streambuf  StreamBuf;
	typedef std::function<void(const std::string& )> MessageCallback;
public:
	 SSL_TcpClient();
	~SSL_TcpClient();
	/**
	* @brief Connect to server
	* @param[in] ip Server ip address like "192.168.10.26".
	* @param[in] port Server listening port for example 8080.
	*/
	void Connect(const std::string& address, uint16_t port);
	/**
	* @brief Connect to server
	* @param[in] url Server url like "www.google.com".
	* @param[in] protocol Connection protocol like "http".
	*/
	void Connect(const std::string &url, const std::string &protocol);
	void Disconnect();
	/**
	* @brief Send message to server
	* @param[in] msg Want to send a message.
	*/
	void Send(std::shared_ptr<MyBuffer> ptrBuffer);
	void SyncRead();
	void setMessageCallback(const MessageCallback& callback)
	{
		messageCallback_ = callback;
	}
private:
	std::string getPassword();
	boost::asio::io_service m_service;
	boost::asio::ssl::context m_context;		//SSL������
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket>   m_socket;		//����SSL Socket�׽���
	boost::asio::ip::tcp::endpoint	m_endPoint;	//Զ���׽���


	std::string m_delimiter;

	MessageCallback messageCallback_;	//���յ���Ϣ��ص�����
private:
	//connected state
	bool m_isConnected;//�ж��Ƿ�����
	//recvive buffer
	StreamBuf m_recvBuffer;
};

#endif	//SYNC_TCPCLIENT_HPP