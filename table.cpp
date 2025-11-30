#include "table.h"
#include "file_utils.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <locale>
#include <fstream>
#include <random>
#include <stdexcept>
#include <cstdint>
#include <sstream>

using namespace std;

enum class ObjectType {
    CONSOLE_TEXT = 1,
    TEXT_FILE = 2,
    IMAGE_FILE = 3,
    KEY_GENERATION = 4
};

vector<uint64_t> getColumnOrder(const wstring& key) {
    uint64_t keyLength = static_cast<uint64_t>(key.length());
    vector<pair<wchar_t, uint64_t>> keyPairs;
    for (uint64_t i = 0; i < keyLength; i++) {
        keyPairs.push_back({ key[i], i });
    }
    sort(keyPairs.begin(), keyPairs.end());

    vector<uint64_t> columnOrder(keyLength);
    for (uint64_t i = 0; i < keyLength; i++) {
        columnOrder[keyPairs[i].second] = i + 1;
    }
    return columnOrder;
}

wstring removeSpaces(const wstring& text) {
    wstring cleanText;
    for (wchar_t c : text) {
        if (c != L' ') {
            cleanText += c;
        }
    }
    return cleanText;
}

wstring addSpacesToGroups(const wstring& text, uint64_t groupSize) {
    if (groupSize <= 0) return text;

    wstring cleanText = removeSpaces(text);
    wstring result;

    for (size_t i = 0; i < cleanText.length(); i++) {
        if (i > 0 && i % groupSize == 0) {
            result += L' ';
        }
        result += cleanText[i];
    }
    return result;
}

vector<vector<wchar_t>> createMatrixByColumns(const wstring& text, uint64_t keyLength) {
    uint64_t textLength = static_cast<uint64_t>(text.length());
    uint64_t numRows = (textLength + keyLength - 1) / keyLength;

    vector<vector<wchar_t>> matrix(numRows, vector<wchar_t>(keyLength, L' '));
    uint64_t pos = 0;

    for (uint64_t j = 0; j < keyLength; j++) {
        for (uint64_t i = 0; i < numRows; i++) {
            if (pos < textLength) {
                matrix[i][j] = text[pos++];
            }
            else {
                matrix[i][j] = L'x';
            }
        }
    }
    return matrix;
}

vector<vector<wchar_t>> transposeColumns(const vector<vector<wchar_t>>& matrix,
    const vector<uint64_t>& columnOrder) {
    uint64_t numRows = static_cast<uint64_t>(matrix.size());
    uint64_t numCols = static_cast<uint64_t>(matrix[0].size());

    vector<vector<wchar_t>> transposed(numRows, vector<wchar_t>(numCols, L' '));

    for (uint64_t j = 0; j < numCols; j++) {
        uint64_t newCol = columnOrder[j] - 1;
        for (uint64_t i = 0; i < numRows; i++) {
            transposed[i][newCol] = matrix[i][j];
        }
    }
    return transposed;
}

vector<vector<wchar_t>> restoreColumns(const vector<vector<wchar_t>>& matrix,
    const vector<uint64_t>& columnOrder) {
    uint64_t numRows = static_cast<uint64_t>(matrix.size());
    uint64_t numCols = static_cast<uint64_t>(matrix[0].size());

    vector<vector<wchar_t>> restored(numRows, vector<wchar_t>(numCols, L' '));

    for (uint64_t j = 0; j < numCols; j++) {
        uint64_t originalCol = static_cast<uint64_t>(-1);
        for (uint64_t k = 0; k < numCols; k++) {
            if (columnOrder[k] == j + 1) {
                originalCol = k;
                break;
            }
        }
        for (uint64_t i = 0; i < numRows; i++) {
            restored[i][originalCol] = matrix[i][j];
        }
    }
    return restored;
}

wstring readMatrixByRows(const vector<vector<wchar_t>>& matrix) {
    wstring result;
    for (const auto& row : matrix) {
        for (wchar_t c : row) {
            result += c;
        }
    }
    return result;
}

