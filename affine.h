#ifndef AFFINE_H
#define AFFINE_H

#include <string>
#include <vector>
#include <cstdint>

uint64_t gcd(uint64_t a, uint64_t b);
int64_t modInverse(uint64_t a, uint64_t m);

bool isSpecialSymbolWide(wchar_t c);
uint64_t getSpecialSymbolIndex(wchar_t c);
wchar_t getSpecialSymbolFromIndex(uint64_t index);

std::wstring affineEncryptWide(const std::wstring& text, uint64_t a, uint64_t b);
std::wstring affineDecryptWide(const std::wstring& text, uint64_t a, uint64_t b);
std::vector<unsigned char> affineEncryptBinary(const std::vector<unsigned char>& data, uint64_t a, uint64_t b);
std::vector<unsigned char> affineDecryptBinary(const std::vector<unsigned char>& data, uint64_t a, uint64_t b);

bool isValidTextKey(uint64_t a);
bool isValidBinaryKey(uint64_t a);

bool generateAffineKeys(uint64_t& a, uint64_t& b, bool forText, uint64_t min_a, uint64_t max_a, uint64_t min_b, uint64_t max_b);

void affine();

#endif 
