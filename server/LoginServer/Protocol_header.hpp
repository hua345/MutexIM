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

//body_length  �������ĳ���
//type_id		�������ͺ�
//checknum		У���

//type for login
enum IM_TYPE
{
	LOGIN_REQ_MSGSERVER = 1,		//�����½
	LOGIN_RES_MSGSERVER = 2,		//��¼���󷵻�
	LOGIN_REQ_USERLOGIN = 3,		//��֤��½���󣬷����û���Ϣ
	LOGIN_RES_USERLOGIN = 4,		//��¼��֤���󷵻أ�������Կ

	//type for message

	MSG_DATA = 101,	//��Ϣ����
	MSG_DATA_ACK = 102, 	//��Ϣ������շ���
	MSG_READ_ACK = 103, 	//��Ϣ�����Ѷ�����
	MSG_UNREAD_MSG_REUQEST = 104,	//δ����Ϣ����
	MSG_UNREAD_MSG_RESPONSE = 105		//δ����Ϣ��Ӧ
};


// 	// command id for buddy list
// 
// 	CID_BUDDY_LIST_ONLINE_FRIEND_LIST_NOTIFY = 201,	//������ϵ��֪ͨ
// 	CID_BUDDY_LIST_STATUS_NOTIFY = 202,				//�û�״̬֪ͨ
// 	CID_BUDDY_LIST_USER_STATUS_REQUEST = 203,			//�����û�״̬���� 
// 	CID_BUDDY_LIST_USER_STATUS_RESPONSE = 204,		//�����û�״̬��Ӧ
// 	CID_BUDDY_LIST_USER_INFO_RESPONSE = 205,			//��ȡ�û���Ϣ���� 
// 	CID_BUDDY_LIST_USER_INFO_REQUEST = 206,			//��ȡ�û���Ϣ����
// 	CID_BUDDY_LIST_ALL_USER_REQUEST = 207,			//�����û���Ϣ����
// 	CID_BUDDY_LIST_ALL_USER_RESPONSE = 208,			//�����û���Ϣ���� 
// 	CID_BUDDY_LIST_USERS_STATUS_REQUEST = 209,		//�û��б�״̬����
// 	CID_BUDDY_LIST_USERS_STATUS_RESPONSE = 210,		//�û��б�״̬��Ӧ
// 	CID_BUDDY_LIST_CATEGORY_REQUEST = 211,			//������ѷ�����Ϣ
// 	CID_BUDDY_LIST_CATEGORY_RESPONSE = 212,			//���غ��ѷ�����Ϣ
// 
// 	// command id for group message
// 
// 	CID_GROUP_LIST_REQUEST = 301, 				// ��ȡ����Ⱥ������ get group id list for an user
// 	CID_GROUP_LIST_RESPONSE = 301,				// ��������Ⱥ����Ӧ return group id list for an user
// 	CID_GROUP_USER_LIST_REQUEST = 303,			// ��ȡȺ��Ա���� get user id list in a group
// 	CID_GROUP_USER_LIST_RESPONSE = 304,			// ����Ⱥ��Ա��Ӧ return user list in a group

class Protocol_header
{
public:
	static const uint32_t HEADER_LENGTH = 8;
	//����������ȡͷ��
	friend std::istream& operator>>(std::istream& is, Protocol_header& header)
	{
		return is.read(reinterpret_cast<char*>(header.m_header), 8);
	}
	//�������д��ͷ��
	friend std::ostream& operator<<(std::ostream& os, const Protocol_header& header)
	{
		return os.write(reinterpret_cast<const char*>(header.m_header), 8);
	}
	//Ĭ�ϳ�ʼ��
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
		m_header[a] = static_cast<uint8_t>(n >> 8);//16λ����8λ
		m_header[b] = static_cast<uint8_t>(n & 0xFF);//16λ����8λ
	}
	inline void encodeCheckSum(uint32_t n)
	{
		memcpy(m_header + 4, &n, sizeof(n));
// 		m_header[4] = static_cast<byte>(n >> 24);//32λ����8λ
// 		m_header[5] = static_cast<byte>(n >> 16);//32λ���и�8λ
// 		m_header[6] = static_cast<byte>(n >> 8); //32λ���е�8λ
// 		m_header[7] = static_cast<byte>(n & 0xFF);//32λ����8λ
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