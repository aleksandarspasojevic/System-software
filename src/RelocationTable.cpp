#include "../inc/RelocationTable.hpp"


std::unordered_map<Usage_type, std::string> Usage_type_name =
    {
        { Usage_type::SYMBOL, "SYMBOL"},
        { Usage_type::SYMBOL_INDIRECT, "SYMBOL_INDIRECT"},
        { Usage_type::NONE, "NONE"}
    };

std::unordered_map<std::string, Usage_type> Usage_name_type =
    {
        { "SYMBOL", Usage_type::SYMBOL},
        { "SYMBOL_INDIRECT", Usage_type::SYMBOL_INDIRECT},
        { "NONE", Usage_type::NONE}
    };

std::ostream& operator<<(std::ostream& os, const Relocation& relocation){
   os << std::left << "  " <<
    std::setw(20) << relocation.symbol_name <<
    std::setw(20) << Usage_type_name.at(relocation.type_of_relocation) <<
    std::setw(20) << std::dec << relocation.location <<
    std::setw(20) << relocation.section_name <<
    std::endl;

  return os;
}


std::ostream& operator<<(std::ostream& os, const RelocationTable& relocation_table) {

  for (auto reloc : relocation_table.table) {
    os<<*(reloc.second);
  }
  
  return os;
}


void RelocationTable::add_relocation(Relocation* reloc){
  this->table.insert({reloc->symbol_name, reloc});
}


std::vector<Relocation*> RelocationTable::get_relocations(std::string symbol_name){
  std::vector<Relocation*> matching_relocations;

  auto range = table.equal_range(symbol_name);
  for (auto it = range.first; it != range.second; ++it) 
      matching_relocations.push_back(it->second);
  

  return matching_relocations;
}


void RelocationTable::remove_relocation(std::string symbol_name){
  std::map<std::string, Relocation*>::iterator it = table.find(symbol_name);

  if (it != table.end()) {
      delete it->second;
      table.erase(it);
  }
}


void RelocationTable::clear_table(){
  for (auto it = table.begin(); it != table.end(); ++it) {
      delete it->second;
  }
  table.clear();
}


RelocationTable::RelocationTable(){}

RelocationTable::~RelocationTable(){
  this->clear_table();
}