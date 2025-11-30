#include "affine.h"
#include "file_utils.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <locale>
#include <codecvt>
#include <random>
#include <stdexcept>
#include <cstdint>

using namespace std;

enum class ObjectType {
    CONSOLE_TEXT = 1,
    TEXT_FILE = 2,
    IMAGE_FILE = 3,
    KEY_GENERATION = 4
};

uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int64_t modInverse(uint64_t a, uint64_t m) {
    a = a % m;
    
    for (uint64_t x = 1; x < m; x++) {
        if ((a * x) % m == 1) {
            return static_cast<int64_t>(x);
        }
    }
    return -1;
}

bool isSpecialSymbolWide(wchar_t c) {
    return (c >= L'!' && c <= L'/') || 
           (c >= L':' && c <= L'@') ||
           (c >= L'[' && c <= L'`') || 
           (c >= L'{' && c <= L'~');
}

uint64_t getSpecialSymbolIndex(wchar_t c) {
    if (c >= L'!' && c <= L'/') return static_cast<uint64_t>(c - L'!');                    // 0-14
    else if (c >= L':' && c <= L'@') return static_cast<uint64_t>((c - L':') + 15);        // 15-21
    else if (c >= L'[' && c <= L'`') return static_cast<uint64_t>((c - L'[') + 22);        // 22-27  
    else if (c >= L'{' && c <= L'~') return static_cast<uint64_t>((c - L'{') + 28);        // 28-32
    return static_cast<uint64_t>(-1);
}

wchar_t getSpecialSymbolFromIndex(uint64_t index) {
    if (index >= 0 && index <= 14) return L'!' + static_cast<wchar_t>(index);
    else if (index >= 15 && index <= 21) return L':' + static_cast<wchar_t>(index - 15);
    else if (index >= 22 && index <= 27) return L'[' + static_cast<wchar_t>(index - 22);
    else if (index >= 28 && index <= 32) return L'{' + static_cast<wchar_t>(index - 28);
    return L' ';
}

bool generateAffineKeys(uint64_t& a, uint64_t& b, bool forText, uint64_t min_a, uint64_t max_a, uint64_t min_b, uint64_t max_b) {
    random_device rd;
    mt19937_64 gen(rd());
    
    if (forText) {
        uniform_int_distribution<uint64_t> a_dist(min_a, max_a);
        
        int attempts = 0;
        const int max_attempts = 1000;
        
        do {
            a = a_dist(gen);
            attempts++;
        } while (!isValidTextKey(a) && attempts < max_attempts);
        
        if (attempts >= max_attempts) {
            wcout << L"Не удалось найти валидный ключ a за " << max_attempts << L" попыток!" << endl;
            return false;
        }
        
        uniform_int_distribution<uint64_t> b_dist(min_b, max_b);
        b = b_dist(gen);
        
    } else {
        uint64_t actual_max_a = (max_a > 255) ? 255 : max_a;
        uniform_int_distribution<uint64_t> a_dist(min_a, actual_max_a);
        
        int attempts = 0;
        const int max_attempts = 1000;
        
        do {
            a = a_dist(gen);
            attempts++;
        } while (!isValidBinaryKey(a) && attempts < max_attempts);
        
        if (attempts >= max_attempts) {
            wcout << L"Не удалось найти валидный ключ a за " << max_attempts << L" попыток!" << endl;
            return false;
        }
        
        uint64_t actual_max_b = (max_b > 255) ? 255 : max_b;
        uniform_int_distribution<uint64_t> b_dist(min_b, actual_max_b);
        b = b_dist(gen);
    }
    
    return true;
}

