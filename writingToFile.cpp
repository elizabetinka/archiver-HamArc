#include "writingToFile.h"
#include "Hamming.h"

void ToNumbersI(uint32_t x, uint8_t numbers[], uint8_t byte){
    for (int i=(byte-1);i>=0;i--){
        numbers[i]=static_cast<uint8_t>(x/(static_cast<uint32_t>(pow(256,i))));
        x=x%(static_cast<uint32_t>(pow(256,i)));
    }
}

void GetBin(uint8_t x, bool bin_ch[]) {
    for (int i = 7; i >= 0; i--) {
        bin_ch[i] = x % 2;
        x = x / 2;
    }
}

uint8_t GetNum(bool bin_ch[]) {
    uint8_t x = 0;
    int i = 0;
    for (int j = 7; j >= 0; j--) {
        if (bin_ch[j] == true) {
            x = x + static_cast<uint8_t>(pow(2, i));
        }
        i++;
    }
    return x;
}

void WriteIbytesHamming31(uint32_t x, std::ofstream& fout, bit_enter& enter_bit_to_archive, uint8_t byte) {
    uint8_t numbers[byte];
    bool bin_ch[8];
    ToNumbersI(x, numbers, byte);
    for (int i = 0; i < byte; i++) {
        GetBin(numbers[i], bin_ch);
        for (int j = 0; j < 8; j++) {
            enter_bit_to_archive.enter(bin_ch[j], fout);
            enter_bit_to_archive.enter(bin_ch[j], fout);
            enter_bit_to_archive.enter(bin_ch[j], fout);
        }
    }
}

uint32_t ReadIbytesHamming31(struct bit_read& read_bit_from_archive, std::ifstream& fin,uint8_t byte) {
    bool kod_slovo_hammin31[4];
    uint32_t x = 0;
    uint8_t numbers[byte];
    bool bin_ch[8];
    for (int k = 0; k < byte; k++) {
        for (int j = 0; j < 8; j++) {
            for (int i = 1; i <= 3; i++) {
                kod_slovo_hammin31[i] = read_bit_from_archive.read(fin);
            }

            bin_ch[j] = dekoderHamming31(kod_slovo_hammin31);
        }
        numbers[k] = GetNum(bin_ch);
    }
    for (int k = 0; k < byte; k++) {
        x = x + numbers[k] * static_cast<uint32_t>(pow(256, k));
    }
    return x;
}
