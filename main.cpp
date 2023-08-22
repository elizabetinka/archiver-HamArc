//
// Created by Елизавета Кравченкова on 03.11.2022.
//

#include "parsing.h"
#include "archive.h"

int main(int argc, char **argv){
    // создаем папку в которой будут храниться все данные об архивах
    std::filesystem::create_directory("Ham_Arc");

    struct Operations arguments;
    Parsing(argc,argv,arguments);

    for (int i=0;i<6;i++){
        if (arguments.flag[Create]== true){
            CreateArchive(arguments);
            break;
        }
        if (arguments.flag[List]== true){
            PrintNamesFile(arguments.name_of_archive);
            break;
        }
        if (arguments.flag[Extract]== true){
            if (arguments.all_files== true){
                ExtractALlfiles(arguments);
            }
            else{
                ExtractSomefiles(arguments,arguments.files[0]);
            }
            break;
        }
        if (arguments.flag[Append]== true){
            AddFileToArchive(arguments,arguments.files[0]);
            break;
        }
        if (arguments.flag[Delete]== true){
            DeleteSomefiles(arguments,arguments.files[0]);
            break;
        }
        if (arguments.flag[Concatenate]== true){
            MergeArchive(arguments.name_of_archive,arguments.second_name_of_archive,arguments.third_name_of_archive);
            break;
        }
    }
    return 0;
}