wstring affineEncryptWide(const wstring& text, uint64_t a, uint64_t b) {
    wstring result = L"";

    for (wchar_t c : text) {
        if (c == L' ') {
            result += L' ';
            continue;
        }
        
        if (c >= L'А' && c <= L'Я') {
            uint64_t x = static_cast<uint64_t>(c - L'А');
            uint64_t encrypted = (a * x + b) % 32;
            result += static_cast<wchar_t>(L'А' + encrypted);
        }
        else if (c >= L'а' && c <= L'я') {
            uint64_t x = static_cast<uint64_t>(c - L'а');
            uint64_t encrypted = (a * x + b) % 32;
            result += static_cast<wchar_t>(L'а' + encrypted);
        }
        else if (c >= L'A' && c <= L'Z') {
            uint64_t x = static_cast<uint64_t>(c - L'A');
            uint64_t encrypted = (a * x + b) % 26;
            result += static_cast<wchar_t>(L'A' + encrypted);
        }
        else if (c >= L'a' && c <= L'z') {
            uint64_t x = static_cast<uint64_t>(c - L'a');
            uint64_t encrypted = (a * x + b) % 26;
            result += static_cast<wchar_t>(L'a' + encrypted);
        }
        else if (c >= L'0' && c <= L'9') {
            uint64_t x = static_cast<uint64_t>(c - L'0');
            uint64_t encrypted = (a * x + b) % 10;
            result += static_cast<wchar_t>(L'0' + encrypted);
        }
        else if (isSpecialSymbolWide(c)) {
            uint64_t symbol_index = getSpecialSymbolIndex(c);
            if (symbol_index != static_cast<uint64_t>(-1)) {
                uint64_t encrypted_index = (a * symbol_index + b) % 33;
                wchar_t encrypted_char = getSpecialSymbolFromIndex(encrypted_index);
                result += encrypted_char;
            } else {
                result += c;
            }
        }
        else {
            result += c;
        }
    }
    return result;
}

wstring affineDecryptWide(const wstring& text, uint64_t a, uint64_t b) {
    wstring result = L"";

    int64_t a_inv_rus = modInverse(a, 32);
    int64_t a_inv_eng = modInverse(a, 26);
    int64_t a_inv_dig = modInverse(a, 10);
    int64_t a_inv_sym = modInverse(a, 33);

    for (wchar_t c : text) {
        if (c == L' ') {
            result += L' ';
            continue;
        }
        if (c >= L'А' && c <= L'Я') {
            uint64_t y = static_cast<uint64_t>(c - L'А');
            int64_t decrypted = (a_inv_rus * (static_cast<int64_t>(y) - static_cast<int64_t>(b) + 32)) % 32;
            if (decrypted < 0) decrypted += 32;
            result += static_cast<wchar_t>(L'А' + decrypted);
        }
        else if (c >= L'а' && c <= L'я') {
            uint64_t y = static_cast<uint64_t>(c - L'а');
            int64_t decrypted = (a_inv_rus * (static_cast<int64_t>(y) - static_cast<int64_t>(b) + 32)) % 32;
            if (decrypted < 0) decrypted += 32;
            result += static_cast<wchar_t>(L'а' + decrypted);
        }
        else if (c >= L'A' && c <= L'Z') {
            uint64_t y = static_cast<uint64_t>(c - L'A');
            int64_t decrypted = (a_inv_eng * (static_cast<int64_t>(y) - static_cast<int64_t>(b) + 26)) % 26;
            if (decrypted < 0) decrypted += 26;
            result += static_cast<wchar_t>(L'A' + decrypted);
        }
        else if (c >= L'a' && c <= L'z') {
            uint64_t y = static_cast<uint64_t>(c - L'a');
            int64_t decrypted = (a_inv_eng * (static_cast<int64_t>(y) - static_cast<int64_t>(b) + 26)) % 26;
            if (decrypted < 0) decrypted += 26;
            result += static_cast<wchar_t>(L'a' + decrypted);
        }
        else if (c >= L'0' && c <= L'9') {
            uint64_t y = static_cast<uint64_t>(c - L'0');
            int64_t decrypted = (a_inv_dig * (static_cast<int64_t>(y) - static_cast<int64_t>(b) + 10)) % 10;
            if (decrypted < 0) decrypted += 10;
            result += static_cast<wchar_t>(L'0' + decrypted);
        }
        else if (isSpecialSymbolWide(c)) {
            if (a_inv_sym != -1) {
                uint64_t symbol_index = getSpecialSymbolIndex(c);
                int64_t decrypted_index = (a_inv_sym * (static_cast<int64_t>(symbol_index) - static_cast<int64_t>(b) + 33)) % 33;
                if (decrypted_index < 0) decrypted_index += 33;
                wchar_t decrypted_char = getSpecialSymbolFromIndex(static_cast<uint64_t>(decrypted_index));
                result += decrypted_char;
            } else {
                result += c;
            }
        }
        else {
            result += c;
        }
    }
    return result;
}

