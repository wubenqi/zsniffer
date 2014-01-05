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

void IOBuffer::Resize(int size) {
	if (size_<size) {
		data_ = static_cast<uint8*>(realloc(data_, size));
		CHECK(data_);
		size_ = size;
		inited_size_ = size_;
	}
}

//}  // namespace net
