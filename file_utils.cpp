#include "file_utils.h"
#include <fstream>
#include <vector>
#include <locale>
#include <codecvt>  
#include <string>

using namespace std;

string ws2s(const wstring& ws) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(ws);
}

wstring s2ws(const string& s) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(s);
}

wstring readTextFile(const wstring& filename) {
    string narrow_filename = ws2s(filename);
    ifstream file(narrow_filename, ios::binary);
    if (!file.is_open()) return L"";
    
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    
    if (fileSize == 0) return L"";
    
    vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();
    
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    try {
        return converter.from_bytes(string(buffer.data(), fileSize));
    } catch (...) {
        return L"";
    }
}

bool writeTextFile(const wstring& filename, const wstring& content) {
    string narrow_filename = ws2s(filename);
    ofstream file(narrow_filename, ios::binary);
    if (!file.is_open()) return false;
    
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    string utf8_content = converter.to_bytes(content);
    file.write(utf8_content.data(), utf8_content.size());
    file.close();
    return true;
}

vector<unsigned char> readBinaryFile(const wstring& filename) {
    string narrow_filename = ws2s(filename);
    ifstream file(narrow_filename, ios::binary);
    if (!file.is_open()) return {};
    
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    
    if (fileSize == 0) return {};
    
    vector<unsigned char> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();
    return buffer;
}

bool writeBinaryFile(const wstring& filename, const vector<unsigned char>& data) {
    string narrow_filename = ws2s(filename);
    ofstream file(narrow_filename, ios::binary);
    if (!file.is_open()) return false;
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    return true;
}