vector<unsigned char> affineEncryptBinary(const vector<unsigned char>& data, uint64_t a, uint64_t b) {
    vector<unsigned char> result;
    result.reserve(data.size());
    
    for (unsigned char byte : data) {
        uint64_t encrypted = (a * static_cast<uint64_t>(byte) + b) % 256;
        result.push_back(static_cast<unsigned char>(encrypted));
    }
    
    return result;
}

vector<unsigned char> affineDecryptBinary(const vector<unsigned char>& data, uint64_t a, uint64_t b) {
    vector<unsigned char> result;
    result.reserve(data.size());
    
    int64_t a_inv = modInverse(a, 256);
    if (a_inv == -1) {
        return data; 
    }
    
    for (unsigned char byte : data) {
        int64_t decrypted = (a_inv * (static_cast<int64_t>(byte) - static_cast<int64_t>(b) + 256)) % 256;
        if (decrypted < 0) decrypted += 256;
        result.push_back(static_cast<unsigned char>(decrypted));
    }
    
    return result;
}

bool isValidTextKey(uint64_t a) {
    return gcd(a, 32) == 1 && gcd(a, 26) == 1 && 
           gcd(a, 10) == 1 && gcd(a, 33) == 1;
}

bool isValidBinaryKey(uint64_t a) {
    return gcd(a, 256) == 1;
}

