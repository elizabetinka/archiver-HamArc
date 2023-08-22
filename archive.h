#pragma once

#include <iostream>
#include <fstream>
#include <math.h>
#include <cstring>
#include <filesystem>
#include "parsing.h"

// деление на число с округлением наверх
uint32_t DivisionUp(uint32_t a, uint32_t b);

// создает путь для архива
std::string CreatePath(const std::string& path);

// создает путь, по которому будет лежать файл после извлечения
std::string CreatePathExtractFile(const std::string& name_archive_without_haf, char* name_file, uint16_t name_size);

void AddFileToArchive(const Operations& arguments,char* file_name);

// извлекает следующий файл (от позиции, в которой мы находимся в архиве)
void ExtractOnefile(const Operations& arguments,std::ifstream& fin);

void ExtractALlfiles(const Operations& arguments);

// извлекает файл по его имени
void ExtractSomefiles(const Operations& arguments,char* file_name);

void DeleteSomefiles(const Operations& arguments,char* file_name);

void MergeArchive(const std::string& name_of_archive,const std::string& second_name_of_archive,const std::string& third_name_of_archive);

void CreateArchive(const Operations& arguments);

// выводит следующее имя файла (от позиции, в которой мы находимся)
void PrintOneName(std::ifstream& fin);

void PrintNamesFile(const std::string& archive_name);


