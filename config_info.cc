// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "zsniffer/config_info.h"

#include <iostream>
#include "base/xml_ini_file.h"

int ConfigInfo::Initialize(const char* ini_file) {
	if (ini_file==NULL) {
		std::cerr << "ini_file is empty()";
		return -1;
	}

	XMLIniFile xml_ini;
	if(!xml_ini.LoadXMLIniFile(ini_file)) {
		std::cerr << "parser config_file " << ini_file << " error.";
		return -1;
	}

	Destroy();

	xml_ini.GetStrings("MapPath", "Path", map_paths_);
	xml_ini.GetString("System", "message_type_proto_file", message_type_proto_file_, "const_message_types.proto");
	xml_ini.GetString("System", "cap_port", cap_port_, "4023");
	xml_ini.GetString("System", "encoding", encoding_, "utf8");

	return 0;
}

void ConfigInfo::Destroy() {
}
