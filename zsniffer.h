// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#ifndef ZSNIFFER_ZSNIFFER_H_
#define ZSNIFFER_ZSNIFFER_H_
#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

#include "base/basictypes.h"

#define HAVE_REMOTE
#include "pcap.h"

#include "base/thread.h"

#include "netengine/io_buffer.h"

struct CommChannelInfo {
	uint32 source_ip;
	uint16 source_port;
	uint32 dest_ip;
	uint16 dest_port;

	CommChannelInfo(uint32 s_ip, uint16 s_port, uint32 d_ip, uint16 d_port)
		: source_ip(s_ip)
		, source_port(s_port)
		, dest_ip(d_ip)
		, dest_port(d_port) {
	}

	CommChannelInfo(const CommChannelInfo& that)
		: source_ip(that.source_ip)
		, source_port(that.source_port)
		, dest_ip(that.dest_ip)
		, dest_port(that.dest_port) {
	}

	CommChannelInfo& operator=(const CommChannelInfo& that) {
		source_ip = that.source_ip;
		source_port = that.source_port;
		dest_ip = that.dest_ip;
		dest_port = that.dest_port;
		return *this;
	}

	bool operator==(const CommChannelInfo& that) const {
		return source_ip == that.source_ip
			&& source_port == that.source_port
			&& dest_ip == that.dest_ip
			&& dest_port == that.dest_port;
	}

	bool operator!=(const CommChannelInfo& that) const {
		return source_ip != that.source_ip
			|| source_port != that.source_port
			|| dest_ip != that.dest_ip
			|| dest_port != that.dest_port;
	}

	// Required for some STL containers and operations
	bool operator<(const CommChannelInfo& that) const {
		if (source_ip < that.source_ip
			&& source_port < that.source_port
			&& dest_ip < that.dest_ip
			&& dest_port < that.dest_port ) {
			return true;
		}

		return false;
	}

};

struct IPAddress;
class PCapThread : public base::Thread {
public:
	PCapThread(const std::string& cap_tcp_port, const std::string& device_name, const std::string& device_descr, u_int net_mask);
	~PCapThread();

	bool Create();
	void Destroy();
	bool IsCapPort(int port);
	
	boost::shared_ptr<IOBuffer> GetIOBuffer(const IPAddress& s_ip, uint16 s_port, const IPAddress& d_ip, uint16 d_port);
	void OnDataReceived(const IPAddress& from_ip, int from_port, const IPAddress&  to_ip, int to_port, char* data, int data_len);

protected:
	virtual void Run();

private:
	std::string packet_filter_;
	std::string name_;
	std::string description_;
	u_int net_mask_;
	pcap_t* cap_;
	char err_[PCAP_ERRBUF_SIZE];
	bool runnging_;
	struct bpf_program fcode_;
	int cap_port_;

	std::map<CommChannelInfo, boost::shared_ptr<IOBuffer> > channels_;
}; 

#endif
