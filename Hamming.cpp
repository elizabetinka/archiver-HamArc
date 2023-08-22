#include "Hamming.h"

bool CheckGoodArguments(uint16_t count_inf_bit, uint16_t& count_dop_bit) {
    if ((static_cast<uint16_t>(pow(2, count_dop_bit))) < (count_dop_bit + count_inf_bit + 1)) {
        return false;
    }
    while ((static_cast<uint16_t>(pow(2, count_dop_bit))) >= (count_dop_bit + count_inf_bit + 1)) {
        count_dop_bit--;
    }
    count_dop_bit++;
    return true;
}

uint16_t counterDopBits(uint16_t count_inf_bit) {
    if (count_inf_bit == 0) {
        return 0;
    }
    uint16_t r = 0;
    while ((static_cast<int>(pow(2, r))) < (r + count_inf_bit + 1)) {
        r++;
    }
    return r;
}

uint16_t counterInfoBits(uint16_t count_dop_bit) {
    if (count_dop_bit == 0) {
        return 0;
    }
    uint16_t i = static_cast<int>(pow(2, count_dop_bit)) - 1 - count_dop_bit;
    return i;
}

bool dekoderHamming31(bool KodSlovo[]) {
    int summ = static_cast<int>(KodSlovo[1]) + static_cast<int>(KodSlovo[2]) + static_cast<int>(KodSlovo[3]);
    if (summ >= 2) {
        return true;
    }
    return false;
}

// внимание!!! нумерация в KodSlovo с 1
void koder(bool inf_bits[], int count_inf_bits ,int count_dop_bits,bool KodSlovo[]) {
    for (int i = 1; i <= (count_dop_bits + count_inf_bits); i++) {
        KodSlovo[i] = true;
    }
    for (int i = 0; i < count_dop_bits; i++) {
        KodSlovo[static_cast<int>(pow(2, i))] = false;
    }
    int current_inf_bit = 0;
    for (int i = 1; i <= (count_dop_bits + count_inf_bits); i++) {
        if (KodSlovo[i] == true) {
            KodSlovo[i] = inf_bits[current_inf_bit];
            current_inf_bit++;
        }
    }
    for (int i = 0; i < count_dop_bits; i++) {
        int temp_sum = 0;
        int j = static_cast<int>(pow(2, i));
        for (j; j <= (count_dop_bits + count_inf_bits); j = j + 2 * (static_cast<int>(pow(2, i)))) {
            for (int k = 0; k < (static_cast<int>(pow(2, i))); k++) {
                if ((j + k) > (count_dop_bits + count_inf_bits)) {
                    break;
                }
                temp_sum = temp_sum + (static_cast<int>(KodSlovo[j + k]));
            }
        }
        KodSlovo[static_cast<int>(pow(2, i))] = temp_sum % 2;
    }
    return;
}

void getInfBits(bool inf_bits[], int count_inf_bits ,int count_dop_bits,bool KodSlovo[]) {
    bool index_no_kvadrat[count_inf_bits + count_dop_bits + 1];
    for (int i = 0; i <= (count_inf_bits + count_dop_bits); i++) {
        index_no_kvadrat[i] = true;
    }
    for (int i = 0; i < count_dop_bits; i++) {
        index_no_kvadrat[static_cast<int>(pow(2, i))] = false;
    }
    int j = 0;
    for (int i = 1; i <= (count_inf_bits + count_dop_bits); i++) {
        if (index_no_kvadrat[i] == true) {
            inf_bits[j] = KodSlovo[i];
            j++;
        }
    }
}

bool checkGoodKodslovo(int count_inf_bits ,int count_dop_bits,bool KodSlovo[],std::vector<int>& mistakes_dop_bits) {
    for (int i = 0; i < count_dop_bits; i++) {
        int temp_sum = 0;
        int j = static_cast<int>(pow(2, i));
        for (j; j <= (count_dop_bits + count_inf_bits); j = j + 2 * (static_cast<int>(pow(2, i)))) {
            for (int k = 0; k < (static_cast<int>(pow(2, i))); k++) {
                if ((j + k) > (count_dop_bits + count_inf_bits)) {
                    break;
                }
                if ((j + k) != static_cast<int>(pow(2, i))) {
                    temp_sum = temp_sum + (static_cast<int>(KodSlovo[j + k]));
                }
            }
        }
        // проверяем правильный ли контрольный бит
        if ((KodSlovo[static_cast<int>(pow(2, i))]) != (temp_sum % 2)) {
            mistakes_dop_bits.push_back(static_cast<int>(pow(2, i)));
        }
    }
    if (mistakes_dop_bits.size() <= 1) {
        return true;
    } else {
        return false;
    }
}

void dekoder(bool inf_bits[], int count_inf_bits ,int count_dop_bits,bool KodSlovo[]) {
    std::vector<int> mistakes_dop_bits;
    // нет ошибок
    if (checkGoodKodslovo(count_inf_bits, count_dop_bits, KodSlovo, mistakes_dop_bits) == true) {
        getInfBits(inf_bits, count_inf_bits, count_dop_bits, KodSlovo);
        return;
    }
    int mistake_index = 0;
    for (int i = 0; i < mistakes_dop_bits.size(); i++) {
        mistake_index = mistake_index + mistakes_dop_bits[i];
    }
    if (mistake_index > (count_inf_bits + count_dop_bits)) {
        getInfBits(inf_bits, count_inf_bits, count_dop_bits, KodSlovo);
        std::cerr << "более 2 ошибок, не могу исправить" << std::endl;
        exit(1);
    }
    KodSlovo[mistake_index] = (KodSlovo[mistake_index] + 1) % 2;
    getInfBits(inf_bits, count_inf_bits, count_dop_bits, KodSlovo);
    mistakes_dop_bits.clear();
    if (checkGoodKodslovo(count_inf_bits, count_dop_bits, KodSlovo, mistakes_dop_bits) == true) {
        return;
    } else {
        std::cerr << "более 2 ошибок, не могу исправить" << std::endl;
        exit(1);
    }
}
