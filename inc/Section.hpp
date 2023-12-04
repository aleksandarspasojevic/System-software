#ifndef _SECTION_
#define _SECTION_

#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>

class Section{

public:
  Section (std::string name, std::string file_name = "");
  ~Section();

  void append_code_byte(uint8_t code_byte);

  void write_instruction(uint8_t OP_CODE, uint8_t MOD, uint8_t A, uint8_t B, uint8_t C, uint16_t DISP);
  void write_instruction(uint8_t OP_CODE, uint8_t MOD, uint8_t A, uint8_t B, uint8_t C);
  void write_instruction(uint8_t OP_CODE, uint8_t MOD, uint8_t A, uint8_t B);
  void write_word(uint32_t word);
  int get_writing_location();
  static void deserialize_line_section_code(Section& section, const std::string& line);

  friend std::ostream& operator<<(std::ostream& os, const Section& section);

  void hex_output(std::ostream& os);

  std::string name;
  std::string file_name;
  std::vector<uint8_t> section_code; 

  //LINKER
  uint32_t location = 0;
  bool placed = false;
};


#endif
