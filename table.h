#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <cstdint>

std::vector<uint64_t> getColumnOrder(const std::wstring& key);

std::wstring removeSpaces(const std::wstring& text);
std::wstring addSpacesToGroups(const std::wstring& text, uint64_t groupSize);

std::vector<std::vector<wchar_t>> createMatrixByColumns(const std::wstring& text, uint64_t keyLength);
std::vector<std::vector<wchar_t>> transposeColumns(const std::vector<std::vector<wchar_t>>& matrix, const std::vector<uint64_t>& columnOrder);
std::vector<std::vector<wchar_t>> restoreColumns(const std::vector<std::vector<wchar_t>>& matrix, const std::vector<uint64_t>& columnOrder);
std::wstring readMatrixByRows(const std::vector<std::vector<wchar_t>>& matrix);
std::wstring readMatrixByColumns(const std::vector<std::vector<wchar_t>>& matrix);

std::vector<unsigned char> encryptTableBinary(const std::vector<unsigned char>& data, const std::wstring& key);
std::vector<unsigned char> decryptTableBinary(const std::vector<unsigned char>& data, const std::wstring& key);

std::wstring encryptTable(const std::wstring& key, const std::wstring& text);
std::wstring decryptTable(const std::wstring& key, const std::wstring& encryptedText);

std::wstring generateTableKey(uint64_t min_value, uint64_t max_value);

void table();

#endif 
