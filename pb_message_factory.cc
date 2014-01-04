// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "zsniffer/pb_message_factory.h"

void PBMessageFactory::MapPath(const std::string& virtual_path, const std::string& disk_path) {
	disk_tree_.MapPath(virtual_path, disk_path);
}

void PBMessageFactory::MapPaths(const std::vector<std::string>& paths) {
	for(size_t i = 0; i < paths.size(); i++) {
		disk_tree_.MapPath("", paths[i]);
	}
}

google::protobuf::Message* PBMessageFactory::CreateMessage(const std::string& proto_file, const std::string& type_name) {
	const google::protobuf::FileDescriptor* f = importer_.Import(proto_file);
	if(f == NULL)
		return NULL;

	const google::protobuf::Descriptor* d = f->FindMessageTypeByName(type_name);
	if(d == NULL)
		return NULL;

	google::protobuf::Message* msg = message_factory_.GetPrototype(d)->New();
	return msg;
}

/*
{
	PBMessageFactory message_factory;

	std::vector<std::string> map_paths;
	xml_ini.GetStrings("MapPath", "Path", map_paths);
	message_factory.MapPaths(map_paths);

	scoped_ptr<google::protobuf::Message> message(message_factory.CreateMessage(pb_file, pb_name));
	std::ifstream ifs;

	ifs.open(pb_data.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!ifs.good()) {
		LOG(ERROR) << "open data file " << pb_data << " error!";
		return 0;
	}

	if (!message->ParseFromIstream(&ifs)) {
		LOG(ERROR) << "Parse proto data file " << pb_data << " error!";
		return 0;
	}

	std::cout << base::UTF8ToNativeMB(message->Utf8DebugString()) << std::endl;

}
*/
