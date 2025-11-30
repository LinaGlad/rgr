#include "skytale.h"
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

wstring transformSkytaleConsole(const wstring& text, uint64_t key, bool encrypt) {
    if (key <= 0) return text;

    uint64_t length = static_cast<uint64_t>(text.length());
    uint64_t columns = (length - 1) / key + 1;

    wstring result(key*columns, L' ');
    for (uint64_t i = 0; i < length; i++) {
        uint64_t index = key * (i % columns) + i / columns;
        if (encrypt) {
            result[index] = text[i];
        } else {
            result[i] = text[index];
        }
    }
    return result;
}

wstring transformSkytaleText(const wstring& text, uint64_t key, bool encrypt) {
    if (key <= 0) return text;

    uint64_t length = static_cast<uint64_t>(text.length());
    if (length == 0) return text;
    
    uint64_t columns = (length + key - 1) / key;
    uint64_t matrix_size = key * columns;
    uint64_t padding = matrix_size - length;
    
    wstring work_text = text;
    
    if (encrypt) {
        work_text += wstring(padding, L' ');
    }
    wstring result(work_text.length(), L' ');
    for (uint64_t i = 0; i < work_text.length(); i++) {
        uint64_t index = key * (i % columns) + i / columns;
        if (encrypt) {
            result[index] = work_text[i];
        } else {
            result[i] = work_text[index];
        }
    }
    if (!encrypt) {
        result = result.substr(0, static_cast<size_t>(length));
    }
    return result;
}

vector<unsigned char> transformSkytaleBinary(const vector<unsigned char>& data, uint64_t key, bool encrypt) {
    if (key <= 0 || data.empty()) return data;

    uint64_t length = static_cast<uint64_t>(data.size());
    uint64_t columns = (length + key - 1) / key;
    uint64_t matrix_size = key * columns;
    uint64_t padding = matrix_size - length;
    
    vector<unsigned char> work_data = data;
    
    if (encrypt) {
        work_data.insert(work_data.end(), static_cast<size_t>(padding), 0);
    }
    vector<unsigned char> result(work_data.size(), 0);
    for (uint64_t i = 0; i < static_cast<uint64_t>(work_data.size()); i++) {
        uint64_t index = key * (i % columns) + i / columns;
        if (encrypt) {
            result[static_cast<size_t>(index)] = work_data[static_cast<size_t>(i)];
        } else {
            result[static_cast<size_t>(i)] = work_data[static_cast<size_t>(index)];
        }
    }
    if (!encrypt) {
        result.resize(static_cast<size_t>(length));
    }
    return result;
}

uint64_t generateSkytaleKey(uint64_t min_value, uint64_t max_value) {
    try {
        random_device rd;
        mt19937_64 gen(rd());
        uniform_int_distribution<uint64_t> dis(min_value, max_value);
        
        return dis(gen);
    } catch (const exception& e) {
        wcerr << L"Ошибка при генерации ключа: " << e.what() << endl;
        throw;
    }
}

void skytale() {
    try {
        wcout << L"Выбран шифр Скитала." << endl;
        
        wcout << L"Выберите объект для шифрования: " << endl;
        wcout << L"Нажмите 1 для ввода текста с консоли. " << endl;
        wcout << L"Нажмите 2 для чтения текста с файла." << endl;
        wcout << L"Нажмите 3 для чтения изображения." << endl;
        wcout << L"Нажмите 4 для генерации ключа." << endl;
        wcout << L"Введите номер выбранного объекта: ";
        
        int choice;
        wcin >> choice;
        wcin.ignore();

        ObjectType objectType = static_cast<ObjectType>(choice);

        uint64_t key = 0;
        
        if (objectType != ObjectType::KEY_GENERATION) {
            wcout << L"Введите ключ: ";
            wcin >> key;
            wcin.ignore();

            if (key <= 0) {
                wcout << L"Ключ должен быть положительным числом!" << endl;
                return;
            }
        }

        switch (objectType) {
            case ObjectType::CONSOLE_TEXT: {
                wcout << L"Введите сообщение: ";
                wstring message;
                getline(wcin, message);

                if (message.empty()) {
                    wcout << L"Сообщение не может быть пустым!" << endl;
                    break;
                }

                wstring encrypted = transformSkytaleConsole(message, key, true);
                wcout << L"Зашифрованный текст: " << encrypted << endl;

                wstring decrypted = transformSkytaleConsole(encrypted, key, false);
                wcout << L"Расшифрованный текст: " << decrypted << endl;
                break;
            }
            
            case ObjectType::TEXT_FILE: {
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

                wstring encryptedText = transformSkytaleText(originalText, key, true);
                if (writeTextFile(encryptedFilename, encryptedText)) {
                    wcout << L"Текст успешно зашифрован и записан в: " << encryptedFilename << endl;
                } else {
                    wcout << L"Ошибка записи зашифрованного файла." << endl;
                    break;
                }

                wstring decryptedText = transformSkytaleText(encryptedText, key, false);
                if (writeTextFile(decryptedFilename, decryptedText)) {
                    wcout << L"Текст успешно расшифрован и записан в: " << decryptedFilename << endl;
                } else {
                    wcout << L"Ошибка записи расшифрованного файла." << endl;
                }
                break;
            }
            
            case ObjectType::IMAGE_FILE: {
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

                vector<unsigned char> encryptedData = transformSkytaleBinary(originalData, key, true);
                if (writeBinaryFile(encryptedImage, encryptedData)) {
                    wcout << L"Изображение зашифровано и записано в: " << encryptedImage << endl;
                } else {
                    wcout << L"Ошибка записи зашифрованного изображения." << endl;
                    break;
                }

                vector<unsigned char> decryptedData = transformSkytaleBinary(encryptedData, key, false);
                if (writeBinaryFile(decryptedImage, decryptedData)) {
                    wcout << L"Изображение расшифровано и записано в: " << decryptedImage << endl;
                } else {
                    wcout << L"Ошибка записи расшифрованного изображения." << endl;
                }
                break;
            }
            
            case ObjectType::KEY_GENERATION: {
                wcout << L"Введите минимальное значение ключа: ";
                uint64_t min_key;
                wcin >> min_key;
                
                wcout << L"Введите максимальное значение ключа: ";
                uint64_t max_key;
                wcin >> max_key;
                wcin.ignore();

                if (min_key >= max_key) {
                    wcout << L"Минимальное значение должно быть меньше максимального!" << endl;
                    break;
                }

                uint64_t generated_key = generateSkytaleKey(min_key, max_key);
                wcout << L"Сгенерированный ключ: " << generated_key << endl;
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
