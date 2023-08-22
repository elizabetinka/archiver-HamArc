#include "archive.h"
#include "Hamming.h"
#include "writingToFile.h"

// Структура Архива
// Header для каждого файла (каждое из этих полей кодируется кодом Хемминга (3,1))
// Числовые значения вносятся в little-endian
// Поля:
// размер Header (2 байта) -> (6 байт)
// Размер в байтах выделенный для этого файла в архиве (4 байта) -> (12 байт)
// Размер имени файла (2 байта) -> (6 байт)
// Имя файла (strlen(arguments.files[0]) байт) -> (strlen(arguments.files[0])*3 байт)
// Количество информационных бит в блоке (2 байта) -> (6 байт)
// Количество дополнительных бит в блоке (2 байта) -> (6 байт)
// Остаток бит, слишком маленький для 1 блока (2 байта) -> (6 байт)
// Оставшиеся биты кодируются кодом Хемминга (3,1)

const uint8_t kSizeOfHeaderWithoutName = 14;
const uint8_t kSizeInHeaderAfterNameX3 = 18;
const uint8_t kSizeInHeaderBeforeNameX3 = 24;

uint32_t DivisionUp(uint32_t a, uint32_t b) {
    return static_cast<uint32_t>((a + b - 1) / b);
}

std::string CreatePath(const std::string& path) {
    return "../Ham_Arc/" + path + ".haf";
}

std::string CreatePathExtractFile(const std::string& name_archive_without_haf, char* name_file, uint16_t name_size) {
    for (int i = 0; i < name_size; i++) {
        if (name_file[i] == '/') {
            name_file[i] = '_';
        }
    }
    std::string ans = "Ham_Arc/" + name_archive_without_haf + "/";
    for (int i = 0; i < name_size; i++) {
        ans = ans + name_file[i];
    }
    return ans;
}

void AddFileToArchive(const Operations& arguments, char* file_name) {
    std::ofstream fout;
    fout.open(CreatePath(arguments.name_of_archive), std::ios::app);
    if (!fout.is_open()) {
        std::cerr << " Файл архива не открыт " << CreatePath(arguments.name_of_archive);
        exit(1);
    }
    bit_enter enter_bit_to_archive;
    bool bin_ch[8];

    std::ifstream fin(file_name);
    if (!fin.is_open()) {
        std::cerr << " Ошибка открытия файла " << file_name;
        exit(1);
    }
    uint32_t size_of_file;
    fin.seekg(0, std::ios_base::end);
    size_of_file = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    // подсчет Header
    const uint16_t size_of_header = (kSizeOfHeaderWithoutName + strlen(file_name)) * 3;
    const int count_full_part_of_inf_bit = ((size_of_file * 8) / arguments.info_bits);
    uint16_t ost_bits = (size_of_file * 8) % arguments.info_bits;
    size_of_file = ((size_of_file * 8) / static_cast<uint32_t>(arguments.info_bits)) *
                   (static_cast<uint32_t>(arguments.info_bits) + static_cast<uint32_t>(arguments.dop_bits)) +
                   static_cast<uint32_t>(3 * ost_bits);
    size_of_file = DivisionUp(size_of_file, 8);
    size_of_file = size_of_file + static_cast<uint32_t>(size_of_header);
    const uint16_t size_of_name = strlen(file_name);

    // ввод полей до имени
    WriteIbytesHamming31(size_of_header, fout, enter_bit_to_archive,2);
    WriteIbytesHamming31(size_of_file, fout, enter_bit_to_archive,4);
    WriteIbytesHamming31(size_of_name, fout, enter_bit_to_archive,2);

    // запись имени файла
    for (int i = 0; i < strlen(file_name); i++) {
        uint8_t q = static_cast<uint8_t>(file_name[i]);
        GetBin(q, bin_ch);
        for (int j = 0; j < 8; j++) {
            enter_bit_to_archive.enter(bin_ch[j], fout);
            enter_bit_to_archive.enter(bin_ch[j], fout);
            enter_bit_to_archive.enter(bin_ch[j], fout);
        }
    }

    // ввод полей после имени
    WriteIbytesHamming31(arguments.info_bits, fout, enter_bit_to_archive,2);
    WriteIbytesHamming31(arguments.dop_bits, fout, enter_bit_to_archive,2);
    WriteIbytesHamming31(ost_bits, fout, enter_bit_to_archive,2);

    struct bit_read read_bit_brom_file;
    bool mass_info_bits[arguments.info_bits];
    bool kod_slovo[arguments.dop_bits + arguments.info_bits];

    // чтение, кодирование и запись основной части файла
    for (int i = 0; i < count_full_part_of_inf_bit; i++) {
        for (int j = 0; j < arguments.info_bits; j++) {
            mass_info_bits[j] = read_bit_brom_file.read(fin);
        }
        koder(mass_info_bits, arguments.info_bits, arguments.dop_bits, kod_slovo);
        for (int j = 1; j <= (arguments.dop_bits + arguments.info_bits); j++) {
            enter_bit_to_archive.enter(kod_slovo[j], fout);
        }
    }
    // чтение и запись остаточных битов
    if (ost_bits != 0) {
        for (int i = 0; i < ost_bits; i++) {
            bool x = read_bit_brom_file.read(fin);
            enter_bit_to_archive.enter(x, fout);
            enter_bit_to_archive.enter(x, fout);
            enter_bit_to_archive.enter(x, fout);
        }
    }
    // дозаписываем, то что могло недозаписаться из-за некратности 8 битов
    enter_bit_to_archive.enterLast(fout);
    fout.close();
}

