#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <string>
#include <vector>

struct Vec3 {
    float x, y, z;
};

class Memory {
public:
    HANDLE handle = nullptr;
    DWORD pid = 0;
    uintptr_t client_base = 0;
    size_t client_size = 0;

    bool attach(const char* process_name) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32 pe{};
        pe.dwSize = sizeof(pe);
        if (!Process32First(snap, &pe)) { CloseHandle(snap); return false; }

        do {
            if (_stricmp(pe.szExeFile, process_name) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &pe));
        CloseHandle(snap);

        if (!pid) return false;
        handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
        return handle != nullptr;
    }

    bool find_module(const char* module_name) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return false;

        MODULEENTRY32 me{};
        me.dwSize = sizeof(me);
        if (!Module32First(snap, &me)) { CloseHandle(snap); return false; }

        do {
            if (_stricmp(me.szModule, module_name) == 0) {
                client_base = (uintptr_t)me.modBaseAddr;
                client_size = me.modBaseSize;
                CloseHandle(snap);
                return true;
            }
        } while (Module32Next(snap, &me));
        CloseHandle(snap);
        return false;
    }

    template<typename T>
    T read(uintptr_t address) const {
        T value{};
        if (!handle) return value;
        SIZE_T bytesRead = 0;
        BOOL success = ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead);
        if (!success || bytesRead < sizeof(T)) return T{};
        return value;
    }

    std::vector<uint8_t> read_bytes(uintptr_t address, size_t size) const {
        std::vector<uint8_t> buf;
        if (!handle) return buf;
        SIZE_T bytesRead = 0;
        buf.resize(size);
        BOOL success = ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(address), buf.data(), size, &bytesRead);
        if (!success) return std::vector<uint8_t>();
        buf.resize(bytesRead);
        return buf;
    }

    uintptr_t read_ptr(uintptr_t address) const {
        return read<uintptr_t>(address);
    }

    Vec3 read_vec3(uintptr_t address) const {
        return read<Vec3>(address);
    }

    uintptr_t get_entity(uintptr_t entity_list, int index) const {
        if (!entity_list) return 0;
        int chunk_idx = (index & 0x7FFF) >> 9;
        int entry_idx = index & 0x1FF;
        uintptr_t chunk_ptr = read_ptr(entity_list + 0x10 + chunk_idx * 8);
        if (!chunk_ptr) return 0;
        uintptr_t identity = chunk_ptr + entry_idx * 0x70;
        return read_ptr(identity);
    }

    bool read_view_matrix(uintptr_t address, float out[16]) const {
        auto data = read_bytes(address, 16 * sizeof(float));
        if (data.size() < 16 * sizeof(float)) return false;
        for (int i = 0; i < 16; i++)
            out[i] = *reinterpret_cast<float*>(data.data() + i * 4);
        return true;
    }

    ~Memory() {
        if (handle) CloseHandle(handle);
    }
};