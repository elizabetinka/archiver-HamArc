#pragma once

#include <iostream>
//#include <cstring>
#include <vector>
#include <math.h>

// проверяет подходит ли данное соотношение инф/доп битов.
// Если доп битов указано слишком много-уменьшает их до минимального возможного
bool CheckGoodArguments(uint16_t count_inf_bit,uint16_t& count_dop_bit);

// если не указано --dop, высчитывает его
uint16_t counterDopBits(uint16_t count_inf_bit);

// если не указано --info, высчитывает его
uint16_t counterInfoBits(uint16_t count_dop_bit);

void koder(bool inf_bits[], int count_inf_bits ,int count_dop_bits,bool KodSlovo[]);

// получение информационного слова из кодового
void getInfBits(bool inf_bits[], int count_inf_bits ,int count_dop_bits,bool KodSlovo[]);

// проверка все ли биты правильные в кодовом слове
bool checkGoodKodslovo(int count_inf_bits ,int count_dop_bits,bool KodSlovo[],std::vector<int>& mistakes_dop_bits);

void dekoder(bool inf_bits[], int count_inf_bits ,int count_dop_bits,bool KodSlovo[]);

bool dekoderHamming31(bool KodSlovo[]);
