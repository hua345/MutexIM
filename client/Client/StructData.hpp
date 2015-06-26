#ifndef STRUCTDATA_HPP
#define STRUCTDATA_HPP
#include <string>
#include <stdint.h>
struct UseInfoStruct
{
	 int64_t        timeTamp;	//登录时间戳
	 std::string	user_id ;		//用户ID
	 std::string	name ;			//用户名
	 std::string	nick_name ;		//昵称
	 std::string	image_url ;		//头像
	 std::string	position ;     	//地区
	 std::string	email ;        	//邮箱
	 unsigned int  sex = 0;			//性别
};

#endif