//
//  PsdNativeFile_Mac.cpp
//  Contributed to psd_sdk
//
//  Created by Oluseyi Sonaiya on 3/29/20.
//  Copyright © 2020 Oluseyi Sonaiya. All rights reserved.
//
// psd_sdk Copyright 2011-2020, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include <wchar.h>
#include <codecvt>
#include <locale>
#include <string>
#include <unistd.h>
#include <fcntl.h>

#include "PsdPch.h"
#include "PsdNativeFile_Mac.h"

#include "PsdAllocator.h"
#include "PsdPlatform.h"
#include "PsdMemoryUtil.h"
#include "PsdLog.h"
#include "Psdinttypes.h"


PSD_NAMESPACE_BEGIN

struct DispatchReadOperation
{
    void* dataReadBuffer;
    uint32_t length;
    uint64_t offset;
};

struct DispatchWriteOperation
{
    const void* dataToWrite;
    uint32_t length;
    uint64_t offset;
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
NativeFile::NativeFile(Allocator* allocator)
    : File(allocator)
    , m_fileDescriptor(0)
{
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool NativeFile::DoOpenRead(const wchar_t* filename)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
    std::string s = convert.to_bytes(filename);
    char const *cs = s.c_str();
    m_fileDescriptor = open(cs, O_RDONLY);
    if (m_fileDescriptor == -1)
    {
        PSD_ERROR("NativeFile", "Cannot obtain handle for file \"%ls\".", filename);
        return false;
    }

    return true;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool NativeFile::DoOpenWrite(const wchar_t* filename)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
    std::string s = convert.to_bytes(filename);
    char const *cs = s.c_str();
    m_fileDescriptor = open(cs, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
    if (m_fileDescriptor == -1)
    {
        PSD_ERROR("NativeFile", "Cannot obtain handle for file \"%ls\".", filename);
        return false;
    }

    return true;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool NativeFile::DoClose(void)
{
    if (m_fileDescriptor == -1)
        return false;
    
    const int success = close(m_fileDescriptor);
    if  (success == -1)
    {
        PSD_ERROR("NativeFile", "Cannot close handle.");
        return false;
    }

    m_fileDescriptor = -1;
    return true;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
File::ReadOperation NativeFile::DoRead(void* buffer, uint32_t count, uint64_t position)
{
    DispatchReadOperation *operation = memoryUtil::Allocate<DispatchReadOperation>(m_allocator);
    operation->dataReadBuffer = buffer;
    operation->length = count;
    operation->offset = position;
    return static_cast<File::ReadOperation>(operation);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool NativeFile::DoWaitForRead(File::ReadOperation& operation)
{
    DispatchReadOperation *op = static_cast<DispatchReadOperation *>(operation);
    lseek(m_fileDescriptor, op->offset, SEEK_SET);
    const ssize_t nbytes = read(m_fileDescriptor, op->dataReadBuffer, op->length);
    
    if (nbytes < op->length)
    {
        PSD_ERROR("NativeFile", "Failed to read required number of bytes.");
        return false;
    }

    return true;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
File::WriteOperation NativeFile::DoWrite(const void* buffer, uint32_t count, uint64_t position)
{
    DispatchWriteOperation *operation = memoryUtil::Allocate<DispatchWriteOperation>(m_allocator);
    operation->length = count;
    operation->offset = position;
    operation->dataToWrite = buffer;
    return static_cast<File::ReadOperation>(operation);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool NativeFile::DoWaitForWrite(File::WriteOperation& operation)
{
    DispatchWriteOperation *op = static_cast<DispatchWriteOperation *>(operation);
    lseek(m_fileDescriptor, op->offset, SEEK_SET);
    const ssize_t nbytes = write(m_fileDescriptor, op->dataToWrite, op->length);
    
    if (nbytes < op->length)
    {
        PSD_ERROR("NativeFile", "Failed to write required number of bytes.");
        return false;
    }

    return true;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t NativeFile::DoGetSize(void) const
{
// fstat

    return 0;
}

PSD_NAMESPACE_END
