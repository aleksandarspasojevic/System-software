#ifndef _LINKER_H_
#define _LINKER_H_


#include "SymbolTable.hpp"
#include "RelocationTable.hpp"
#include "Exceptions.hpp"
#include "Section.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <cmath>


enum class ObjectFileReadingType {
    ENTERING_FILE,
    SYMBOL_TABLE,
    SECTIONS,
    RELOCATION_TABLE
};

//Object file contains symbol table, sections and relocation table
struct Object_file{
    std::string name;

    SymbolTable symbol_table;
    RelocationTable relocation_table;
    std::map<std::string, Section> sections;
    std::vector<std::string> sections_order;   
    std::string current_section_name;     

    Object_file(std::string name): name(name){}

    void addSection(std::string section_name){
        sections.insert({section_name, Section(section_name)});
        sections_order.push_back(section_name);
        current_section_name = section_name;
    }
    
};


class Linker{
    public:
        Linker(bool hex_option, std::map<std::string, uint64_t> section_places);
        ~Linker();

        void decompose_input_files(std::vector<std::string> input_file_names);
        void Link(std::vector<std::string> input_file_names, std::string output_file_name);


    private:
        void fill_symbol_table();
        void undefined_symbol_check();
        void formSections();
        void check_sections_overlapping();
        void resolve_relocations();
        void write_output_file(std::ofstream& output_file); 

        uint32_t location_counter = 0;
        static std::ofstream log_file;

        std::map<std::string, Object_file> object_files;
        std::vector<std::string> object_files_order;
        std::map<std::pair<std::string, std::string>, Symbol*> linker_sections;   //section_name, file_name

        std::map<std::pair<std::string, std::string>, Section*> output_sections;
        std::vector<std::pair<std::string, std::string>> output_sections_order;

        SymbolTable linker_symbol_table;
        RelocationTable relocation_table;

        bool hex_option;
        std::map<std::string, uint64_t> section_places;

};



#endif 