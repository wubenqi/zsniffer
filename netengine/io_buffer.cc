// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "netengine/io_buffer.h"

#define  NO_LOGGER

#ifndef NO_LOGGER
#include "base/logging.h"
#else

#include <stdlib.h>
#include <iostream>

#define  CHECK
#define  DCHECK
#define  LOG(ERROR) std::cerr

#endif

//namespace net {

IOBuffer::IOBuffer()
: inited_size_(kDefaultIOBufferSize)
, size_(kDefaultIOBufferSize)
, read_pos_(0)
, write_pos_(0) {

  data_ = static_cast<uint8*>(malloc(kDefaultIOBufferSize));
  CHECK(data_);
}

IOBuffer::IOBuffer(int size)
	: inited_size_(size)
	, size_(size)
	, read_pos_(0)
	, write_pos_(0) {

	DCHECK(size > 0);
	data_ = static_cast<uint8*>(malloc(size));
	CHECK(data_);
}

IOBuffer::IOBuffer(const uint8* data, int data_size)
	: inited_size_(data_size)
	, size_(data_size)
	, read_pos_(0)
	, write_pos_(data_size) {

	DCHECK(data !=NULL && data_size > 0);
	data_ = static_cast<uint8*>(malloc(data_size));
	CHECK(data_);

	memcpy(data_, data, data_size);
}

IOBuffer::~IOBuffer() {
	if (data_) {
		free(data_);
	}
}

void IOBuffer::remove_read_data() {
	if (read_pos_>0 && read_remaining()>0) {
		memmove(data_, read_ptr(), read_remaining());
		write_pos_ = read_remaining();
		read_pos_ = 0;
	} else {
		read_pos_ = write_pos_ = 0;
	}
}

void IOBuffer::remove_write_data() {
	if (write_pos_ == 0) {
		//Release();
	} else {
		data_ = static_cast<uint8*>(realloc(data_, write_pos_));
		CHECK(data_);
		size_ = write_pos_;
	}
}

bool IOBuffer::append(const uint8* data, int data_size) {
	DCHECK(data !=NULL && data_size > 0);
	if (write_remaining()<data_size) {
		size_ += data_size-write_remaining();
		data_ = static_cast<uint8*>(realloc(data_, size_));
		CHECK(data_);
		//data_.reset(static_cast<uint8*>(realloc(data_.release(), size_)));
		memcpy(write_ptr(), data, data_size);
		write_pos_ = size_;
	} else {
		memcpy(write_ptr(), data, data_size);
		write_pos_+=data_size;
	}
	return true;
}

void IOBuffer::release() {
	read_pos_ = 0;
	write_pos_ = 0;
	size_ = 0;
	data_ = static_cast<uint8*>(realloc(data_, inited_size_));
	CHECK(data_);
}

IOBuffer* IOBuffer::Clone() {
	return new IOBuffer(data_, write_pos_);
}

void IOBuffer::Resize(int size) {
	if (size_<size) {
		data_ = static_cast<uint8*>(realloc(data_, size));
		CHECK(data_);
		size_ = size;
		inited_size_ = size_;
	}
}

//>>====================================================================================
bool IOBuffer::ReadRaw(void* buf, int buf_len, bool is_peek) {
	bool result = false;
	if (buf_len<=read_remaining()) {
		memcpy(buf, read_ptr(), buf_len);
		if (!is_peek) {
			read_pos_ += buf_len;
		}
		result = true;
	}
	return result;
}

bool IOBuffer::ReadUint8(uint8* val, bool is_peek) {
	return ReadRaw(val, sizeof(uint8), is_peek);
}

bool IOBuffer::ReadBigEndianUint16(uint16* val, bool is_peek) {
	uint8 ch[2];

	bool result = ReadRaw( ch, sizeof( ch ), is_peek );

	if( result ) {
		*val = static_cast<uint16>((( ch[0] & 0xFF) << 8) | (ch[1] & 0xFF));
	}

	return result;
}

bool IOBuffer::ReadBigEndianUint24(uint32* val, bool is_peek) {
	uint8 ch[3];

	bool result = ReadRaw( ch, sizeof( ch ), is_peek );

	if( result ) {
		*val = static_cast<uint32>((( ch[0] & 0xFF) << 16) | (( ch[1] & 0xFF) << 8) | (ch[2] & 0xFF));
	}

	return result;
}

