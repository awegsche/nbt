
#include <spdlog/spdlog.h>

using spdlog::info;
using spdlog::debug;
using spdlog::warn;
using spdlog::error;

#ifdef _MSC_VER
#include <intrin.h>
template<typename T, typename U> T __reint(U _val) { return *reinterpret_cast<T *>(&_val); }
template<typename T> unsigned long __swap4(T *_val)
{
    return _byteswap_ulong(*reinterpret_cast<unsigned long *>(_val));
}
template<typename T> unsigned long __swap4(const T *_val)
{
    return _byteswap_ulong(*reinterpret_cast<const unsigned long *>(_val));
}
template<typename T> unsigned short __swap2(T *_val)
{
    return _byteswap_ushort(*reinterpret_cast<unsigned short *>(_val));
}
template<typename T> unsigned short __swap2(const T *_val)
{
    return _byteswap_ushort(*reinterpret_cast<const unsigned short *>(_val));
}
template<typename T> unsigned long long __swap8(T *_val)
{
    return _byteswap_uint64(*reinterpret_cast<unsigned __int64 *>(_val));
}
template<typename T> unsigned long long __swap8(const T *_val)
{
    return _byteswap_uint64(*reinterpret_cast<const unsigned __int64 *>(_val));
}
#elif __GNUC__
template<typename T> uint32_t __swap4(T *_val) { return __builtin_bswap32(*reinterpret_cast<uint32_t *>(_val)); }
template<typename T> uint32_t __swap4(const T *_val)
{
    return __builtin_bswap32(*reinterpret_cast<const uint32_t *>(_val));
}
template<typename T> uint16_t __swap2(T *_val) { return __builtin_bswap16(*reinterpret_cast<uint16_t *>(_val)); }
template<typename T> uint16_t __swap2(const T *_val)
{
    return __builtin_bswap16(*reinterpret_cast<const uint16_t *>(_val));
}
template<typename T> uint64_t __swap8(T *_val) { return __builtin_bswap64(*reinterpret_cast<uint64_t *>(_val)); }
template<typename T> uint64_t __swap8(const T *_val)
{
    return __builtin_bswap64(*reinterpret_cast<const uint64_t *>(_val));
}

#endif

template<typename T> void push_swapped2(std::vector<unsigned char> &buffer, const T *value)
{
    static_assert(sizeof(T) == 2);
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 1));
    buffer.push_back(*reinterpret_cast<const unsigned char *>(value));
}

template<typename T> void push_swapped4(std::vector<unsigned char> &buffer, const T *value)
{
    static_assert(sizeof(T) == 4);
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 3));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 2));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 1));
    buffer.push_back(*reinterpret_cast<const unsigned char *>(value));
}

template<typename T> void push_swapped8(std::vector<unsigned char> &buffer, const T *value)
{
    static_assert(sizeof(T) == 8);
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 7));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 6));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 5));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 4));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 3));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 2));
    buffer.push_back(*(reinterpret_cast<const unsigned char *>(value) + 1));
    buffer.push_back(*reinterpret_cast<const unsigned char *>(value));
}