void ExtractOnefile(const Operations& arguments, std::ifstream& fin) {
    // позиция с которой мы пришли
    static uint64_t position = fin.tellg();
    struct bit_read read_bit_from_archive;

    // считывание Header до имени
    const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));
    const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin,4);
    const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));

    // считывание имени
    char name[size_of_name + 1];
    bool kod_slovo_hammin31[4];
    bool bin_ch[8];
    for (int k = 0; k < size_of_name; k++) {
        for (int j = 0; j < 8; j++) {
            for (int i = 1; i <= 3; i++) {
                kod_slovo_hammin31[i] = read_bit_from_archive.read(fin);
            }
            bin_ch[j] = dekoderHamming31(kod_slovo_hammin31);;
        }
        name[k] = GetNum(bin_ch);
    }

    // считывание Header после имени
    const uint16_t info_bits = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));
    const uint16_t dop_bits = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));
    const uint16_t ost_bits = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));

    struct bit_enter writer;
    std::ofstream fout;
    fout.open(CreatePathExtractFile(arguments.name_of_archive, name, size_of_name), std::ios::binary);
    if (!fout.is_open()) {
        std::cerr << " Ошибка открытия файла "<<CreatePathExtractFile(arguments.name_of_archive, name, size_of_name);
        exit(1);
    }

    const uint64_t count_kod_slov = ((size_of_file - size_of_header) * 8 - 3 * ost_bits) / (info_bits + dop_bits);
    bool kod_slovo[info_bits + dop_bits + 1];
    bool info_slovo[info_bits];

    // считываем кодовое слово, декодируем, записываем в файл
    for (uint64_t k = 0; k < count_kod_slov; k++) {
        for (int i = 1; i <= (info_bits + dop_bits); i++) {
            kod_slovo[i] = read_bit_from_archive.read(fin);
        }
        dekoder(info_slovo, info_bits, dop_bits, kod_slovo);
        for (int j = 0; j < info_bits; j++) {
            writer.enter(info_slovo[j], fout);
        }
    }

    // считываем остаточные закодированные биты, декодируем и записываем
    for (int i = 0; i < ost_bits; i++) {
        for (int j = 1; j <= 3; j++) {
            kod_slovo_hammin31[j] = read_bit_from_archive.read(fin);
        }
        bool info_bit_hammin31 = dekoderHamming31(kod_slovo_hammin31);
        writer.enter(info_bit_hammin31, fout);
    }
    // дозаписываем, то что могло недозаписаться из-за некратности 8 битов
    writer.enterLast(fout);

    // сдвигаем позицию на конец файла
    position = position + size_of_file;
    fin.seekg(position, std::ios_base::beg);
    fout.close();
}

void ExtractALlfiles(const Operations& arguments) {
    std::ifstream fin(CreatePath(arguments.name_of_archive), std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << " Ошибка открытия файла1";
        exit(1);
    }

    // создаем папку, в которую будем извлекать файлы
    std::filesystem::create_directory("Ham_Arc/" + arguments.name_of_archive);

    uint64_t size_of_archive = 0;
    fin.seekg(0, std::ios_base::end);
    size_of_archive = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    if (size_of_archive == 0) {
        return;
    }
    while (fin.good()) {
        ExtractOnefile(arguments, fin);
        if (fin.tellg() >= size_of_archive) {
            break;
        }
    }
    return;
}

