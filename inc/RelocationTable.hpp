
#ifndef _RELOCATION_TABLE_H_
#define _RELOCATION_TABLE_H_

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>


enum class Usage_type{
  SYMBOL,
  SYMBOL_INDIRECT,
  NONE
};

extern std::unordered_map<std::string, Usage_type> Usage_name_type;



struct Relocation{
  std::string symbol_name;
  Usage_type type_of_relocation;
  uint32_t location;
  std::string section_name;
  
  Relocation(): symbol_name(""), type_of_relocation(Usage_type::NONE), location(0), section_name(""){}

  Relocation(std::string symbol_name, Usage_type type_of_relocation, uint32_t location, std::string section_name): symbol_name(symbol_name),
    type_of_relocation(type_of_relocation), location(location), section_name(section_name){}

  
  static Relocation* deserialize_relocation_entry(std::string line) {
    std::stringstream ss(line);
    std::string token;
    Relocation relocation;

    ss>>token; relocation.symbol_name = token;
    ss>>token; relocation.type_of_relocation = Usage_name_type.at(token);
    ss>>token; relocation.location = std::stoull(token, nullptr, 0);         //NOTE: location is decimal
    ss>>token; relocation.section_name = token;

    return new Relocation(relocation);
  }


  friend std::ostream& operator<<(std::ostream& os, const Relocation&);
};

class RelocationTable{
  public:
    
    std::multimap<std::string, Relocation*> table;

    void add_relocation(Relocation* relocation);
    std::vector<Relocation*> get_relocations(std::string symbol_name);
    void remove_relocation(std::string symbol_name);
    void clear_table();
    friend std::ostream& operator<<(std::ostream& os, const RelocationTable& table);


    RelocationTable();
    ~RelocationTable(); 

    friend std::ostream& operator<<(std::ostream& os, const RelocationTable&);

};

#endif 