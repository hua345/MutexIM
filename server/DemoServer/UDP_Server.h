#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include <iostream>
class UDP_Server
{
public:
	UDP_Server() = delete;
	UDP_Server(boost::asio::io_service& io);
	~UDP_Server();
	std::string make_daytime_string();
private:
	void Start_receive();
	//receive callback function
	void Handle_receive(const boost::system::error_code &e
		,size_t bytes_transferred);
	//send callback function
	void Handle_send(const boost::system::error_code &e
		, size_t bytes_transferred);
	//udp socket
	boost::asio::ip::udp::socket m_socket;
	//client address
	boost::asio::ip::udp::endpoint m_remote_endpoint;
	//receive buffer
	char m_recvBuf[1024];
};

