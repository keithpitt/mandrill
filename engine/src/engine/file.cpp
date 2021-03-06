#include "file.hpp"

#include <stdio.h>
#include <string>
#include <physfs/physfs.h>

#include <boost/format.hpp>

void kp::file::init(const char* argv0)
{
    if(PHYSFS_init(argv0) == 0)
    {
        //kp::debug::fatal("PHYSFS_init: %s", PHYSFS_getLastError());
    }

#ifdef __APPLE__
    std::string formatted = boost::str(boost::format("%s/%s") % PHYSFS_getBaseDir() % "Contents/Resources");
    const char* directory = formatted.c_str();
#else
    const char* directory = PHYSFS_getBaseDir();
#endif
    
    //printf("%c", kp::string::format("%s/%s", PHYSFS_getBaseDir(), "Contents/Resources"));

    if(PHYSFS_mount(directory, NULL, 1) == 0)
    {
        //kp::debug::fatal("PHYSFS_mount(%c): %s", directory, PHYSFS_getLastError());
    }
};

void kp::file::cleanup()
{
    if(PHYSFS_deinit() == 0)
    {
        //kp::debug::fatal("PHYSFS_deinit: %s", PHYSFS_getLastError());
    }
}

const char* kp::file::read(const char* filename)
{
    PHYSFS_File* handle = PHYSFS_openRead(filename);

    if(handle == NULL)
    {
        //kp::debug::error("PHYSFS_openRead(%s): %s", filename, PHYSFS_getLastError());
        
        return NULL;
    }

    // Create a buffer big enough for the file
    PHYSFS_sint64 size = PHYSFS_fileLength(handle);

    // Append an extra byte to the string so we can null terminate it
    char* buffer = new char[size+1];

    // Read the bytes
    if(PHYSFS_readBytes(handle, buffer, size) != size)
    {
        //kp::debug::error("PHYSFS_read: %s", PHYSFS_getLastError());
        
        return NULL;
    }

    // Null terminate the buffer
    buffer[size] = '\0';

    // Close the file handle
    PHYSFS_close(handle);

    return buffer;
}
