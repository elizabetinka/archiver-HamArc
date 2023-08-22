#pragma once

#include <iostream>
#include <cstring>
#include <vector>

enum Options {
    Create = 0, List = 1, Extract = 2, Append = 3, Delete = 4, Concatenate = 5 , InfoBits = 6, DopBits = 7
};

// Обязательные параметры:
// info_bits или dop_bits при Create,Append
struct Operations {
    std::string name_of_archive;
    std::vector<char*> files;
    // количество информационных бит в одном блоке
    uint16_t info_bits;
    // количество избыточных бит в одном блоке
    uint16_t dop_bits;
    // для extract
    bool all_files;
    // для concatenate
    std::string second_name_of_archive;
    std::string third_name_of_archive;
    // проверяет, какие требуются операции(True-вводились, False-нет)
    bool flag[8];
};

// получает имя архива, имена файлов и аргументы для кодировки и декодировки
void GetNames(int i,int argc, char** argv, Operations& arguments);

void Parsing(int argc, char** argv, Operations& arguments);

// введены данные, проверка их или высчитывание недостающих
void CheckDopArguments(Operations& arguments);

