#ifndef AV_LIB_H
#define AB_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

//create custom defineS for platforms?????????
#ifdef __linux__
#include <sys/stat.h>
#include <dlfcn.h>

#elif   _WIN32
#include <Windows.h>

#endif  //platform

typedef unsigned char   ubyte;
typedef int8_t          int8;
typedef int16_t         int16;
typedef int32_t         int32;
typedef int64_t         int64;
typedef uint8_t         uint8;
typedef uint16_t        uint16;
typedef uint32_t        uint32;
typedef uint64_t        uint64; 

typedef int8_t          i8;
typedef int16_t         i16;
typedef int32_t         i32;
typedef int64_t         i64;
typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64; 

#define Kilobytes(x) ((x)*1024LL)
#define Megabytes(x) (Kilobytes(x)*1024LL)
#define Gigabytes(x) (Megabytes(x)*1024LL)

int     checkArg(int argc, char** argv, const char* argument);
int     copyFile(const char* source, const char* destination);
uint64  fileChangeTime(const char* path);
int     strToInt(const char* str);

void*   loadLibrary(const char* file_name);
int     freeLibrary(void* library);
void*   getSymAddress(void* library, const char* sym_name);

void*   virtualAlloc(void* address, uint64 size);
int     virtualFree(void* address, uint64 size);

#ifdef AV_LIB_IMPLEMENTATION
int checkArg(int argc, char** argv, const char* argument)
{
    for(int i = 0; i < argc; ++i){
        if(strcmp(argv[i], argument) == 0)
            return i;
    }
    return -1;
}

#ifdef __linux__

int copyFile(const char* source, const char* destination)
{

    printf("copying %s to %s\n", source, destination);
    FILE* src = fopen(source, "rb");
    FILE* dst = fopen(destination, "wb+");
    int status = 0;
    uint64 size; 
    uint64 bytes_copyed = 0;
    ubyte* buffer;

    if(src == NULL || dst == NULL){
        status = -1;
        goto exit;
    }

    fseek(src, 0, SEEK_END);
    size = ftell(src); 
    fseek(src, 0, SEEK_SET);
    buffer = (ubyte*)malloc(size);
    if(buffer == NULL){
        status = -1; 
        goto exit;
    }

    fread(buffer, sizeof(ubyte), size, src);
    bytes_copyed = fwrite(buffer, sizeof(ubyte), size, dst);
    
    if(bytes_copyed != size){
        printf("wrong size \n");
        status = -1;
    }

exit:
    fclose(src);
    fclose(dst);
    if(buffer) free(buffer);
    printf("done copying %d original size%ld size:%ld\n\n", status, size, bytes_copyed);
    return status;
}

uint64 fileChangeTime(const char* path)
{
    struct stat attr;    
    if(stat(path, &attr) == -1)
        return 0;
    return attr.st_mtime;
}

void* loadLibrary(const char* file_name)
{
    printf("loading fucking lib %s \n", file_name);
    return dlopen(file_name, RTLD_NOW);
}

int freeLibrary(void* library)
{
    return dlclose(library);
}

void* getSymAddress(void* library, const char* sym_name)
{
    return dlsym(library, sym_name);
}

void* virtualAlloc(void* address, uint64 size)
{
    //TODO: implement linux version
    assert(malloc(size));
    return 0;
}

int virtualFree(void* address, uint64 size)
{
    //TODO: implement linux version
    //assert(free(size));
    free(address);
    return 0;
}

#elif _WIN32
int copyFile(const char* src, const char* dst)
{
    int result = CopyFile(src, dst, FALSE);
    if(result == 0){
        return -1;
    }
    return 0;
}

uint64 fileChangeTime(const char* path)
{
    WIN32_FIND_DATA find_data;
    HANDLE file_handle = FindFirstFileA(path, &find_data);
    if(file_handle == INVALID_HANDLE_VALUE)
        return 0;
    FindClose(file_handle);
    FILETIME file_time = find_data.ftLastWriteTime;

    uint64 result = file_time.dwHighDateTime;
    result = result << 32;
    result |= file_time.dwLowDateTime;
    return result;
}

void* loadLibrary(const char* file_name)
{
    return LoadLibrary(file_name);
}

int freeLibrary(void* library)
{
    int result = FreeLibrary((HMODULE)library);
    if(result != 0) //success
        return 0;
    else 
        return -1; 
}

void* getSymAddress(void* library, const char* sym_name)
{
    void* lel = GetProcAddress((HMODULE)library, sym_name);
    return lel;
}

void* virtualAlloc(void* address, uint64 size)
{
    return VirtualAlloc(address, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}

int virtualFree(void* address, uint64 size)
{
    if(VirtualFree(address, size, MEM_RELEASE) == 0)
        return -1;
    else return 0;
}
    
#endif  //platform

int strToInt(const char* str) //unsafe? callers problem to pass \0 strs?
{
    //TODO: add base 16 support 0x...
    int result = 0;
    for(const char *c = str; *c != '\0'; ++c){
        if(*c >= '0' && *c <= '9'){
            result *= 10;
            result += *c - '0';
        } 
    }
    
    return result;
}
#endif //AV_LIB_IMPLEMENTATION
#endif //AV_LIB_H
