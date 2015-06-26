#ifndef DEMOPROTOCOL_HEADER_HPP
#define DEMOPROTOCOL_HEADER_HPP

#include <istream>
#include <ostream>
#include <algorithm>
#include <stdint.h>
// The wire format of an protocol header is:
// 
// 0               8               16                             31
// +---------------+---------------+------------------------------+      ---
// |                               |                              |       ^
// |		    type_id		       |            body_length       |       |
// |                               |                              |       |
// +---------------+---------------+------------------------------+     8 bytes
// |                                                              |       |
// |							checkSum                          |       |
// |                                                              |       v
// +-------------------------------+------------------------------+		 ---

//body_length  整个包的长度
//type_id		服务类型号
//checknum		校验和

//type for login
enum IM_TYPE
{
	LOGIN_REQ_MSGSERVER = 1,		//请求登陆
	LOGIN_RES_MSGSERVER = 2,		//登录请求返回
	LOGIN_REQ_USERLOGIN = 3,		//验证登陆请求，发送用户信息
	LOGIN_RES_USERLOGIN = 4,		//登录验证请求返回，交换密钥

	//type for message

	MSG_DATA = 101,	//消息信令
	MSG_DATA_ACK = 102, 	//消息信令接收反馈
	MSG_READ_ACK = 103, 	//消息信令已读反馈
	MSG_UNREAD_MSG_REUQEST = 104,	//未读消息请求
	MSG_UNREAD_MSG_RESPONSE = 105		//未读消息响应
};


// 	// command id for buddy list
// 
// 	CID_BUDDY_LIST_ONLINE_FRIEND_LIST_NOTIFY = 201,	//在线联系人通知
// 	CID_BUDDY_LIST_STATUS_NOTIFY = 202,				//用户状态通知
// 	CID_BUDDY_LIST_USER_STATUS_REQUEST = 203,			//单个用户状态请求 
// 	CID_BUDDY_LIST_USER_STATUS_RESPONSE = 204,		//单个用户状态响应
// 	CID_BUDDY_LIST_USER_INFO_RESPONSE = 205,			//获取用户信息返回 
// 	CID_BUDDY_LIST_USER_INFO_REQUEST = 206,			//获取用户信息请求
// 	CID_BUDDY_LIST_ALL_USER_REQUEST = 207,			//所有用户信息请求
// 	CID_BUDDY_LIST_ALL_USER_RESPONSE = 208,			//所有用户信息返回 
// 	CID_BUDDY_LIST_USERS_STATUS_REQUEST = 209,		//用户列表状态请求
// 	CID_BUDDY_LIST_USERS_STATUS_RESPONSE = 210,		//用户列表状态响应
// 	CID_BUDDY_LIST_CATEGORY_REQUEST = 211,			//请求好友分组信息
// 	CID_BUDDY_LIST_CATEGORY_RESPONSE = 212,			//返回好友分组信息
// 
// 	// command id for group message
// 
// 	CID_GROUP_LIST_REQUEST = 301, 				// 获取所有群号请求 get group id list for an user
// 	CID_GROUP_LIST_RESPONSE = 301,				// 返回所有群号响应 return group id list for an user
// 	CID_GROUP_USER_LIST_REQUEST = 303,			// 获取群成员请求 get user id list in a group
// 	CID_GROUP_USER_LIST_RESPONSE = 304,			// 返回群成员响应 return user list in a group

class Protocol_header
{
public:
	static const uint32_t HEADER_LENGTH = 8;
	//从输入流读取头部
	friend std::istream& operator>>(std::istream& is, Protocol_header& header)
	{
		return is.read(reinterpret_cast<char*>(header.m_header), 8);
	}
	//向输出流写入头部
	friend std::ostream& operator<<(std::ostream& os, const Protocol_header& header)
	{
		return os.write(reinterpret_cast<const char*>(header.m_header), 8);
	}
	//默认初始化
	Protocol_header() {
		memset(m_header, 0, HEADER_LENGTH);
	}
	unsigned char* GetHeaderRef(){ return m_header; }

	IM_TYPE getType() const { return static_cast<IM_TYPE>(decode(0, 1)); }
	uint16_t getBodyLength() const { return decode(2, 3); }
	uint32_t getCheckSum() const { return decodeCheckSum(); }
	//set header
	void setType(uint16_t n)  { encode(0, 1, n); }
	void setBodyLength(uint16_t n)  { encode(2, 3, n); }
	void setCheckSum(uint32_t n)  { encodeCheckSum(n); }
private:
	inline uint16_t decode(int a, int b) const
	{
		return static_cast<uint16_t>(m_header[a] << 8) + m_header[b];
	}

	inline void encode(int a, int b, uint16_t n)
	{
		m_header[a] = static_cast<uint8_t>(n >> 8);//16位数高8位
		m_header[b] = static_cast<uint8_t>(n & 0xFF);//16位数低8位
	}
	inline void encodeCheckSum(uint32_t n)
	{
		memcpy(m_header + 4, &n, sizeof(n));
// 		m_header[4] = static_cast<byte>(n >> 24);//32位数高8位
// 		m_header[5] = static_cast<byte>(n >> 16);//32位数中高8位
// 		m_header[6] = static_cast<byte>(n >> 8); //32位数中低8位
// 		m_header[7] = static_cast<byte>(n & 0xFF);//32位数低8位
	}
	inline uint32_t decodeCheckSum() const
	{
		uint32_t temp;
		memcpy(&temp, m_header + 4, sizeof(temp));
		return temp;
// 		return static_cast<uint32_t>(m_header[4] << 24)
// 			+ static_cast<uint32_t>(m_header[5] << 16)
// 			+ static_cast<uint32_t>(m_header[6] << 8)
// 			+ m_header[7];
	}
	
	uint8_t m_header[8];
};
template <typename Iterator>
void compute_checksum(Protocol_header& header,
	Iterator body_begin, Iterator body_end)
{
	unsigned int sum = (header.version() << 8) + (header.type_id() << 8)
		+ header.command_id() + header.total_length()
		+ header.identifier() + header.sequence_number();

	Iterator body_iter = body_begin;
	while (body_iter != body_end)
	{
		sum += (static_cast<unsigned char>(*body_iter++) << 8);
		if (body_iter != body_end)
			sum += static_cast<unsigned char>(*body_iter++);
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	header.checksum(static_cast<uint16_t>(~sum));
}
#endif // ICMP_HEADER_HPP