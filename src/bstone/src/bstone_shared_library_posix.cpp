/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef _WIN32

#include "bstone_shared_library.h"

#if defined(__3DS__) || defined(_3DS)

namespace bstone {

void SharedLibraryHandleDeleter::operator()(void*) const noexcept
{
}

bool SharedLibrary::try_open(const char*)
{
    close();
    return false;
}

void* SharedLibrary::find_symbol(const char*) const noexcept
{
    return nullptr;
}

} // namespace bstone

#else

#include <dlfcn.h>
#include "bstone_exception.h"

namespace bstone {

void SharedLibraryHandleDeleter::operator()(void* handle) const noexcept
{
    dlclose(handle);
}

bool SharedLibrary::try_open(const char* file_path)
{
    close();

    auto handle = SharedLibraryHandleUPtr{
        dlopen(file_path, RTLD_NOW | RTLD_LOCAL)
    };

    if (handle == nullptr)
    {
        return false;
    }

    handle_.swap(handle);
    return true;
}

void* SharedLibrary::find_symbol(const char* symbol_name) const noexcept
{
    return dlsym(handle_.get(), symbol_name);
}

} // namespace bstone

#endif // Nintendo 3DS

#endif // _WIN32