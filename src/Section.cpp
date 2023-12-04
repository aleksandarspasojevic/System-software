#include "../inc/Section.hpp"


std::ostream& operator<<(std::ostream& os, const Section& section) {

    os << "Section: " << section.name;
    for (int i = 0; i < section.section_code.size(); i++){
        if (i % 8 == 0) os << std::endl;
        os << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast<uint32_t>(section.section_code[i]) << " ";
    }

    return os << std::setfill(' ') << std::endl;
}

void Section::hex_output(std::ostream& os){
  //Formatting options
  int location_counter = this->location-8;

  for (int i = 0; i < this->section_code.size(); i++){
      if (i % 8 == 0)
        os << std::uppercase << std::setfill('0') << std::setw(4) << std::right << std::hex <<std::endl<<(location_counter+=8)<<": ";

      os << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex<< static_cast<uint32_t>(this->section_code[i]) << " ";
  }

}

void Section::deserialize_line_section_code(Section& section, const std::string& line) {
    std::stringstream ss(line);

    uint32_t code_byte;
    while (ss >> std::hex >> code_byte) {
        section.append_code_byte(code_byte & 0xFF);
    }
}

Section::Section(std::string name, std::string file_name): name(name), file_name(file_name){}


void Section::append_code_byte(uint8_t code_byte){
  this->section_code.push_back(code_byte);
}

//Actual param sizes: OP_CODE = 4b, MOD = 4b, A = 4b, B = 4b, C = 4b, DISP = 12b
void Section::write_instruction(uint8_t OP_CODE, uint8_t MOD, uint8_t A, uint8_t B, uint8_t C, uint16_t DISP){
  uint8_t first_byte = ((OP_CODE&0x0F)<<4) | MOD&0x0F;
  uint8_t second_byte = ((A&0x0F)<<4) | B&0x0F;
  uint8_t third_byte = (C&0x0F)<<4 | ((DISP&0x0F00)>>8);
  uint8_t fourth_byte = DISP & 0xFF;

  this->section_code.push_back(first_byte);
  this->section_code.push_back(second_byte);
  this->section_code.push_back(third_byte);
  this->section_code.push_back(fourth_byte);
}

void Section::write_instruction(uint8_t OP_CODE, uint8_t MOD, uint8_t A, uint8_t B, uint8_t C){
  uint8_t first_byte = ((OP_CODE&0x0F)<<4) | MOD&0x0F;
  uint8_t second_byte = ((A&0x0F)<<4) | B&0x0F;
  uint8_t third_byte = (C&0x0F)<<4;

  this->section_code.push_back(first_byte);
  this->section_code.push_back(second_byte);
  this->section_code.push_back(third_byte);
}

void Section::write_instruction(uint8_t OP_CODE, uint8_t MOD, uint8_t A, uint8_t B = 0){
  uint8_t first_byte = ((OP_CODE&0x0F)<<4) | MOD&0x0F;
  uint8_t second_byte = ((A&0x0F)<<4) | B&0x0F;

  this->section_code.push_back(first_byte);
  this->section_code.push_back(second_byte);
}

void Section::write_word(uint32_t word){
  this->section_code.push_back((word & 0xFF000000) >> 3*8);
  this->section_code.push_back((word & 0x00FF0000) >> 2*8);
  this->section_code.push_back((word & 0x0000FF00) >> 1*8);
  this->section_code.push_back(word & 0x000000FF);
}

int Section::get_writing_location(){
  return this->section_code.size();
}

Section::~Section(){

}