bool IOBuffer::ReadBigEndianUint32(uint32* val, bool is_peek) {
	uint8 ch[4];

	bool result = ReadRaw( ch, sizeof( ch ), is_peek );

	if( result ) {
		*val = static_cast<uint32>((( ch[0] & 0xFF) << 24) | ((ch[1] & 0xFF) << 16) | ((ch[2] & 0xFF) <<  8) | (ch[3] & 0xFF));
	}

	return result;
}

bool IOBuffer::ReadLittleEndianUint16(uint16* val, bool is_peek) {
	return ReadRaw(val, sizeof(*val), is_peek);
}

bool IOBuffer::ReadLittleEndianUint24(uint32* val, bool is_peek) {
	uint8 ch[3];

	bool result = ReadRaw( ch, sizeof( ch ), is_peek );

	if( result ) {
		*val = static_cast<uint32>( ((ch[2] & 0xFF)<<16) | ((ch[1] & 0xFF)<<8) | (ch[0] & 0xFF) );
	}

	return result;
}

bool IOBuffer::ReadLittleEndianUint32(uint32* val, bool is_peek) {
	return ReadRaw(val, sizeof(*val), is_peek);
}

// offset
bool IOBuffer::ReadOffsetRaw(void* buf, int offset, int buf_len) {
	bool result = false;
	if (buf_len+offset<=read_remaining()) {
		memcpy(buf, read_ptr()+offset, buf_len);
		result = true;
	}
	return result;
}

bool IOBuffer::ReadOffsetUint8(uint8* val, int offset) {
	return ReadOffsetRaw(val, sizeof(*val), offset);
}

// 大端
bool IOBuffer::ReadOffsetBigEndianUint16(uint16* val, int offset) {
	uint8 ch[2];

	bool result = ReadOffsetRaw( ch, sizeof( ch ), offset );

	if( result ) {
		*val = static_cast<uint32>((( ch[1] & 0xFF) << 8) | (ch[0] & 0xFF));
	}

	return result;
}

bool IOBuffer::ReadOffsetBigEndianUint24(uint32* val, int offset) {
	uint8 ch[3];

	bool result = ReadOffsetRaw( ch, sizeof( ch ), offset );

	if( result ) {
		*val = static_cast<uint32>((( ch[0] & 0xFF) << 16) | (( ch[1] & 0xFF) << 8) | (ch[2] & 0xFF));
	}

	return result;
}

bool IOBuffer::ReadOffsetBigEndianUint32(uint32* val, int offset) {
	uint8 ch[4];

	bool result = ReadOffsetRaw( ch, sizeof( ch ), offset );

	if( result ) {
		*val = static_cast<uint32>((( ch[0] & 0xFF) << 24) | ((ch[1] & 0xFF) << 16) | ((ch[2] & 0xFF) <<  8) | (ch[3] & 0xFF));
	}

	return result;
}

// 小端
bool IOBuffer::ReadOffsetLittleEndianUint16(uint16* val, int offset) {
	return ReadOffsetRaw(val, sizeof(*val), offset);
}

bool IOBuffer::ReadOffsetLittleEndianUint24(uint32* val, int offset) {
	uint8 ch[3];

	bool result = ReadOffsetRaw( ch, sizeof( ch ), offset );

	if( result ) {
		*val = static_cast<uint32>( ((ch[2] & 0xFF)<<16) | ((ch[1] & 0xFF)<<8) | (ch[0] & 0xFF) );
	}

	return result;
}

bool IOBuffer::ReadOffsetLittleEndianUint32(uint32* val, int offset) {
	return ReadOffsetRaw(val, sizeof(*val), offset);
}


// 写
bool IOBuffer::WriteRaw(const void* buf, int buf_len) {
	bool result = false;

	do {
		DCHECK(buf !=NULL && buf_len > 0);

		if (buf==NULL || buf_len<=0) {
			LOG(ERROR) << "Invalid write raw data";
			break;
		}

		if (write_remaining()<buf_len) {
			size_ += buf_len-write_remaining();
			data_ = static_cast<uint8*>(realloc(data_, size_));
			CHECK(data_);
			if (data_==NULL) {
				break;
			}
		}

		memcpy(write_ptr(), buf, buf_len);
		write_pos_ += buf_len;
		result = true;
	} while (0);
	
	return result;
}