wstring readMatrixByColumns(const vector<vector<wchar_t>>& matrix) {
    wstring result;
    uint64_t numRows = static_cast<uint64_t>(matrix.size());
    uint64_t numCols = static_cast<uint64_t>(matrix[0].size());

    for (uint64_t j = 0; j < numCols; j++) {
        for (uint64_t i = 0; i < numRows; i++) {
            result += matrix[i][j];
        }
    }

    while (!result.empty() && result.back() == L'x') {
        result.pop_back();
    }

    return result;
}

vector<unsigned char> encryptTableBinary(const vector<unsigned char>& data, const wstring& key) {
    vector<uint64_t> columnOrder = getColumnOrder(key);
    uint64_t keyLength = static_cast<uint64_t>(key.length());
    uint64_t dataLength = static_cast<uint64_t>(data.size());
    uint64_t numRows = (dataLength + keyLength - 1) / keyLength;

    vector<vector<unsigned char>> matrix(numRows, vector<unsigned char>(keyLength, 0));
    uint64_t pos = 0;

    for (uint64_t j = 0; j < keyLength; j++) {
        for (uint64_t i = 0; i < numRows; i++) {
            if (pos < dataLength) {
                matrix[i][j] = data[pos++];
            } else {
                matrix[i][j] = 0; 
            }
        }
    }

    vector<vector<unsigned char>> transposed(numRows, vector<unsigned char>(keyLength, 0));
    for (uint64_t j = 0; j < keyLength; j++) {
        uint64_t newCol = columnOrder[j] - 1;
        for (uint64_t i = 0; i < numRows; i++) {
            transposed[i][newCol] = matrix[i][j];
        }
    }

    vector<unsigned char> result;
    for (const auto& row : transposed) {
        for (unsigned char byte : row) {
            result.push_back(byte);
        }
    }
    return result;
}

vector<unsigned char> decryptTableBinary(const vector<unsigned char>& data, const wstring& key) {
    vector<uint64_t> columnOrder = getColumnOrder(key);
    uint64_t keyLength = static_cast<uint64_t>(key.length());
    uint64_t dataLength = static_cast<uint64_t>(data.size());
    uint64_t numRows = dataLength / keyLength;

    vector<vector<unsigned char>> matrix(numRows, vector<unsigned char>(keyLength));
    uint64_t pos = 0;
    for (uint64_t i = 0; i < numRows; i++) {
        for (uint64_t j = 0; j < keyLength; j++) {
            matrix[i][j] = data[pos++];
        }
    }

    vector<vector<unsigned char>> restored(numRows, vector<unsigned char>(keyLength, 0));
    for (uint64_t j = 0; j < keyLength; j++) {
        uint64_t originalCol = static_cast<uint64_t>(-1);
        for (uint64_t k = 0; k < keyLength; k++) {
            if (columnOrder[k] == j + 1) {
                originalCol = k;
                break;
            }
        }
        for (uint64_t i = 0; i < numRows; i++) {
            restored[i][originalCol] = matrix[i][j];
        }
    }

    vector<unsigned char> result;
    for (uint64_t j = 0; j < keyLength; j++) {
        for (uint64_t i = 0; i < numRows; i++) {
            result.push_back(restored[i][j]);
        }
    }

    while (!result.empty() && result.back() == 0) {
        result.pop_back();
    }

    return result;
}

wstring encryptTable(const wstring& key, const wstring& text) {
    wstring cleanText = removeSpaces(text);
    vector<uint64_t> columnOrder = getColumnOrder(key);

    auto matrix = createMatrixByColumns(cleanText, static_cast<uint64_t>(key.length()));
    auto transposedMatrix = transposeColumns(matrix, columnOrder);

    return readMatrixByRows(transposedMatrix);
}

