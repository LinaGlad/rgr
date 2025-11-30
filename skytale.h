#ifndef SKYTALE_H
#define SKYTALE_H

#include <string>
#include <vector>
#include <cstdint>

std::wstring transformSkytaleConsole(const std::wstring& text, uint64_t key, bool encrypt);
std::wstring transformSkytaleText(const std::wstring& text, uint64_t key, bool encrypt);
std::vector<unsigned char> transformSkytaleBinary(const std::vector<unsigned char>& data, uint64_t key, bool encrypt);

uint64_t generateSkytaleKey(uint64_t min_value, uint64_t max_value);

void skytale();

#endif 
