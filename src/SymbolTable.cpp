#include "../inc/SymbolTable.hpp"

int Symbol::next_number = 0;

SymbolTable::SymbolTable(){
  
}

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
  os << std::left << "  " <<
    std::setw(25) << symbol.name <<
    std::setw(25) << symbol.section_name <<
    std::setw(15) << symbol.value <<
    std::setw(15) << symbol.is_global <<
    std::setw(15) << symbol.is_extern <<
    std::setw(15) << symbol.number <<
    std::setw(15) << symbol.size <<
    std::setw(15) << symbol.defined <<
    std::setw(50) << symbol.file_name <<
    std::endl;

  return os;
}


void SymbolTable::add_symbol(Symbol* symbol){
  this->table.insert({symbol->name, symbol});
}


Symbol* SymbolTable::get_symbol_by_name(const std::string& symbol_name) {
  auto it = table.find(symbol_name);
  return (it != table.end()) ? it->second : nullptr;
}


bool SymbolTable::update_section_size(std::string symbol_name, uint32_t size){
  std::map<std::string, Symbol*>::iterator it = table.find(symbol_name);

  if (it != table.end()) {
    Symbol* symbol = it->second;
    symbol->size = size;
    return true;
  }
      
  return false;
}


void SymbolTable::remove_symbol(std::string symbol_name){
  std::map<std::string, Symbol*>::iterator it = table.find(symbol_name);

  if (it != table.end()) {
      delete it->second;
      table.erase(it);
  }
}


void SymbolTable::clear_table(){
  for (auto it = table.begin(); it != table.end(); ++it) {
      delete it->second;
  }
  table.clear();
}


SymbolTable::~SymbolTable(){
  this->clear_table();
}


std::ostream& operator<<(std::ostream& os, const SymbolTable& symbol_table) {

  for (auto symbol : symbol_table.table) {
    os<<*(symbol.second);
  }

  
  return os;
}













