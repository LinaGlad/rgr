#include <iostream>
#include <string>
#include <locale>
#include "skytale.h"
#include "affine.h"
#include "table.h"

using namespace std;

enum class Cipher {
    EXIT,
    SKYTALE,
    AFFINE,
    TABLE,
    INVALID
};

Cipher getCipherFromString(const wstring& str) {
    if (str == L"0") return Cipher::EXIT;
    if (str == L"1") return Cipher::SKYTALE;
    if (str == L"2") return Cipher::AFFINE;
    if (str == L"3") return Cipher::TABLE;
    return Cipher::INVALID;
}

void displayMenu() {
    wcout << L"Выберите шифр: " << endl;
    wcout << L"Нажмите 1 для выбора шифра Скитала." << endl;
    wcout << L"Нажмите 2 для выбора Аффинного шифра." << endl;
    wcout << L"Нажмите 3 для выбора Табличной шифровки с ключевым словом." << endl;
    wcout << L"Нажмите 0 для выхода из программы." << endl;
    wcout << L"Введите номер выбранного шифра: ";
}

int main() {
    locale::global(locale(""));
    wcin.imbue(locale());
    wcout.imbue(locale());
    
    wstring correctPassword = L"АБ421";
    wcout << L"Введите пароль: ";
    wstring passwordOption;
    wcin >> passwordOption;

    if (correctPassword != passwordOption) {
        wcout << L"Неверный пароль!" << endl;
        return -1;
    }
    wcout << endl;
    displayMenu();
    wstring number;
    wcin >> number;
    wcin.ignore();

    while (number != L"0") {
        wcout << endl;
        Cipher choice = getCipherFromString(number);

        switch (choice) {
        case Cipher::SKYTALE:
            skytale();
            break;

        case Cipher::AFFINE:
            affine();
            break;

        case Cipher::TABLE:
            table();
            break;

        default:
            wcout << L"Нет такого номера." << endl;
            break;
        }
        
        wcout << endl;
        displayMenu();
        wcin >> number;
        wcin.ignore();
    }

    wcout << L"Выход из программы." << endl;
    return 0;
}
