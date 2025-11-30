#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>

std::string ws2s(const std::wstring& ws);
std::wstring s2ws(const std::string& s);

std::wstring readTextFile(const std::wstring& filename);
bool writeTextFile(const std::wstring& filename, const std::wstring& content);

std::vector<unsigned char> readBinaryFile(const std::wstring& filename);
bool writeBinaryFile(const std::wstring& filename, const std::vector<unsigned char>& data);

#endif
