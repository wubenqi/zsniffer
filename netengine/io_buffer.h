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

	IOBuffer* Clone();

	//>>====================================================================================
	// 读
	bool ReadRaw(void* buf, int buf_len, bool is_peek = false);
	bool ReadUint8(uint8* val, bool is_peek = false);

	// 大端
	bool ReadBigEndianUint16(uint16* val, bool is_peek = false);
	bool ReadBigEndianUint24(uint32* val, bool is_peek = false);
	bool ReadBigEndianUint32(uint32* val, bool is_peek = false);

	// 小端
	bool ReadLittleEndianUint16(uint16* val, bool is_peek = false);
	bool ReadLittleEndianUint24(uint32* val, bool is_peek = false);
	bool ReadLittleEndianUint32(uint32* val, bool is_peek = false);

	// offset
	bool ReadOffsetRaw(void* buf, int offset, int buf_len);
	bool ReadOffsetUint8(uint8* val, int offset);

	// 大端
	bool ReadOffsetBigEndianUint16(uint16* val, int offset);
	bool ReadOffsetBigEndianUint24(uint32* val, int offset);
	bool ReadOffsetBigEndianUint32(uint32* val, int offset);

	// 小端
	bool ReadOffsetLittleEndianUint16(uint16* val, int offset);
	bool ReadOffsetLittleEndianUint24(uint32* val, int offset);
	bool ReadOffsetLittleEndianUint32(uint32* val, int offset);

	// 写
	bool WriteRaw(const void* buf, int buf_len);
	bool WriteUint8(uint8 val);

	// 大端
	bool WriteBigEndianUint16(uint16 val);
	bool WriteBigEndianUint24(uint32 val);
	bool WriteBigEndianUint32(uint32 val);

	// 小端
	bool WriteLittleEndianUint16(uint16 val);
	bool WriteLittleEndianUint24(uint32 val);
	bool WriteLittleEndianUint32(uint32 val);

	// offset
	bool WriteOffsetRaw(const void* buf, int buf_len, int offset);
	bool WriteOffsetUint8(uint8 val, int offset);

	// 大端
	bool WriteOffsetBigEndianUint16(uint16 val, int offset);
	bool WriteOffsetBigEndianUint24(uint32 val, int offset);
	bool WriteOffsetBigEndianUint32(uint32 val, int offset);

	// 小端
	bool WriteOffsetLittleEndianUint16(uint16 val, int offset);
	bool WriteOffsetLittleEndianUint24(uint32 val, int offset);
	bool WriteOffsetLittleEndianUint32(uint32 val, int offset);

private:
	uint8* data_;

	int size_;			// 数据块大小
	int read_pos_;		// 读位置
	int write_pos_;		// 写位置

	int inited_size_;	
};


namespace base {
inline void ReadBigEndianUint8(const void* buf, uint8* out) {
	*out = static_cast<const uint8*>(buf)[0];
}

inline void ReadBigEndianUint16(const void* buf, uint16* out) {
	*out = static_cast<const uint8*>(buf)[0];
	*out <<= 8;
	*out |= static_cast<const uint8*>(buf)[1];
}

inline void ReadBigEndianUint24(const void* buf, uint16* out) {
	*out = static_cast<const uint8*>(buf)[0];

	*out <<= 8;
	*out |= static_cast<const uint8*>(buf)[1];

	*out <<= 8;
	*out |= static_cast<const uint8*>(buf)[2];
}

inline void ReadBigEndianUint32(const void* buf, uint32* out) {
	*out = static_cast<const uint8*>(buf)[0];
	
	*out <<= 8;
	*out |= static_cast<const uint8*>(buf)[1];

	*out <<= 8;
	*out |= static_cast<const uint8*>(buf)[2];

	*out <<= 8;
	*out |= static_cast<const uint8*>(buf)[3];

}

inline void WriteBigEndianUint8(void* buf, uint8 val) {
	static_cast<uint8*>(buf)[0] = val;
}

inline void WriteBigEndianUint16(void* buf, uint16 val) {
	static_cast<uint8*>(buf)[1] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;

	static_cast<uint8*>(buf)[0] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;
}

inline void WriteBigEndianUint24(void* buf, uint32 val) {
	static_cast<uint8*>(buf)[2] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;

	static_cast<uint8*>(buf)[1] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;

	static_cast<uint8*>(buf)[0] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;
}

inline void WriteBigEndianUint32(void* buf, uint32 val) {
	static_cast<uint8*>(buf)[3] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;

	static_cast<uint8*>(buf)[2] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;

	static_cast<uint8*>(buf)[1] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;

	static_cast<uint8*>(buf)[0] = static_cast<uint8>(val & 0xFF);;
	val >>= 8;
}

}

#define CHECK_IOBUFFER_READABLE(io_buffer, size) \
do { \
    if((size) > (io_buffer).read_remaining()) { \
        LOG(WARNING) << "Not enough data. Wanted: " << (size) << "; Got: " << (io_buffer).read_remaining(); \
        return false; \
    } \
}while(0)


#endif  // NET_BASE_IO_BUFFER_H_
