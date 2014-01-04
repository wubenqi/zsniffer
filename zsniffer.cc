// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "zsniffer/zsniffer.h"

#include <vector>
#include <iostream>

#include <boost/scoped_ptr.hpp>

#include "base/arg.h"
#include "base/utf8_util.h"

#include "netengine/packet_header.h"

#include "zsniffer/config_info.h"
#include "zsniffer/message_manager.h"
#include "zsniffer/net_headers.h"

struct PCapDevice {
	std::string name;
	std::string description;
	u_int net_mask;
};

typedef std::vector<PCapDevice> PCapDeviceList;

bool g_running = false;

BOOL WINAPI ConsoleCtrlHandler(DWORD ctrl_type) {
	switch (ctrl_type) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		g_running = false;
		return TRUE;
	default:
		return FALSE;
	}
}

std::string GetAppName(const std::string& app) {
	std::string		ident(app);

#ifndef WIN32
	size_t  startpos = ident.rfind( '/' );
#else
	size_t  startpos = ident.rfind( '\\' );
#endif
	if( std::string::npos != startpos ) {
		ident = ident.c_str() + startpos + 1;
	}
	size_t  endpos = ident.rfind( '.' );
	if( std::string::npos != endpos ) {
		ident = ident.substr( 0, endpos );
	}
	return ident;
}

void PCapThread::OnDataReceived(const IPAddress& from_ip, int from_port, const IPAddress&  to_ip, int to_port, char* data, int data_len) {
	boost::shared_ptr<IOBuffer> io_buffer = GetIOBuffer(from_ip, from_port, to_ip, to_port);
	if (!io_buffer) {
		std::cerr << "Invalid error, maybe not enough memory" << std::endl;
	}
	
	io_buffer->append((const uint8*)data, data_len);

	uint16 message_type;
	uint32 body_len;

	while (io_buffer->read_remaining()>=PacketHeader::kPacketHeaderSize) {
		int ret = PacketHeader::ParsePacketHeader((const uint8*)(io_buffer->read_ptr()), &message_type, &body_len);
		if (ret==-1) {
			std::cerr << "Recv a valid packet" << std::endl;
			io_buffer->release();
			//Close();
			//LOG(ERROR) << "Recv a valid packet";
			return;
		} else {
			const MessageInfo* message_info = MessageManager::Instance().FindMessageInfo(message_type);
			if (!message_info) {
				std::cerr << "Recv a valid packet, message_type [" << message_type << "] is valid" << std::endl;
				io_buffer->release();
				return;
			}

			std::cout << "Recv message_type " << message_info->message_type << std::endl;
			
			if (ret==1) {
				if (!message_info->is_empty) {
					std::cerr << "Recv a valid packet, message_type [" << message_type << "] is valid" << std::endl;
					io_buffer->release();
					return;
				}
				io_buffer->skip_read_pos(PacketHeader::kPacketHeaderSize);
				io_buffer->remove_read_data();
			} else if (ret==0) {
				if (io_buffer->read_remaining()>=(int)((body_len+PacketHeader::kPacketHeaderSize))) {

					boost::scoped_ptr<google::protobuf::Message> message(MessageManager::Instance().CreateMessage(message_type));
					if (message) {
						if(message->ParseFromArray(io_buffer->read_ptr()+PacketHeader::kPacketHeaderSize, body_len)) {
							if (ConfigInfo::Instance().encoding() == "utf8") {
								std::cout << message->Utf8DebugString() << std::endl;
							} else {
								std::cout << base::UTF8ToNativeMB(message->Utf8DebugString()) << std::endl;
							}
						} else {
							std::cerr << "ParseFromArray error" << std::endl;
							io_buffer->release();
							return;
						}
					}
					io_buffer->skip_read_pos(body_len+PacketHeader::kPacketHeaderSize);
					io_buffer->remove_read_data();
				} else {
					break;
				}
			}
		}
	}
}

void PacketCaptureCallBack(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data) {
	PCapThread* cap_thread = (PCapThread*)param;

	int pkt_data_len = header->caplen;

	IPHeader* ip_header;
	unsigned short ip_header_len = 0;
	unsigned short ip_total_len = 0;

	TCPHeader* tcp_header;
	unsigned short tcp_header_len;

	unsigned short source_port;
	int dest_port;

	char* data;
	int data_len;

	ip_header = (IPHeader*)(pkt_data + sizeof(EthernetHeader));
	ip_header_len = ip_header->ip_header_len*4;
	
	if(ip_header->ip_protocol==IPPROTO_TCP) {
		tcp_header = (TCPHeader*)( pkt_data + ip_header_len + sizeof(EthernetHeader) );
		tcp_header_len = tcp_header->data_offset*4;

		source_port=ntohs(tcp_header->source_port);
		dest_port=ntohs(tcp_header->dest_port);

		data = (char*)(pkt_data + sizeof(EthernetHeader) + ip_header_len + tcp_header_len);
		data_len = pkt_data_len - sizeof(EthernetHeader) - ip_header_len - tcp_header_len;

		ip_total_len = ntohs(ip_header->ip_total_length);

		if (data_len>0 && (pkt_data_len == (sizeof(EthernetHeader) + ip_total_len)) && cap_thread!=NULL) {

			char from_ip[64];
			char to_ip[64];

			sprintf(from_ip, "%d.%d.%d.%d", ip_header->ip_srcaddr.byte1, ip_header->ip_srcaddr.byte2, ip_header->ip_srcaddr.byte3, ip_header->ip_srcaddr.byte4);
			sprintf(to_ip, "%d.%d.%d.%d", ip_header->ip_destaddr.byte1, ip_header->ip_destaddr.byte2, ip_header->ip_destaddr.byte3, ip_header->ip_destaddr.byte4);
			std::cout << from_ip << "[" << source_port << "] ==> " << to_ip << "[" << dest_port << "], data_len = " << data_len << std::endl;

			cap_thread->OnDataReceived(ip_header->ip_srcaddr, source_port, ip_header->ip_destaddr, dest_port, data, data_len);
		}
	}
}

