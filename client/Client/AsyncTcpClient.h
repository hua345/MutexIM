#ifndef ASYNC_TCP_CLIENT_HPP
#define ASYNC_TCP_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <stdint.h>
#include <queue>
#include "Message.pb.h"
#include <boost/asio/streambuf.hpp>
#include "MyBuffer.h"
static uint32_t g_send_pkt_cnt = 0;		// �������ݰ�����
static uint32_t g_recv_pkt_cnt = 0;		// �������ݰ�����
typedef std::shared_ptr<google::protobuf::Message> shared_ptr_Msg;//����ָ��

class AsyncTcpClient;
#define WM_ASIO_MESSAGE (WM_USER + 1)
typedef std::shared_ptr<AsyncTcpClient> TcpConnectionPtr;

class AsyncTcpClient:public std::enable_shared_from_this<AsyncTcpClient>
{
public:
	typedef boost::asio::streambuf  StreamBuf;
	typedef std::function<void(const boost::asio::ip::tcp::endpoint&)> ConnectedCallback;
	typedef std::function<void(const boost::asio::ip::tcp::endpoint&)> DisconnectedCallback;
	typedef std::function<void(const TcpConnectionPtr&, const std::string&)> MessageCallback;//when have message
	explicit AsyncTcpClient(boost::asio::io_service& service, CWnd* ptrWnd);
	~AsyncTcpClient();
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
	/**
	* @brief Connect to server
	* @param[in] endpoint .
	*/
	void Connect(const boost::asio::ip::tcp::endpoint& endpoint);
	/**
	* @brief Send message to server
	* @param[in] msg Want to send a message.
	*/
	void Send(std::shared_ptr<MyBuffer> ptrBuffer);
	//connect operate
	void Reconnect(const boost::system::error_code& error);
	/**
	* @brief Close network connection, you can call connect function
	*        to reconnect.
	*/
	void Disconnect();
	//set message callback
	void setMessageCallback(const MessageCallback& callback)
	{
		messageCallback_ = callback;
	}
	void setConnectionCallback(const ConnectedCallback& callback)
	{
		ConnectedCallback_ = callback;
	}
	void setDisconnectedCallback(const DisconnectedCallback& callback)
	{
		DisconnectedCallback_ = callback;
	}
private:
	void handle_connect(const boost::system::error_code& error);
	//read && write
	void read();
	//handle read
	void handle_read(const boost::system::error_code& error
		, uint32_t byte_transferred);
	//write callback function
	void handle_write(const boost::system::error_code& error,
		uint32_t bytes_transferred);
	void close();
	void do_close();
private:
	CWnd*						m_ptrWnd;		//����ָ��
	boost::asio::io_service&	m_service;		//�첽���������
	boost::asio::ip::tcp::socket m_socket;		//����Socket�׽���
	boost::asio::ip::tcp::endpoint	m_endPoint;	//Զ���׽���

	MessageCallback messageCallback_;			//��ȡ���ݺ����Ϣ�ص�
	ConnectedCallback ConnectedCallback_;		//���ӻص�
	DisconnectedCallback DisconnectedCallback_; //�Ͽ��ص�

	std::string m_delimiter;       
private:
	//connected state
	bool m_isConnected;//�ж��Ƿ�����
	//recvive buffer
	StreamBuf m_buffer;
	//��Ϣ����
	std::queue<std::shared_ptr<MyBuffer>> m_sendMessages;
};

#endif 