#ifndef _ASSEMBLER_
#define _ASSEMBLER_

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


//Order of defining enums must be the same as in the map (token_type, regex)!
enum class Token_type {
    COMMA,
    END,
    DIRECTIVE,
    OPERAND_REG_IND,
    OPERAND_REG,
    OPERAND_REG_SPEC,
    OPERAND_REG_STATUS_CONTROL,
    OPERAND_DECIMAL_INDIRECT,
    OPERAND_HEX_INDIRECT,
    OPERAND_DECIMAL,
    OPERAND_HEX,
    INSTRUCTION,
    LABEL,
    SYMBOL_INDIRECT,
    SYMBOL,
    NONE
};


enum class Directive_type {
    GLOBAL,
    EXTERN,
    SECTION,
    WORD,
    SKIP,
    NONE
};

enum class Instruction {
    HALT,
    INT,
    IRET,
    RET,
    CALL,
    JMP,
    BEQ, BNE, BGT,
    PUSH, POP,
    XCHG,
    ADD, SUB, MUL, DIV,
    NOT, AND, OR, XOR, SHL, SHR,
    LD, ST,
    CSRRD, CSRWR,
    NONE
};

enum Pass{
  FIRST_PASS, SECOND_PASS
};

std::unordered_map<Instruction, unsigned char> instruction_op_codes = {
  {Instruction::HALT, 0x00},
  {Instruction::INT, 0x10},
  {Instruction::CALL, 0x20},
  {Instruction::IRET, 0x34},       //new op_codes form section of jump
  {Instruction::RET, 0x3C},
  {Instruction::JMP, 0x30},
  {Instruction::BEQ, 0x30},
  {Instruction::BNE, 0x30},
  {Instruction::BGT, 0x30},
  {Instruction::PUSH, 0x81},
  {Instruction::POP, 0x93},
  {Instruction::XCHG, 0x40},
  {Instruction::ADD, 0x50},
  {Instruction::SUB, 0x51},
  {Instruction::MUL, 0x52},
  {Instruction::DIV, 0x53},
  {Instruction::NOT, 0x60},
  {Instruction::AND, 0x61},
  {Instruction::OR, 0x62},
  {Instruction::XOR, 0x63},
  {Instruction::SHL, 0x70},
  {Instruction::SHR, 0x71},
  {Instruction::LD, 0x90},
  {Instruction::ST, 0x80},
  {Instruction::CSRRD, 0x90},
  {Instruction::CSRWR, 0x94},

};


std::unordered_map<Instruction, uint32_t> instruction_sizes = {
  {Instruction::HALT, 1},
  {Instruction::INT, 1},         
  {Instruction::CALL, 4 + 4},       //uses operand     Operand is stored right below the instruction on 4 bytes
  {Instruction::IRET, 1},
  {Instruction::RET, 1},
  {Instruction::JMP, 4 + 4},        //uses operand
  {Instruction::BEQ, 4 + 4},        //uses operand
  {Instruction::BNE, 4 + 4},        //uses operand
  {Instruction::BGT, 4 + 4},        //uses operand
  {Instruction::PUSH, 4},
  {Instruction::POP, 4},
  {Instruction::XCHG, 3},
  {Instruction::ADD, 3},
  {Instruction::SUB, 3},
  {Instruction::MUL, 3},
  {Instruction::DIV, 3},
  {Instruction::NOT, 2},
  {Instruction::AND, 3},
  {Instruction::OR, 3},
  {Instruction::XOR, 3},
  {Instruction::SHL, 3},
  {Instruction::SHR, 3},
  {Instruction::LD, 4 + 4},        //uses operand
  {Instruction::ST, 4 + 4},        //uses operand
  {Instruction::CSRRD, 2},
  {Instruction::CSRWR, 2},

};