void ExtractSomefiles(const Operations& arguments, char* file_name) {
    std::ifstream fin(CreatePath(arguments.name_of_archive), std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << " Ошибка открытия файла1";
        exit(1);
    }

    // создаем папку, в которую будем извлекать файлы
    std::filesystem::create_directory("Ham_Arc/" + arguments.name_of_archive);

    uint64_t size_of_archive = 0;
    fin.seekg(0, std::ios_base::end);
    size_of_archive = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    struct bit_read read_bit_from_archive;
    uint64_t position = 0;
    bool flag = true;
    while (flag == true) {
        const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));
        const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin,4);
        const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));

        // файл не тот, который нам нужен, пропускаем его
        if (size_of_name != strlen(file_name)) {
            position = position + size_of_file;
            fin.seekg(position, std::ios_base::beg);
            if (fin.tellg() >= size_of_archive) {
                break;
            }
        } else {
            char name[size_of_name + 1];
            bool kod_slovo_hammin31[4];
            bool bin_ch[8];
            for (uint64_t k = 0; k < size_of_name; k++) {
                for (int j = 0; j < 8; j++) {
                    for (int i = 1; i <= 3; i++) {
                        kod_slovo_hammin31[i] = read_bit_from_archive.read(fin);
                    }
                    bin_ch[j] = dekoderHamming31(kod_slovo_hammin31);;
                }
                name[k] = GetNum(bin_ch);
            }
            bool flag_ravn = true;
            for (uint64_t i = 0; i < size_of_name; i++) {
                if (name[i] != file_name[i]) {
                    flag_ravn = false;
                    // файл не тот, который нам нужен, пропускаем его
                    position = position + size_of_file;
                    fin.seekg(position, std::ios_base::beg);
                    if (fin.tellg() >= size_of_archive) {
                        flag = false;
                        break;
                    }
                    break;
                }
            }
            // файл тот, который нам нужен, извлекаем его
            if (flag_ravn == true) {
                uint64_t temp = fin.tellg();
                temp = temp + kSizeInHeaderAfterNameX3 - size_of_header;
                fin.seekg(temp, std::ios_base::beg);
                ExtractOnefile(arguments, fin);
                break;
            }
        }
    }
    return;
}

void DeleteSomefiles(const Operations& arguments, char* file_name) {
    std::ifstream fin(CreatePath(arguments.name_of_archive), std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << " Ошибка открытия архива " << CreatePath(arguments.name_of_archive);
        exit(1);
    }

    uint64_t size_of_archive = 0;
    fin.seekg(0, std::ios_base::end);
    size_of_archive = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    std::ofstream fout;
    std::string temp_dir=std::filesystem::temp_directory_path();
    fout.open(temp_dir+arguments.name_of_archive, std::ios::binary);
    if (!fout.is_open()) {
        std::cerr << " Файл вспомогательного архива не создан "<<temp_dir+arguments.name_of_archive;
        exit(1);
    }

    struct bit_read read_bit_from_archive;
    bool flag = true;
    while (flag == true) {
        const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));
        const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin,4);
        const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));

        // не тот файл, записываем его
        if (size_of_name != strlen(file_name)) {
            uint64_t temp = fin.tellg();
            temp = temp - kSizeInHeaderBeforeNameX3;
            // перемещаемся в начало файла, чтоб записать Header
            fin.seekg(temp, std::ios_base::beg);
            uint8_t ch;
            for (int j = 0; j < (size_of_file); j++) {
                ch = fin.get();
                fout.put(ch);
            }
            if (fin.tellg() >= size_of_archive) {
                break;
            }
        } else {
            char name[size_of_name + 1];
            bool kod_slovo_hammin31[4];
            bool bin_ch[8];
            for (int k = 0; k < size_of_name; k++) {
                for (int j = 0; j < 8; j++) {
                    for (int i = 1; i <= 3; i++) {
                        kod_slovo_hammin31[i] = read_bit_from_archive.read(fin);
                    }
                    bin_ch[j] = dekoderHamming31(kod_slovo_hammin31);;
                }
                name[k] = GetNum(bin_ch);
            }
            bool flag_ravn = true;
            for (int i = 0; i < size_of_name; i++) {
                if (name[i] != file_name[i]) {
                    flag_ravn = false;
                    // не тот файл, возвращаемся в начало, чтобы записать Header
                    uint64_t temp = fin.tellg();
                    temp = temp - size_of_header + kSizeInHeaderAfterNameX3;
                    fin.seekg(temp, std::ios_base::beg);
                    uint8_t ch;
                    for (int j = 0; j < (size_of_file); j++) {
                        ch = fin.get();
                        fout.put(ch);
                    }
                    if (fin.tellg() >= size_of_archive) {
                        flag = false;
                        break;
                    }
                    break;
                }
            }
            // тот файл, пропускаем его
            if (flag_ravn == true) {
                uint64_t temp = fin.tellg();
                temp = temp + size_of_file - size_of_header + kSizeInHeaderAfterNameX3;
                fin.seekg(temp, std::ios_base::beg);
                if (fin.tellg() >= size_of_archive) {
                    flag = false;
                }
            }
        }
    }
    // удаляем изначальный файл, перемещаем дополнительный
    remove(CreatePath(arguments.name_of_archive).c_str());
    if (rename((temp_dir+arguments.name_of_archive).c_str(), CreatePath(arguments.name_of_archive).c_str()) !=
        0) {
        std::cerr << " Ошибка перемещения файла " << std::endl;
    }
    return;
}

