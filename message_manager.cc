// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "zsniffer/message_manager.h"

#include <stdio.h>

#include <iostream>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "zsniffer/config_info.h"

namespace {

std::string ReadLine(FILE* fp) {
	if (!fp)
		return "";

	std::string result = "";
	const size_t kBufferSize = 100;
	char buffer[kBufferSize];
	while ((fgets(buffer, kBufferSize - 1, fp)) != NULL) {
		result += buffer;
		size_t len = result.length();
		if (len == 0)
			break;
		char end = result[len - 1];
		if (end == '\n' || end == '\0')
			break;
	}

	return result;
}

// ASCII-specific tolower.  The standard library's tolower is locale sensitive,
// so we don't want to use it here.
template <class Char> inline Char ToLowerASCII(Char c) {
	return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str> inline void StringToLowerASCII(str* s) {
	for (typename str::iterator i = s->begin(); i != s->end(); ++i)
		*i = ToLowerASCII(*i);
}

template <class str> inline str StringToLowerASCII(const str& s) {
	// for std::string and std::wstring
	str output(s);
	StringToLowerASCII(&output);
	return output;
}

bool GenMessageInfoByMessageType(const std::string& message_type, MessageInfo* message_info) {
	std::vector<std::string> tokens;
	boost::split(tokens, message_type, boost::is_any_of("_"));

	if (tokens.size()<=1) {
		return false;
	}

	message_info->message_type = message_type;

	message_info->message_proto_file = StringToLowerASCII(tokens[1]);
	size_t i = 2;
	for (; i<tokens.size(); ++i) {
		message_info->message_proto_file += "_";
		message_info->message_proto_file += StringToLowerASCII(tokens[i]);
	}
	message_info->message_proto_file += ".proto";

	//script_file_name += "handler.lua";

	i = 1;
	message_info->message_class_name = tokens[i++];
	for (; i<tokens.size(); ++i) {
		std::string tmp;
		message_info->message_class_name.push_back(tokens[i][0]);
		tmp = StringToLowerASCII(tokens[i]);
		message_info->message_class_name += tmp.substr(1, tmp.length()-1);
	}

	return true;
}

}

bool MessageManager::Initialize(const std::vector<std::string>& map_paths, const std::string& message_types_proto) {
	std::string message_types_pb_path;
	std::map<std::string, std::string> proto_files; 

	for (size_t i=0; i<map_paths.size(); ++i) {
		boost::filesystem::path data_path(map_paths[i]);
		if (!boost::filesystem::exists(data_path)) {
			std::cerr << "file path [" << map_paths[i] << "] not exist." << std::endl;
			return false;
			// continue;
		}
		if (!boost::filesystem::is_directory(data_path)) {
			std::cerr << "file path [" << map_paths[i] << "] is not directory." << std::endl;
			return false;
			// continue;
		}

		boost::filesystem::directory_iterator end_itr;
		for (boost::filesystem::directory_iterator itr(data_path); itr != end_itr; ++itr) {
			if (!strnicmp(itr->path().extension().c_str(), ".proto", 6)) {
				if (message_types_proto == itr->path().filename()) {
					message_types_pb_path = itr->path().file_string();
				}
				proto_files.insert(std::make_pair(itr->path().filename(), itr->path().file_string()));
			}
		}
	}

	if (message_types_pb_path.empty()) {
		std::cerr << "Can't find the " << message_types_proto << std::endl;
		return false;
	}

	message_factory_.MapPaths(map_paths);

	FILE* fp = fopen(message_types_pb_path.c_str(), "r");
	if (!fp) {
		std::cout << "can't open " << message_types_pb_path << std::endl;
		return false;
	}

	bool state = false;

	int code_type = 0;

	// Read entries.
	while (!feof(fp) && !ferror(fp)) {
		std::string buf = ReadLine(fp);
		if (buf == "")
			break;

		std::string line2 = buf;
		boost::trim(line2);
		// TrimWhitespaceASCII(buf, TRIM_ALL, &line2);
		if (line2.empty() || line2[0]=='/') {
			continue;
		}
		if (!boost::starts_with(line2, "MESSAGE_")) {
			continue;
		}

		std::string message_type;
		std::string message_type_val;

		size_t i;
		for (i=0; i<line2.length(); ++i) {
			if (line2[i]==' ' || line2[i]=='=' || line2[i]=='\t') {
				break;
			} else {
				message_type.push_back(line2[i]);
			}
		}

		for (; i<line2.length(); ++i) {
			if (line2[i] == ';') {
				break;
			}

			if (line2[i]==' ' || line2[i]=='\t' || line2[i]=='=') {
			} else {
				message_type_val.push_back(line2[i]);
			}
		}

		MessageInfo message_info;
		if(GenMessageInfoByMessageType(message_type, &message_info)) {
			std::map<std::string, std::string>::iterator it = proto_files.find(message_info.message_proto_file);
			if (it != proto_files.end()) {
				message_info.is_empty = false;
				// std::cerr << "Not invalid message type [" << message_info.message_type << "]" << std::endl;
				// continue;
			}
			// message_info.message_proto_file = it->second;			
			// std::cout << message_type << " ==> [" <<  message_type_val << ", " << message_info.message_proto_file << ", " << message_info.message_class_name << "]" << std::endl;
			message_types_.insert(std::make_pair(boost::lexical_cast<uint16>(message_type_val), message_info));

		}
	}
	
	fclose(fp);

	return true;
}

const MessageInfo* MessageManager::FindMessageInfo(uint16 message_type) {
	MessageInfo* message_info = NULL;

	std::map<uint16, MessageInfo>::iterator it = message_types_.find(message_type);
	if (it != message_types_.end()) {
		message_info = &(it->second);
	}

	return message_info;
}

google::protobuf::Message* MessageManager::CreateMessage(uint16 message_type) {
	google::protobuf::Message* message = NULL;

	const MessageInfo* message_info = FindMessageInfo(message_type);
	if (message_info && !message_info->is_empty) {
		message = message_factory_.CreateMessage(message_info->message_proto_file, message_info->message_class_name);
	}

	return message;
}
