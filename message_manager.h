// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//
// 1.从const_message_types.proto文件读取消息类型定义
// 2.获取消息类型和消息类型值
// 3.缓存待以后使用

#ifndef ZSNIFFER_MESSAGE_MANAGER_H_
#define ZSNIFFER_MESSAGE_MANAGER_H_
#pragma once

#include <string>
#include <map>

#include "base/basictypes.h"
#include "base/singleton.h"

#include "zsniffer/pb_message_factory.h"

struct MessageInfo {
	MessageInfo()
		: is_empty(true) {
	}

	std::string message_type;
	// uint16 message_type_val;
	std::string message_class_name;
	std::string message_proto_file;
	bool is_empty;
};

class MessageManager : public Singleton<MessageManager> {
public:
	bool Initialize(const std::vector<std::string>& map_paths, const std::string& message_types_proto);

	const MessageInfo* FindMessageInfo(uint16 message_type);
	google::protobuf::Message* CreateMessage(uint16 message_type);
	

private:
	friend class Singleton<MessageManager>;

	MessageManager() {}
	~MessageManager() {}

	std::map<uint16, MessageInfo> message_types_;

	PBMessageFactory message_factory_;


/*
	std::vector<std::string> map_paths;
	xml_ini.GetStrings("MapPath", "Path", map_paths);
	message_factory.MapPaths(map_paths);
 */

};

#endif
