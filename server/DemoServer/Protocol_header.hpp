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
// |               |               |                              |       ^
// |    version    |   type_id     |          command_id          |       |
// |               |               |                              |       |
// +---------------+---------------+------------------------------+     8 bytes
// |                               |                              |       |
// |          body_length          |       sequence number        |       |
// |                               |                              |       v
// +-------------------------------+------------------------------+		 ---

//version		Э��汾,1byte��������1
//type_id		�������ͺ�
//checknum		У���
//command_id	��Ϣ�����
//total_length  �������ĳ���
//identification ����ʶ��ǰ���ݱ�
//sequence number ���к�
//type_id		�������ͺ�

class Protocol_header
{
public:
	enum  {
		MSG_TYPE_LOGIN = 0x00,						//for login
		MSG_TYPE_BUDDY_LIST = 0x01,				//for friend list
		MSG_TYPE_MESSAGE = 0x02,					//message
		MSG_TYPE_SWITCH_SERVICE = 0x03,			//for switch service
		MSG_TYPE_GROUP = 0x04,						//for group message
		MSG_TYPE_FILE = 0x05,					//for file
		MSG_TYPE_OTHER = 0x06					//empty for init
	};
	//command id for login
	enum  {
		CID_LOGIN_REQ_MSGSERVER = 1, 	//�����½
		CID_LOGIN_RES_MSGSERVER = 2,	//��¼���󷵻�
		CID_LOGIN_REQ_USERLOGIN = 3,	//��֤��½���󣬷����û���Ϣ
		CID_LOGIN_RES_USERLOGIN = 4,	//��¼��֤���󷵻أ�������Կ
	};
	// command id for message
	enum {
		CID_MSG_DATA = 1,				//��Ϣ����
		CID_MSG_DATA_ACK = 2, 			//��Ϣ������շ���
		CID_MSG_READ_ACK = 3, 			//��Ϣ�����Ѷ�����
		CID_MSG_UNREAD_MSG_REUQEST = 4,	//δ����Ϣ����
		CID_MSG_UNREAD_MSG_RESPONSE = 5,//δ����Ϣ��Ӧ
	};
	// command id for buddy list
	enum {
		CID_BUDDY_LIST_ONLINE_FRIEND_LIST_NOTIFY = 1,	//������ϵ��֪ͨ
		CID_BUDDY_LIST_STATUS_NOTIFY = 2,				//�û�״̬֪ͨ
		CID_BUDDY_LIST_USER_STATUS_REQUEST = 3,			//�����û�״̬���� 
		CID_BUDDY_LIST_USER_STATUS_RESPONSE = 4,		//�����û�״̬��Ӧ
		CID_BUDDY_LIST_USER_INFO_RESPONSE = 5,			//��ȡ�û���Ϣ���� 
		CID_BUDDY_LIST_USER_INFO_REQUEST = 6,			//��ȡ�û���Ϣ����
		CID_BUDDY_LIST_ALL_USER_REQUEST = 7,			//�����û���Ϣ����
		CID_BUDDY_LIST_ALL_USER_RESPONSE = 8,			//�����û���Ϣ���� 
		CID_BUDDY_LIST_USERS_STATUS_REQUEST = 9,		//�û��б�״̬����
		CID_BUDDY_LIST_USERS_STATUS_RESPONSE = 10,		//�û��б�״̬��Ӧ
		CID_BUDDY_LIST_CATEGORY_REQUEST = 11,			//������ѷ�����Ϣ
		CID_BUDDY_LIST_CATEGORY_RESPONSE = 12,			//���غ��ѷ�����Ϣ
	};
	// command id for group message
	enum {
		CID_GROUP_LIST_REQUEST = 1, 				// ��ȡ����Ⱥ������ get group id list for an user
		CID_GROUP_LIST_RESPONSE = 2,				// ��������Ⱥ����Ӧ return group id list for an user
		CID_GROUP_USER_LIST_REQUEST = 3,			// ��ȡȺ��Ա���� get user id list in a group
		CID_GROUP_USER_LIST_RESPONSE = 4,			// ����Ⱥ��Ա��Ӧ return user list in a group
	};
	static const uint32_t HEADER_LENGTH = 8;

	friend std::istream& operator>>(std::istream& is, Protocol_header& header)
	{
		return is.read(reinterpret_cast<char*>(header.m_header), 8);
	}

	friend std::ostream& operator<<(std::ostream& os, const Protocol_header& header)
	{
		return os.write(reinterpret_cast<const char*>(header.m_header), 8);
	}

	Protocol_header() {
		memset(m_header, 0, HEADER_LENGTH);
		m_header[0] = 'a';//Ĭ�ϰ汾��a
		m_header[1] = static_cast<uint8_t>(MSG_TYPE_OTHER);//TYPEĬ������
	}
	unsigned char* GetHeaderRef(){ return m_header; }
	//get header 
	uint8_t get_version() const { return m_header[0]; }
	uint8_t get_type_id() const { return m_header[1]; }
	uint16_t get_command_id() const { return decode(2, 3); }
	uint16_t get_body_length() const { return decode(4, 5); }
	uint16_t get_identifier() const { return decode(6, 7); }
	//set header
	void set_version(uint8_t n)  { m_header[0] = n; }
	void set_type_id(uint8_t n)  { m_header[1] = n; }
	void set_command_id(uint16_t n)  { encode(2, 3, n); }
	void set_body_length(uint16_t n)  { encode(4, 5, n); }
	void set_identifier(uint16_t n)  { encode(6, 7, n); }
private:
	uint16_t decode(int a, int b) const
	{
		return static_cast<uint16_t>(m_header[a] << 8) + m_header[b];
	}

	void encode(int a, int b, uint16_t n)
	{
		m_header[a] = static_cast<uint8_t>(n >> 8);//16λ����8λ
		m_header[b] = static_cast<uint8_t>(n & 0xFF);//16λ����8λ
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