void affine() {
    try {
        wcout << L"Выбран Аффинный шифр." << endl;
        
        wcout << L"Выберите объект для шифрования: " << endl;
        wcout << L"Нажмите 1 для ввода текста с консоли. " << endl;
        wcout << L"Нажмите 2 для чтения текста с файла." << endl;
        wcout << L"Нажмите 3 для чтения изображения." << endl;
        wcout << L"Нажмите 4 для генерации ключей." << endl;
        wcout << L"Введите номер выбранного объекта: ";
        
        int choice;
        wcin >> choice;
        wcin.ignore();

        ObjectType objectType = static_cast<ObjectType>(choice);

        uint64_t a = 0, b = 0;
        
        if (objectType != ObjectType::KEY_GENERATION) {
            wcout << L"Введите ключ a: ";
            wcin >> a;
            wcout << L"Введите ключ b: ";
            wcin >> b;
            wcin.ignore();
        }

        switch (objectType) {
            case ObjectType::CONSOLE_TEXT: {
                if (!isValidTextKey(a)) {
                    wcout << L"Ключ a невалиден!" << endl;
                    wcout << L"Ключ a должен быть взаимно простым с 32, 26, 10 и 33." << endl;
                    break;
                }

                wcout << L"Введите текст: ";
                wstring text;
                getline(wcin, text);

                if (text.empty()) {
                    wcout << L"Сообщение не может быть пустым!" << endl;
                    break;
                }

                wstring encrypted = affineEncryptWide(text, a, b);
                wstring decrypted = affineDecryptWide(encrypted, a, b);

                wcout << L"Зашифрованный текст: " << encrypted << endl;
                wcout << L"Расшифрованный текст: " << decrypted << endl;
                break;
            }
            
            case ObjectType::TEXT_FILE: {
                if (!isValidTextKey(a)) {
                    wcout << L"Ключ a невалиден!" << endl;
                    wcout << L"Ключ a должен быть взаимно простым с 32, 26, 10 и 33." << endl;
                    break;
                }

                wcout << L"Введите имя входного файла: ";
                wstring inputFilename;
                getline(wcin, inputFilename);
                
                wcout << L"Введите имя выходного файла для шифрования: ";
                wstring encryptedFilename;
                getline(wcin, encryptedFilename);
                
                wcout << L"Введите имя выходного файла для дешифрования: ";
                wstring decryptedFilename;
                getline(wcin, decryptedFilename);

                wstring originalText = readTextFile(inputFilename);
                if (originalText.empty()) {
                    wcout << L"Не удалось прочитать файл или файл пуст." << endl;
                    break;
                }

                wstring encryptedText = affineEncryptWide(originalText, a, b);
                if (writeTextFile(encryptedFilename, encryptedText)) {
                    wcout << L"Текст успешно зашифрован и записан в: " << encryptedFilename << endl;
                } else {
                    wcout << L"Ошибка записи зашифрованного файла." << endl;
                    break;
                }

                wstring decryptedText = affineDecryptWide(encryptedText, a, b);
                if (writeTextFile(decryptedFilename, decryptedText)) {
                    wcout << L"Текст успешно расшифрован и записан в: " << decryptedFilename << endl;
                } else {
                    wcout << L"Ошибка записи расшифрованного файла." << endl;
                }
                break;
            }
            
            case ObjectType::IMAGE_FILE: {
                if (!isValidBinaryKey(a)) {
                    wcout << L"Ключ a невалиден!" << endl;
                    wcout << L"Ключ a должен быть взаимно простым с 256." << endl;
                    break;
                }

                wcout << L"Введите имя входного изображения: ";
                wstring inputImage;
                getline(wcin, inputImage);
                
                wcout << L"Введите имя выходного файла для шифрования: ";
                wstring encryptedImage;
                getline(wcin, encryptedImage);
                
                wcout << L"Введите имя выходного файла для дешифрования: ";
                wstring decryptedImage;
                getline(wcin, decryptedImage);

                vector<unsigned char> originalData = readBinaryFile(inputImage);
                if (originalData.empty()) {
                    wcout << L"Не удалось прочитать изображение или файл пуст." << endl;
                    break;
                }

                vector<unsigned char> encryptedData = affineEncryptBinary(originalData, a, b);
                if (writeBinaryFile(encryptedImage, encryptedData)) {
                    wcout << L"Изображение зашифровано и записано в: " << encryptedImage << endl;
                } else {
                    wcout << L"Ошибка записи зашифрованного изображения." << endl;
                    break;
                }

                vector<unsigned char> decryptedData = affineDecryptBinary(encryptedData, a, b);
                if (writeBinaryFile(decryptedImage, decryptedData)) {
                    wcout << L"Изображение расшифровано и записано в: " << decryptedImage << endl;
                } else {
                    wcout << L"Ошибка записи расшифрованного изображения." << endl;
                }
                break;
            }
            
            case ObjectType::KEY_GENERATION: {
                wcout << L"Выберите тип ключей:" << endl;
                wcout << L"1 - для текстовых данных" << endl;
                wcout << L"2 - для бинарных данных" << endl;
                wcout << L"Введите выбор: ";
                
                int keyType;
                wcin >> keyType;
                wcin.ignore();

                wcout << L"Введите минимальное значение для ключа a: ";
                uint64_t min_a;
                wcin >> min_a;
                
                wcout << L"Введите максимальное значение для ключа a: ";
                uint64_t max_a;
                wcin >> max_a;
                wcin.ignore();

                if (min_a >= max_a) {
                    wcout << L"Минимальное значение должно быть меньше максимального для ключа a!" << endl;
                    break;
                }

                wcout << L"Введите минимальное значение для ключа b: ";
                uint64_t min_b;
                wcin >> min_b;
                
                wcout << L"Введите максимальное значение для ключа b: ";
                uint64_t max_b;
                wcin >> max_b;
                wcin.ignore();

                if (min_b >= max_b) {
                    wcout << L"Минимальное значение должно быть меньше максимального для ключа b!" << endl;
                    break;
                }

                if (keyType == 1) {
                    if (generateAffineKeys(a, b, true, min_a, max_a, min_b, max_b)) {
                        wcout << L"Сгенерированные ключи для текста:" << endl;
                        wcout << L"a = " << a << L", b = " << b << endl;
                    }
                } else if (keyType == 2) {
                    if (generateAffineKeys(a, b, false, min_a, max_a, min_b, max_b)) {
                        wcout << L"Сгенерированные ключи для бинарных данных:" << endl;
                        wcout << L"a = " << a << L", b = " << b << endl;
                    }
                } else {
                    wcout << L"Неверный выбор типа ключей!" << endl;
                }
                break;
            }
            
            default:
                wcout << L"Неверный выбор!" << endl;
                return;
        }
        
    } catch (const exception& e) {
        wcerr << L"Ошибка: " << e.what() << endl;
    } catch (...) {
        wcerr << L"Неизвестная ошибка!" << endl;
    }
}
