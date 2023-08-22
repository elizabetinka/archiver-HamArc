#pragma once

#include <iostream>
#include <fstream>
#include <math.h>
#include <cstring>

// структура, которая организовывает запись бита в файл
struct bit_enter{
    bool bitsets[8];
    int size_now;
    bit_enter(){
        for (int i=0;i<8;i++){
            bitsets[i]= false;
        }
        size_now=0;
    };
    void enter(bool x,std::ofstream & fout){
        bitsets[size_now]=x;
        size_now++;
        if (size_now==8){
            uint8_t q=0;
            int j=0;
            for (int i=7;i>=0;i--){
                if (bitsets[i]== true){
                    q=q+static_cast<uint8_t>(pow(2,j));
                }
                j++;
            }
            size_now=0;
            for (int i=0;i<8;i++){
                bitsets[i]= false;
            }
            fout.put(q);
        }
    };
    // организовывает вывод в файл оставшихся битов, не образующих байт
    void enterLast(std::ofstream& fout){
        if (size_now==0){
            return;
        }
        while(size_now!=8){
            bitsets[size_now]= false;
            size_now++;
        }
        uint8_t q=0;
        int j=0;
        for (int i=7;i>=0;i--){
            if (bitsets[i]== true){
                q=q+static_cast<uint8_t>(pow(2,j));
            }
            j++;
        }
        size_now=0;
        fout.put(q);
    };
};

// структура, которая позволяет получать бит из файла
struct bit_read{
    bool bitsets[8];
    int size_now;
    int current;
    bit_read(){
        for (int i=0;i<8;i++){
            bitsets[i]= false;
        }
        size_now=0;
        current=8;
    }
    bool read(std::ifstream& fout){
        if (size_now!=0){
            current++;
            size_now--;
            return bitsets[current-1];
        }
        else{
            size_now=7;
            current=1;
            uint8_t q;
            q=fout.get();
            for (int i=7;i>=0;i--){
                bitsets[i]=q%2;
                q=q/2;
            }
            return bitsets[0];
        }
    }
};

// перевод I байтового числа в little-endian
void ToNumbersI(uint32_t x, uint8_t numbers[], uint8_t byte);

// перевод в двоичную ссч
void GetBin(uint8_t x, bool bin_ch[]);

// пполучение числа из двоичной ссч
uint8_t GetNum(bool bin_ch[]);

// записывает в файл Iбайтовое число с требуемыми преобразованиями
void WriteIbytesHamming31(uint32_t x, std::ofstream& fout, bit_enter& enter_bit_to_archive, uint8_t byte);

// считывает из файла данные для I байтового числа декодирует их и переводит в десятичную ссч
uint32_t ReadIbytesHamming31(struct bit_read& read_bit_from_archive, std::ifstream& fin,uint8_t byte);

