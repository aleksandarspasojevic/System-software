#ifndef _SYMBOLTABLE_
#define _SYMBOLTABLE_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>


struct Symbol{
  static int next_number;

  std::string name;
  std::string section_name;
  int value;
  bool is_global;
  bool is_extern;
  int number;
  int size;
  bool defined; 
  std::string file_name;

  Symbol():name(""), section_name(""), value(0), is_global(false), is_extern(false), number(-1), size(0), defined(false), file_name(""){}

  Symbol(std::string name, std::string section_name, int value, bool is_global, bool is_extern, int size, bool defined){
    this->set_symbol(name, section_name, value, is_global, is_extern, size, defined, "");
    this->number = next_number++;
  }

  void set_symbol(std::string name, std::string section_name, int value, bool is_global, bool is_extern, int size, bool defined, std::string file_name=""){
    this->name = name;
    this->section_name = section_name;
    this->value = value;
    this->is_global = is_global;
    this->is_extern = is_extern;
    this->size = size;
    this->defined = defined;
    this->file_name = file_name;
  }

  bool is_section(){
    return this->size != -1;
  }

  static Symbol* deserialize_symbol(std::string line) {
    std::stringstream ss(line);
    std::string token;
    Symbol symbol;

    ss>>token; symbol.name = token;
    ss>>token; symbol.section_name = token;
    ss>>token; symbol.value = std::stoull(token, nullptr, 0);
    ss>>token; symbol.is_global = std::stoull(token, nullptr, 0);
    ss>>token; symbol.is_extern = std::stoull(token, nullptr, 0);
    ss>>token; symbol.number = std::stoull(token, nullptr, 0);
    ss>>token; symbol.size = std::stoull(token, nullptr, 0);
    ss>>token; symbol.defined = std::stoull(token, nullptr, 0);

    return new Symbol(symbol);
  }

  friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);

};


class SymbolTable{
  public:
    std::map<std::string, Symbol*> table;

    void add_symbol(Symbol* symbol);
    Symbol* get_symbol_by_name(const std::string& symbol_name);
    bool update_section_size(std::string symbol_name, uint32_t size);
    void remove_symbol(std::string symbol_name);
    void clear_table();

    friend std::ostream& operator<<(std::ostream& os, const SymbolTable& symbol_table);

    SymbolTable();
    ~SymbolTable();

};




#endif