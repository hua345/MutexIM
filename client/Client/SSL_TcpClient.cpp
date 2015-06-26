#include "stdafx.h"
#include "SSL_TcpClient.h"

SSL_TcpClient::SSL_TcpClient()
: m_service()
, m_context(boost::asio::ssl::context::sslv23)
, m_socket(m_service,m_context)
, m_isConnected(false)
, m_delimiter("\r\n")
{
	m_context.set_password_callback(std::bind(&SSL_TcpClient::getPassword, this));
	m_context.load_verify_file("ca.crt");
	m_socket.set_verify_mode(boost::asio::ssl::verify_peer);
}


SSL_TcpClient::~SSL_TcpClient()
{
	/*		google::protobuf::ShutdownProtobufLibrary();*/
	Disconnect();
}

void SSL_TcpClient::Connect(const std::string& address, uint16_t port)
{
	boost::asio::ip::tcp::endpoint m_endpoint(
		boost::asio::ip::address::from_string(address), port);

	m_socket.lowest_layer().connect(m_endpoint);//SSL context lowest layer
	m_socket.handshake(boost::asio::ssl::stream_base::client);	//SSL handshake with server
}

void SSL_TcpClient::Connect(const std::string &url, const std::string &protocol)
{
	boost::asio::ip::tcp::resolver::query query(url, protocol);
	boost::asio::ip::tcp::resolver resolver(m_service);
	boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

	boost::asio::connect(m_socket.lowest_layer(), it);//SSL context lowest layer
	m_socket.handshake(boost::asio::ssl::stream_base::client);	//SSL handshake with server
}


void SSL_TcpClient::Send(std::shared_ptr<MyBuffer> ptrBuffer)
{
	boost::asio::write(m_socket, boost::asio::buffer(ptrBuffer->peek(), ptrBuffer->readableBytes()));
}

void SSL_TcpClient::SyncRead()
{
	size_t bytes_transferred = boost::asio::read_until(m_socket, m_recvBuffer, m_delimiter);
	// TODO: you could do some message processing here, like breaking it up
	// into smaller parts, rejecting unknown messages or handling the message protocol
	std::string message(boost::asio::buffer_cast<const char*>(m_recvBuffer.data()), bytes_transferred);

	m_recvBuffer.consume(bytes_transferred);
	// create function to notify listeners
	messageCallback_(message);
}

std::string SSL_TcpClient::getPassword()
{
	return "client_encrypto";
}

void SSL_TcpClient::Disconnect()
{
	boost::system::error_code err;
	m_socket.shutdown(err);

	if (0 != err)
	{
		//Log...
	}
	m_isConnected = false;
}