wstring decryptTable(const wstring& key, const wstring& encryptedText) {
    wstring cleanEncryptedText = removeSpaces(encryptedText);
    vector<uint64_t> columnOrder = getColumnOrder(key);

    uint64_t keyLength = static_cast<uint64_t>(key.length());
    uint64_t textLength = static_cast<uint64_t>(cleanEncryptedText.length());
    uint64_t numRows = textLength / keyLength;

    vector<vector<wchar_t>> matrix(numRows, vector<wchar_t>(keyLength));
    uint64_t pos = 0;
    for (uint64_t i = 0; i < numRows; i++) {
        for (uint64_t j = 0; j < keyLength; j++) {
            matrix[i][j] = cleanEncryptedText[pos++];
        }
    }

    auto restoredMatrix = restoreColumns(matrix, columnOrder);
    return readMatrixByColumns(restoredMatrix);
}

wstring generateTableKey(uint64_t min_value, uint64_t max_value) {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> key_dist(min_value, max_value);
    
    uint64_t key = key_dist(gen);
    return to_wstring(key);
}

void table() {
    try {
        wcout << L"Выбрана Табличная перестановка с ключевым словом." << endl;
        
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

        wstring key;
        uint64_t groupSize = 0;

        if (objectType != ObjectType::KEY_GENERATION) {
            if (objectType == ObjectType::CONSOLE_TEXT || objectType == ObjectType::TEXT_FILE) {
                wcout << L"Введите размер группы символов (0 - без разбиения): ";
                wcin >> groupSize;
                wcin.ignore();
            }

            wcout << L"Введите ключевое слово: ";
            getline(wcin, key);

            if (key.empty()) {
                wcout << L"Ключевое слово не может быть пустым!" << endl;
                return;
            }
        }

        switch (objectType) {
            case ObjectType::CONSOLE_TEXT: {
                wcout << L"Введите текст для шифрования: ";
                wstring text;
                getline(wcin, text);

                if (text.empty()) {
                    wcout << L"Текст не может быть пустым!" << endl;
                    break;
                }

                wstring encrypted = encryptTable(key, text);
                wstring formattedEncrypted = addSpacesToGroups(encrypted, groupSize);

                wcout << L"Зашифрованный текст: " << formattedEncrypted << endl;

                wstring decrypted = decryptTable(key, formattedEncrypted);
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

                wstring encryptedText = encryptTable(key, originalText);
                wstring formattedEncryptedText = addSpacesToGroups(encryptedText, groupSize);
                
                if (writeTextFile(encryptedFilename, formattedEncryptedText)) {
                    wcout << L"Текст успешно зашифрован и записан в: " << encryptedFilename << endl;
                } else {
                    wcout << L"Ошибка записи зашифрованного файла." << endl;
                    break;
                }

                wstring decryptedText = decryptTable(key, formattedEncryptedText);
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

                vector<unsigned char> encryptedData = encryptTableBinary(originalData, key);
                if (writeBinaryFile(encryptedImage, encryptedData)) {
                    wcout << L"Изображение зашифровано и записано в: " << encryptedImage << endl;
                } else {
                    wcout << L"Ошибка записи зашифрованного изображения." << endl;
                    break;
                }

                vector<unsigned char> decryptedData = decryptTableBinary(encryptedData, key);
                if (writeBinaryFile(decryptedImage, decryptedData)) {
                    wcout << L"Изображение расшифровано и записано в: " << decryptedImage << endl;
                } else {
                    wcout << L"Ошибка записи расшифрованного изображения." << endl;
                }
                break;
            }
            
            case ObjectType::KEY_GENERATION: {
                wcout << L"Введите минимальное значение для ключа: ";
                uint64_t min_key;
                wcin >> min_key;
                
                wcout << L"Введите максимальное значение для ключа: ";
                uint64_t max_key;
                wcin >> max_key;
                wcin.ignore();

                if (min_key >= max_key) {
                    wcout << L"Минимальное значение должно быть меньше максимального!" << endl;
                    break;
                }

                wstring generated_key = generateTableKey(min_key, max_key);
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
