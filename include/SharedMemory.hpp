//
// Created by zekro on 3/15/2026.
//

#ifndef HIGHFREQUENCY_SHAREDMEMORY_H
#define HIGHFREQUENCY_SHAREDMEMORY_H

#include <Windows.h>
#include <string>
#include <stdexcept>

template <typename  T>
class SharedMemory
{
private:
    HANDLE _hMapFile = NULL;
    T* _pBuf = nullptr;
    bool _isProducer = false;

public:
    SharedMemory(const std::string& memName, bool isProducer) : _isProducer(isProducer)
    {
        if (isProducer)
        {
            _hMapFile = CreateFileMappingA(
                INVALID_HANDLE_VALUE,
                NULL,
                PAGE_READWRITE,
                0,
                sizeof(T),
                memName.c_str()
                );
        }
        else
        {
            _hMapFile = OpenFileMappingA(
                FILE_MAP_ALL_ACCESS,
                FALSE,
                memName.c_str()
                );
        }

        if (_hMapFile == NULL)
        {
            throw std::runtime_error("Handle Error: " + std::to_string(GetLastError()));
        }

        _pBuf = static_cast<T*>(MapViewOfFile(
            _hMapFile,
            FILE_MAP_ALL_ACCESS,
            0, 0, sizeof(T)
            ));

        if (_pBuf == nullptr)
        {
            CloseHandle(_hMapFile);
            throw std::runtime_error("Map Error: " + std::to_string(GetLastError()));
        }
    }

    T* get() {return _pBuf;}

    ~SharedMemory()
    {
        if (_pBuf != nullptr) UnmapViewOfFile(_pBuf);
        if (_hMapFile) CloseHandle(_hMapFile);
    }

    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;
};

#endif //HIGHFREQUENCY_SHAREDMEMORY_H