std::unordered_map<Token_type, std::string> Token_type_name =
    {
        { Token_type::COMMA, "COMMA"},
        { Token_type::END, "END"},
        { Token_type::LABEL, "LABEL"},
        { Token_type::DIRECTIVE, "DIRECTIVE"},
        { Token_type::SYMBOL_INDIRECT, "SYMBOL_INDIRECT"},
        { Token_type::SYMBOL, "SYMBOL"},
        { Token_type::OPERAND_REG_IND, "OPERAND_REG_IND"},
        { Token_type::OPERAND_REG, "OPERAND_REG"},
        { Token_type::OPERAND_REG_SPEC, "OPERAND_REG_SPEC"},
        { Token_type::OPERAND_REG_STATUS_CONTROL, "OPERAND_REG_STATUS_CONTROL"},
        { Token_type::OPERAND_DECIMAL, "OPERAND_DECIMAL"},
        { Token_type::OPERAND_DECIMAL_INDIRECT, "OPERAND_DECIMAL_INDIRECT"},
        { Token_type::OPERAND_HEX_INDIRECT, "OPERAND_HEX_INDIRECT"},
        { Token_type::OPERAND_HEX, "OPERAND_HEX"},
        { Token_type::INSTRUCTION, "INSTRUCTION"},
        { Token_type::NONE, "NONE"}
    };


enum class Address_mode {
  IMMEDIATE_ADR,
  SYMBOLIC_ADR,
  DIRECT_ADR,
  SYMBOLIC_INDIRECT_ADR,
  REGISTER_DIRECT_ADR,
  REGISTER_INDIRECT_ADR,
  REGISTER_INDIRECT_OFFSET_LITERAL_ADR,
  REGISTER_INDIRECT_OFFSET_SYMBOL_ADR,
  NONE
};


struct Token{
  Token_type type = Token_type::NONE;
  std::string name = "";

  Token(const std::string name, const Token_type type): type(type), name(name){}

  friend std::ostream& operator<<(std::ostream& os, const Token& token);
};



struct Operand{
  Address_mode address_mode = Address_mode::NONE;
  uint32_t reg_num;           //If its register -> register number info
  uint32_t value;             //If its the symbol or immediate value, than its val
  uint32_t offset;            //Stores offest its its register indirect with offset addresing

  Operand(const Address_mode address_mode, uint32_t reg_num): address_mode(address_mode), reg_num(reg_num), offset(0){}

  Operand(const Address_mode address_mode, uint32_t reg_num, int offset): address_mode(address_mode), reg_num(reg_num), offset(offset){}
};


class Assembler{
  public:

    Assembler();
    ~Assembler();

    void Assemble(std::ifstream& input_file, std::ofstream& output_file);
    

  private:
    
    void load_input_file(std::ifstream& inputFile);
    void first_pass();
    void second_pass();
    std::vector<Token> tokenize_line(std::string line);
    void write_output_file(std::ofstream& output_file); 
    Token_type parse_token(std::string string_token);

    void Handle_label(std::vector<Token> tokens, const Pass pass);
    void Handle_directive(std::vector<Token> tokens, const Pass pass);
    void Handle_instruction(std::vector<Token> tokens);
    Directive_type Enumerate_directive(const std::string& directive_name);
    Instruction Enumerate_instruction(const std::string& instruction_name);
    bool syntax_param_check(const std::vector<std::unordered_set<Token_type>>& paramTypes, std::vector<Token>& tokens);
    uint8_t reg_num(const Token&);
    Operand get_operand(const Token&, const Instruction&);
    bool is_jump_instruction(const Instruction&);


    std::vector<std::vector<Token>> tokenized_input_file; 
    std::vector<std::string> loaded_input_file;
    std::unordered_map<const Directive_type, std::function<void(std::vector<Token>&, const Pass)>> Directive_handlers;
    std::unordered_map<const Instruction, std::function<void(std::vector<Token>&)>> Instruction_handlers;
    static std::map<Token_type, std::regex> token_parser;
    SymbolTable symbol_table;
    RelocationTable relocation_table;
    uint32_t line_counter = 0;
    uint32_t location_counter = 0;
    uint32_t symbol_counter = 0;

    std::map<std::string, Section> sections;
    std::vector<std::string> sections_order;
    static std::string current_section_name;

    //Param types
    static std::unordered_set<Token_type> reg_type;
    static std::unordered_set<Token_type> status_control_reg_type;
    static std::unordered_set<Token_type> operand_type;
    static std::unordered_set<Token_type> comma;

    static std::ofstream log_file;

};




#endif