bool IOBuffer::WriteUint8(uint8 val) {
	return WriteRaw(&val, sizeof(val));
}

// 大端
bool IOBuffer::WriteBigEndianUint16(uint16 val) {
	uint8 ch[2];

	ch[0] = (val >> 8) & 0xff;
	ch[1]= val & 0xff;

	return WriteRaw(ch, sizeof(ch));
}

bool IOBuffer::WriteBigEndianUint24(uint32 val) {
	uint8 ch[3];
	
	ch[0] = (val >> 16) & 0xff;
	ch[1] = (val >> 8) & 0xff;
	ch[2]= val & 0xff;

	return WriteRaw(ch, sizeof(ch));
}

bool IOBuffer::WriteBigEndianUint32(uint32 val) {
	uint8 ch[4];
	
	ch[0] = (val >> 24) & 0xff;
	ch[1] = (val >> 16) & 0xff;
	ch[2] = (val >> 8) & 0xff;
	ch[3]= val & 0xff;

	return WriteRaw(ch, sizeof(ch));
}

// 小端
bool IOBuffer::WriteLittleEndianUint16(uint16 val) {
	return WriteRaw(&val, sizeof(val));
}

bool IOBuffer::WriteLittleEndianUint24(uint32 val) {
	uint8 ch[3];

	ch[2] = (val >> 16) & 0xff;
	ch[1] = (val >> 8) & 0xff;
	ch[0]= val & 0xff;

	return WriteRaw(ch, sizeof(ch));
}

bool IOBuffer::WriteLittleEndianUint32(uint32 val) {
	return WriteRaw(&val, sizeof(val));
}

// offset
bool IOBuffer::WriteOffsetRaw(const void* buf, int buf_len, int offset) {
	bool result = false;

	do {
		DCHECK(buf !=NULL && buf_len > 0);

		if (buf==NULL || buf_len<=0) {
			LOG(ERROR) << "Invalid write raw data";
			break;
		}

		if (write_remaining()<buf_len+offset) {
			size_ += buf_len+offset-write_remaining();
			data_ = static_cast<uint8*>(realloc(data_, size_));
			CHECK(data_);
			if (data_==NULL) {
				break;
			}
		}

		memcpy(write_ptr()+offset, buf, buf_len);
		//write_pos_ += buf_len;
		result = true;
	} while (0);

	return result;
}

bool IOBuffer::WriteOffsetUint8(uint8 val, int offset) {
	return WriteOffsetRaw(&val, sizeof(val), offset);
}

// 大端
bool IOBuffer::WriteOffsetBigEndianUint16(uint16 val, int offset) {
	uint8 ch[2];

	ch[0] = (val >> 8) & 0xff;
	ch[1]= val & 0xff;

	return WriteOffsetRaw(ch, sizeof(ch), offset);
}

bool IOBuffer::WriteOffsetBigEndianUint24(uint32 val, int offset) {
	uint8 ch[3];

	ch[0] = (val >> 16) & 0xff;
	ch[1] = (val >> 8) & 0xff;
	ch[2]= val & 0xff;

	return WriteOffsetRaw(ch, sizeof(ch), offset);
}

bool IOBuffer::WriteOffsetBigEndianUint32(uint32 val, int offset) {
	uint8 ch[4];

	ch[0] = (val >> 24) & 0xff;
	ch[1] = (val >> 16) & 0xff;
	ch[2] = (val >> 8) & 0xff;
	ch[3]= val & 0xff;

	return WriteOffsetRaw(ch, sizeof(ch), offset);
}

// 小端
bool IOBuffer::WriteOffsetLittleEndianUint16(uint16 val, int offset) {
	return WriteOffsetRaw(&val, sizeof(val), offset);
}

bool IOBuffer::WriteOffsetLittleEndianUint24(uint32 val, int offset) {
	uint8 ch[3];

	ch[2] = (val >> 16) & 0xff;
	ch[1] = (val >> 8) & 0xff;
	ch[0]= val & 0xff;

	return WriteOffsetRaw(ch, sizeof(ch), offset);
}

bool IOBuffer::WriteOffsetLittleEndianUint32(uint32 val, int offset) {
	return WriteOffsetRaw(&val, sizeof(val), offset);
}

//}  // namespace net