PCapThread::PCapThread(const std::string& cap_tcp_port, const std::string& device_name, const std::string& device_descr, u_int net_mask)
	: name_(device_name)
	, description_(device_descr)
	, net_mask_(net_mask)
	, cap_(NULL)
	, runnging_(false) {

	packet_filter_ = "tcp port " + cap_tcp_port;
	cap_port_ = boost::lexical_cast<int>(cap_tcp_port);
}

bool PCapThread::IsCapPort(int port) {
	return port == cap_port_;
}

bool PCapThread::Create() {
	/* Open the adapter */
	if ((cap_= pcap_open_live(name_.c_str(),	// name of the device
		65536,			// portion of the packet to capture. 
		// 65536 grants that the whole packet will be captured on all the MACs.
		PCAP_OPENFLAG_PROMISCUOUS,			// promiscuous mode (nonzero means promiscuous)
		10,			// read timeout
		err_			// error buffer
		)) == NULL) {
		std::cerr << "Failed to open the adapter." << std::endl;
		return false;
	}

	/* Check the link layer. We support only Ethernet for simplicity. */
	if(pcap_datalink(cap_) != DLT_EN10MB) {
		std::cerr << "Only supports ethernet networks." << std::endl;
		return false;
	}

	//compile the filter
	if (pcap_compile(cap_, &fcode_, packet_filter_.c_str(), 1, net_mask_) <0 ) {
		std::cerr << "Unable to compile the packet filter. Check the syntax." << std::endl;
		return false;
	}

	//set the filter
	if (pcap_setfilter(cap_, &fcode_)<0) {
		std::cerr << "Error setting the filter." << std::endl;
		return false;
	}

	Start();

	return true;
}

void PCapThread::Destroy() {
	if (runnging_) {
		runnging_ = false;
		Join();
	}

	if (cap_) {
		pcap_close(cap_);
		cap_= NULL;
	}
}

void PCapThread::Run() {
	runnging_ = true;
	while (runnging_) {
		pcap_dispatch(cap_, 0, PacketCaptureCallBack, (u_char*)this);
	}
}

PCapThread::~PCapThread() {
	Destroy();
}

boost::shared_ptr<IOBuffer> PCapThread::GetIOBuffer(const IPAddress& s_ip, uint16 s_port, const IPAddress& d_ip, uint16 d_port) {
	uint32* source_ip = (uint32*)&s_ip;
	uint32* dest_ip = (uint32*)&d_ip;
	
	boost::shared_ptr<IOBuffer> io_buffer;

	CommChannelInfo channel(*source_ip, s_port, *dest_ip, d_port);
	std::map<CommChannelInfo, boost::shared_ptr<IOBuffer> >::iterator it = channels_.find(channel);
	if (it==channels_.end()) {
		io_buffer.reset(new IOBuffer());
		channels_.insert(std::make_pair(channel, io_buffer));
	} else {
		io_buffer = it->second;
	}

	return io_buffer;
}


void EnumerateDevices(PCapDeviceList* device_list) {
	pcap_if_t *alldevs;
	pcap_if_t *d;
	char err_buf[PCAP_ERRBUF_SIZE];

	if(pcap_findalldevs(&alldevs, err_buf) == -1) {
		std::cerr << "No network adapters detected!" << std::endl;
	} else {
		PCapDevice device;
		for(d=alldevs; d; d=d->next) {
			if(d->name) {
				device.name = d->name;
				if (d->description) {
					device.description = d->description;
				} else {
					device.description = d->name;
				}
				if(d->addresses != NULL) {
					/* Retrieve the mask of the first address of the interface */
					device.net_mask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
				} else {	/* If the interface is without addresses we suppose to be in a C class network */
					device.net_mask=0xffffff; 
				}
				device_list->push_back(device);
			}
		}

		pcap_freealldevs(alldevs);
	}
}

//-------------------------------------------------------------------------------
// main
int main(int argc, char* argv[]) {
	CArg arg(argc, argv);

	CArg::ArgVal val;
	std::string ini_file;

	if ((val=arg.find1("--config=")).null()) {
		ini_file = GetAppName(argv[0])+".xml";
	} else {
		ini_file = (std::string)val;
	}

	if(0 != ConfigInfo::Instance().Initialize(ini_file.c_str())) {
		return 1;
	}

	if(!MessageManager::Instance().Initialize(ConfigInfo::Instance().map_paths(), ConfigInfo::Instance().message_type_proto_file())) {
		return 2;
	}

	PCapDeviceList device_list;
	EnumerateDevices(&device_list);

	if (device_list.empty()) {
		std::cerr << "Can't not find device" << std::endl; 
		return 1;
	}

	std::vector<PCapThread*> threads;

	for (size_t i=0; i<device_list.size(); ++i) {
		std::cout << device_list[i].description << " ==> " << device_list[i].name << std::endl;
		PCapThread* cap_thread = new PCapThread(ConfigInfo::Instance().cap_port(), device_list[i].name, device_list[i].description, device_list[i].net_mask);
		threads.push_back(cap_thread);
	}
	
	device_list.clear();

	for (size_t i=0; i<threads.size(); ++i) {
		threads[i]->Create();
	}

	g_running = true;
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	while (g_running) {
		Sleep(1);
	}

	for (size_t i=0; i<threads.size(); ++i) {
		threads[i]->Destroy();
		delete threads[i];
	}

	threads.clear();
	return 0;
}

