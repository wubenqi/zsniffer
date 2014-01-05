// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#ifndef NET_BASE_IO_BUFFER_H_
#define NET_BASE_IO_BUFFER_H_
#pragma once

#include "base/basictypes.h"


//namespace net {

class IOBuffer {
public:
	enum {
		kDefaultIOBufferSize = 4096
	};

	IOBuffer();
	explicit IOBuffer(int size);
	IOBuffer(const uint8* data, int data_size);
	virtual ~IOBuffer();

	//>>====================================================================================
	inline uint8* data() {
		return data_;
	}

	inline int size() {
		return size_;
	}

	inline uint8* read_ptr() {
		return data_+read_pos_;
	}

	inline int read_remaining() {
		return write_pos_-read_pos_;
	}

	inline int read_pos() {
		return read_pos_;
	}

	inline bool skip_read_pos(int size) {
		if (size<0) {
			if (read_pos_+size<0) {
				read_pos_ = 0;
				return true;
			}
		}

		if (size>read_remaining()) {
			size = read_remaining();
		}
		read_pos_+=size;
		return true;
	}

	inline bool seek_read_pos(int size) {
		if (size>write_pos_) {
			return false;
		}
		read_pos_ = size;

		return true;
	}

	inline uint8* write_ptr() {
		return data_+write_pos_;
	}

	inline int write_remaining() {
		return size_-write_pos_;
	}

	inline int write_pos() {
		return write_pos_;
	}

	void remove_read_data();
	void remove_write_data();

	bool append(const uint8* data, int data_size);

	inline void Recycle() {
		read_pos_ = 0;
		write_pos_ = 0;
	}
	
	void Resize(int size);

	void release();

private:
	uint8* data_;

	int size_;			// 数据块大小
	int read_pos_;		// 读位置
	int write_pos_;		// 写位置

	int inited_size_;	
};

#define CHECK_IOBUFFER_READABLE(io_buffer, size) \
do { \
    if((size) > (io_buffer).read_remaining()) { \
        LOG(WARNING) << "Not enough data. Wanted: " << (size) << "; Got: " << (io_buffer).read_remaining(); \
        return false; \
    } \
}while(0)

#endif  // NET_BASE_IO_BUFFER_H_
