#ifndef STRUCTDATA_HPP
#define STRUCTDATA_HPP
#include <string>
#include <stdint.h>
struct UseInfoStruct
{
	 int64_t        timeTamp;	//��¼ʱ���
	 std::string	user_id ;		//�û�ID
	 std::string	name ;			//�û���
	 std::string	nick_name ;		//�ǳ�
	 std::string	image_url ;		//ͷ��
	 std::string	position ;     	//����
	 std::string	email ;        	//����
	 unsigned int  sex = 0;			//�Ա�
};

#endif