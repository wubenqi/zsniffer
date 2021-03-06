
#include "base/sys_string_conversions.h"

#ifdef _WIN32
#include <windows.h>


namespace base {

// Do not assert in this function since it is used by the asssertion code!
std::string SysWideToUTF8(const std::wstring& wide) {
    return SysWideToMultiByte(wide, CP_UTF8);
}

// Do not assert in this function since it is used by the asssertion code!
std::wstring SysUTF8ToWide(const std::string& utf8) {
    return SysMultiByteToWide(utf8, CP_UTF8);
}

std::string SysWideToNativeMB(const std::wstring& wide) {
    return SysWideToMultiByte(wide, CP_ACP);
}

std::wstring SysNativeMBToWide(const std::string& native_mb) {
    return SysMultiByteToWide(native_mb, CP_ACP);
}

// Do not assert in this function since it is used by the asssertion code!
std::wstring SysMultiByteToWide(const std::string& mb, unsigned int code_page) {
    if (mb.empty())
        return std::wstring();

    int mb_length = static_cast<int>(mb.length());
    // Compute the length of the buffer.
    int charcount = MultiByteToWideChar(code_page, 0,
            mb.data(), mb_length, NULL, 0);
    if (charcount == 0)
        return std::wstring();

    std::wstring wide;
    wide.resize(charcount);
    MultiByteToWideChar(code_page, 0, mb.data(), mb_length, &wide[0], charcount);

    return wide;
}

// Do not assert in this function since it is used by the asssertion code!
std::string SysWideToMultiByte(const std::wstring& wide, unsigned int code_page) {
    int wide_length = static_cast<int>(wide.length());
    if (wide_length == 0)
        return std::string();

    // Compute the length of the buffer we'll need.
    int charcount = WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
            NULL, 0, NULL, NULL);
    if (charcount == 0)
        return std::string();

    std::string mb;
    mb.resize(charcount);
    WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
            &mb[0], charcount, NULL, NULL);

    return mb;
}

std::string SysNativeMBToUTF8(const std::string& native_mb) {
	return SysWideToUTF8(SysNativeMBToWide(native_mb));
}

}  // namespace base
#endif
