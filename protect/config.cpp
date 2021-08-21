#include "config.h"
#include "protect.hpp"
#include "../server_api.hpp"
#include <stdio.h>

size_t hwid_raw_calc()
{
    static size_t offset = 0;
    static const size_t hwid_raw_size = sizeof(g_hwid_raw) - 1;
    if (offset == 0)
    {
        // DWORD serial_num = 0;
        GetVolumeInformationA(str("C:\\"), NULL, 0, (PDWORD)&g_hwid_raw[offset], NULL, NULL, NULL, 0);
        offset += sizeof(DWORD);

        __cpuid((int*)&g_hwid_raw[offset], 0);
        offset += 4 * sizeof(int);

        DWORD computer_name_size = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerNameA((LPSTR)&g_hwid_raw[offset], &computer_name_size);

        for (size_t i = 0; i < hwid_raw_size; ++i)
            if (g_hwid_raw[i] == 0)
                g_hwid_raw[i] = 0xff;
    }
    return hwid_raw_size;
}

static __forceinline void delta_xor_enc(uint8_t* buffer, SSIZE_T length)
{
    uint8_t last = 0;
    for (intptr_t i = 0; i < length; ++i)
    {
        uint8_t current = buffer[i];
        buffer[i] = (current - last) ^ static_cast<uint8_t>(length - i) ^ const_key[i % sizeof(const_key)];
        last = current;
    }
}

static __forceinline void delta_xor_dec(uint8_t* buffer, SSIZE_T length)
{
    uint8_t last = 0;
    for (intptr_t i = 0; i < length; ++i)
    {
        uint8_t delta = buffer[i] ^ static_cast<uint8_t>(length - i) ^ const_key[i % sizeof(const_key)];
        last = buffer[i] = delta + last;
    }
}

std::wstring c_config::SH_APPDATA;

c_config::c_config()
{
}

c_config::c_config(std::wstring dir_path, std::wstring config_name)
{
    CreateDirectoryW(dir_path.c_str(), NULL);
    this->full_path = dir_path + L"\\" + config_name;
    this->offset = 0;
    FILE* fp = _wfopen(this->full_path.c_str(), nnx::encoding::utf8to16(str("rb")).c_str());
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        SSIZE_T raw_size = ftell(fp);
        this->raw_stack.resize(raw_size);
        fseek(fp, 0, SEEK_SET);
        fread(&this->raw_stack[0], sizeof(this->raw_stack[0]), raw_size, fp);
        delta_xor_dec(&this->raw_stack[0], raw_size);
        fclose(fp);
        if (*reinterpret_cast<uint32_t*>(&this->raw_stack[raw_size - sizeof(uint32_t)]) == C_CONFIG_MAGIC)
            this->offset = raw_size - sizeof(uint32_t);
        this->raw_stack.resize(this->offset);
    }
}

void c_config::save()
{
    FILE* fp = _wfopen(this->full_path.c_str(), nnx::encoding::utf8to16(str("wb")).c_str());
    auto raw_stack_clone = this->raw_stack;
    size_t raw_stack_clone_size = raw_stack_clone.size();
    raw_stack_clone.resize(raw_stack_clone_size + sizeof(uint32_t));
    *reinterpret_cast<uint32_t*>(&raw_stack_clone[raw_stack_clone_size]) = C_CONFIG_MAGIC;
    raw_stack_clone_size = raw_stack_clone.size();
    delta_xor_enc(&raw_stack_clone[0], raw_stack_clone_size);
    fwrite(&raw_stack_clone[0], sizeof(raw_stack_clone[0]), raw_stack_clone_size, fp);
    fclose(fp);
}

int c_config::rm()
{
    return _wremove(this->full_path.c_str());
}

void c_config::push(int32_t value)
{
    uintptr_t new_offset = this->offset + sizeof(value);
    this->raw_stack.resize(new_offset);
    *reinterpret_cast<int32_t*>(&this->raw_stack[this->offset]) = value;
    this->offset = new_offset;
}

void c_config::push(bool value)
{
    uintptr_t new_offset = this->offset + sizeof(value);
    this->raw_stack.resize(new_offset);
    *reinterpret_cast<bool*>(&this->raw_stack[this->offset]) = value;
    this->offset = new_offset;
}

void c_config::push(size_t value)
{
    uintptr_t new_offset = this->offset + sizeof(value);
    this->raw_stack.resize(new_offset);
    *reinterpret_cast<size_t*>(&this->raw_stack[this->offset]) = value;
    this->offset = new_offset;
}

void c_config::push(const std::string& value)
{
    size_t value_len = value.length();
    this->raw_stack.insert(this->raw_stack.begin() + this->offset, value.begin(), value.end());
    this->offset += value_len;
    this->push(value_len);
}

int32_t c_config::pop_int32_t()
{
    this->offset -= sizeof(int32_t);
    if (this->offset < 0)
        return false;
    return *reinterpret_cast<int32_t*>(&this->raw_stack[this->offset]);
}

bool c_config::pop_bool()
{
    this->offset -= sizeof(bool);
    if (this->offset < 0)
        return false;
    return *reinterpret_cast<bool*>(&this->raw_stack[this->offset]);
}

size_t c_config::pop_size_t()
{
    this->offset -= sizeof(size_t);
    if (this->offset < 0)
        return 0;
    return *reinterpret_cast<size_t*>(&this->raw_stack[this->offset]);
}

std::string c_config::pop_string()
{
    size_t str_len = this->pop_size_t();
    this->offset -= str_len;
    if (this->offset < 0)
        return 0;
    const char* ret = reinterpret_cast<const char*>(&this->raw_stack[this->offset]);
    return { ret, &ret[str_len] };
}

void c_config::reset_offset()
{
    this->offset = this->raw_stack.size();
}