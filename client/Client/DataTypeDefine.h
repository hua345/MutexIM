#ifndef MYDATATYPEDEFINE_H
#define MYDATATYPEDEFINE_H
#include <map>
#include <list>
#include <string>
#include <memory>
#include <time.h>
#include "MessageChat.pb.h"

typedef  time_t TimeStamp;//时间戳;

typedef IMChat::MsgData_MSGTYPE ENUM_MSGTYPE;

typedef IMChat::UserData     USERDATA;
typedef std::list<std::shared_ptr<USERDATA>>   List_USERDATA;							//friends list
typedef List_USERDATA::iterator   List_USERDATA_It;

typedef std::list<IMChat::GroupMemberInfo> List_GroupMember;					//group member list
typedef List_GroupMember::iterator List_GroupMember_It;

typedef std::list<std::shared_ptr<IMChat::GroupInfo>> List_GroupInfo;							//group info list
typedef List_GroupInfo::iterator List_GroupInfo_It;

typedef std::map<std::string, std::shared_ptr<List_GroupMember>> MAP_GROUPMEMBER;//std::pair<group id, member list>
typedef MAP_GROUPMEMBER::iterator  MAP_GROUP_It;

typedef std::list<std::shared_ptr<IMChat::MsgData>> List_MSGDATA;   //消息列表;

typedef List_MSGDATA::iterator  LIST_MSGDATA_IT;

typedef std::map<std::string, std::shared_ptr<IMChat::MsgData>> MAP_MSGDATA;//first->userId或者groupId,second->msg;

typedef MAP_MSGDATA::iterator   MAP_MSGDATA_IT;
#endif