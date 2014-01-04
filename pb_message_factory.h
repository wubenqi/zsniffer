// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#ifndef	ZSNIFFER_PB_MESSAGE_FACTORY_H_
#define	ZSNIFFER_PB_MESSAGE_FACTORY_H_
#pragma once

#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>


class PBErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector { 
public:
	virtual void AddError(const std::string& filename, int line, int column, const std::string& message) {
		std::cout << "[file] " << filename << " [line]" << line
			<< " [column]" << column << "\n[message] " << message 
			<< std::endl;
	}
}; 

class PBMessageFactory {
public:
	PBMessageFactory()
		: importer_(&disk_tree_, &error_collector_) {
	}

	void MapPath(const std::string& virtual_path, const std::string& disk_path);
	void MapPaths(const std::vector<std::string>& paths);

	google::protobuf::Message* CreateMessage(const std::string& proto_file, const std::string& type_name);

private:
	PBErrorCollector      error_collector_;
	google::protobuf::compiler::DiskSourceTree disk_tree_;
	google::protobuf::compiler::Importer importer_;
	google::protobuf::DynamicMessageFactory message_factory_;
};

#endif

