#pragma once
#include <vector>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef std::vector<uint8_t> ByteArray;
typedef std::vector<int16_t> SampleArray;

bool readFile( const std::string& filename, ByteArray& bytes )
{
    int fd = ::open( filename.c_str(), O_RDONLY );
    if ( fd<0 ) {
        printf( "Could not open file %s for writing\n", filename.c_str() );
        return false;
    }
    struct stat sb;
    if ( ::fstat( fd, &sb )!=0 ) return false;
    bytes.resize( sb.st_size );
    uint64_t offset = 0;
    uint64_t bufsize = 4*4096;
    while ( offset < sb.st_size ) {
        int64_t nb = ::read( fd, &bytes[offset], bufsize );
        if ( nb<=0 ) break;
        offset += nb;
    }
    ::close( fd );
    printf( "Read %ld bytes from %s\n", offset, filename.c_str() );
    return true;
}

bool writeFile( const std::string& filename, const ByteArray& bytes ) {
    int fd = ::open( filename.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP );
    if ( fd<0 ) {
        printf( "%s\n", strerror( errno ) );
        printf( "Could not open file [%s] for writing\n", filename.c_str() );

        return false;
    }
    uint64_t offset = 0;
    uint64_t bufsize = 4*4096;
    uint64_t left = bytes.size();
    while ( offset < bytes.size() ) {
        int64_t nb = ::write( fd, &bytes[offset], left>bufsize ? bufsize : left );
        if ( nb<=0 ) break;
        offset += nb;
        left -= nb;
    }
    ::close(fd);
    printf( "Wrote %ld bytes to %s\n", offset, filename.c_str() );
    return true;
}