void CreateArchive(const Operations& arguments) {
    std::ofstream fout;
    fout.open(CreatePath(arguments.name_of_archive), std::ios::app);
    if (!fout.is_open()) {
        std::cerr << " Файл архива не создан "<<CreatePath(arguments.name_of_archive);
        exit(1);
    }
    if (arguments.files.empty()) {
        fout.close();
        return;
    }
    for (int i = 0; i < arguments.files.size(); i++) {
        AddFileToArchive(arguments, arguments.files[i]);
    }
}

void PrintOneName(std::ifstream& fin) {
    static uint64_t position = 0;
    struct bit_read read_bit_from_archive;

    // считываем Header до имени файла
    const uint16_t size_of_header = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));
    const uint32_t size_of_file = ReadIbytesHamming31(read_bit_from_archive, fin,4);
    const uint16_t size_of_name = static_cast<uint16_t>(ReadIbytesHamming31(read_bit_from_archive, fin,2));

    // считываем имя файла
    char name[size_of_name + 1];
    bool kod_slovo_hammin31[4];
    bool bin_ch[8];
    for (int k = 0; k < size_of_name; k++) {
        for (int j = 0; j < 8; j++) {
            for (int i = 1; i <= 3; i++) {
                kod_slovo_hammin31[i] = read_bit_from_archive.read(fin);
            }
            bin_ch[j] = dekoderHamming31(kod_slovo_hammin31);;
        }
        name[k] = GetNum(bin_ch);
    }
    for (int i = 0; i < size_of_name; i++) {
        std::cout << name[i];
    }
    // передвигаемся в конец файла
    position = position + size_of_file;
    fin.seekg(position, std::ios_base::beg);
}

void PrintNamesFile(const std::string& archive_name) {
    std::ifstream fin(CreatePath(archive_name), std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << " Ошибка открытия файла " << CreatePath(archive_name);
        exit(1);
    }

    uint64_t size_of_archive = 0;
    fin.seekg(0, std::ios_base::end);
    size_of_archive = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    if (size_of_archive == 0) {
        return;
    }
    while (fin.good()) {
        PrintOneName(fin);
        if (fin.tellg() >= size_of_archive) {
            break;
        }
        std::cout << std::endl;
    }
    return;
}

void MergeArchive(const std::string& name_of_archive, const std::string& second_name_of_archive, const std::string& third_name_of_archive) {
    std::ofstream fout;
    fout.open(CreatePath(third_name_of_archive), std::ios::app);
    if (!fout.is_open()) {
        std::cerr << " Файл архива не создан "<<CreatePath(third_name_of_archive);
        exit(1);
    }

    std::ifstream fin1(CreatePath(name_of_archive), std::ios::binary);
    if (!fin1.is_open()) {
        std::cerr << " Ошибка открытия файла " << CreatePath(name_of_archive);
        exit(1);
    }

    std::ifstream fin2(CreatePath(second_name_of_archive), std::ios::binary);
    if (!fin2.is_open()) {
        std::cerr << " Ошибка открытия файла " << CreatePath(second_name_of_archive);
        exit(1);
    }

    uint8_t q;
    while (fin1.good()) {
        q = fin1.get();
        if (fin1.good()) {
            fout.put(q);
        }
    }
    while (fin2.good()) {
        q = fin2.get();
        if (fin2.good()) {
            fout.put(q);
        }
    }
    fout.close();
}
