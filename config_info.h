// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#ifndef ZSNIFFER_CONFIG_INFO_H_
#define ZSNIFFER_CONFIG_INFO_H_
#pragma once

#include <string>
#include <vector>
#include <map>

#include "base/singleton.h"

class ConfigInfo : public Singleton<ConfigInfo> {
public:
	int Initialize(const char* ini_file);
	void Destroy();

	inline std::vector<std::string>& map_paths() {
		return map_paths_;
	}

	inline const std::string& message_type_proto_file() {
		return message_type_proto_file_;
	}

	inline const std::string& cap_port() {
		return cap_port_;
	}

	inline const std::string& encoding() {
		return encoding_;
	}

private:
	std::vector<std::string> map_paths_;
	std::string message_type_proto_file_;
	std::string cap_port_;
	std::string encoding_;
	// std::string comm_message_proto_path_;
};

#endif
