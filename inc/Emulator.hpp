#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "Exceptions.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <cmath>



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

std::unordered_map<Instruction, std::string> Instruction_name = {
    { Instruction::HALT, "HALT" },
    { Instruction::INT, "INT" },
    { Instruction::IRET, "IRET" },
    { Instruction::RET, "RET" },
    { Instruction::CALL, "CALL" },
    { Instruction::JMP, "JMP" },
    { Instruction::BEQ, "BEQ" },
    { Instruction::BNE, "BNE" },
    { Instruction::BGT, "BGT" },
    { Instruction::PUSH, "PUSH" },
    { Instruction::POP, "POP" },
    { Instruction::XCHG, "XCHG" },
    { Instruction::ADD, "ADD" },
    { Instruction::SUB, "SUB" },
    { Instruction::MUL, "MUL" },
    { Instruction::DIV, "DIV" },
    { Instruction::NOT, "NOT" },
    { Instruction::AND, "AND" },
    { Instruction::OR, "OR" },
    { Instruction::XOR, "XOR" },
    { Instruction::SHL, "SHL" },
    { Instruction::SHR, "SHR" },
    { Instruction::LD, "LD" },
    { Instruction::ST, "ST" },
    { Instruction::CSRRD, "CSRRD" },
    { Instruction::CSRWR, "CSRWR" }
};


std::unordered_map<unsigned char, Instruction> instruction_op_codes = {
  {0x00, Instruction::HALT},
  {0x10, Instruction::INT},

  {0x20, Instruction::CALL},    //Difference in modifiers
  {0x21, Instruction::CALL},

  {0x34, Instruction::IRET},    //new op_codes form section of jump
  {0x3C, Instruction::RET},

  {0x30, Instruction::JMP},
  {0x38, Instruction::JMP},

  {0x31, Instruction::BEQ},
  {0x39, Instruction::BEQ},

  {0x32, Instruction::BNE},
  {0x3A, Instruction::BNE},

  {0x33, Instruction::BGT},
  {0x3B, Instruction::BGT},

  {0x81, Instruction::PUSH},
  {0x93, Instruction::POP},
  {0x40, Instruction::XCHG},
  {0x50, Instruction::ADD},
  {0x51, Instruction::SUB},
  {0x52, Instruction::MUL},
  {0x53, Instruction::DIV},
  {0x60, Instruction::NOT},
  {0x61, Instruction::AND},
  {0x62, Instruction::OR},
  {0x63, Instruction::XOR},
  {0x70, Instruction::SHL},
  {0x71, Instruction::SHR},

  {0x91, Instruction::LD},
  {0x92, Instruction::LD},

  {0x80, Instruction::ST},
  {0x82, Instruction::ST},       //*MODIFIED*

  {0x90, Instruction::CSRRD},
  {0x94, Instruction::CSRWR},

};


class Emulator{
    public:
        Emulator();
        ~Emulator();

        void Emulate(std::ifstream& inputFile);

    private:
        void init_memory(std::ifstream& inputFile);
        void execute();
        void write_output(std::ostream& os);
        void interrupt_check();

        void inc_pc();
        unsigned char read_memory_byte(int address);
        uint32_t read_memory_32(int address);
        void write_memory_byte(int address, unsigned char value);
        void write_memory_32(int address, uint32_t value);
        void push(int& val);
        void pop(int& val);

        std::unordered_map<Instruction, std::function<void(unsigned char op_code)>> instruction_handlers;
        static std::ofstream log_file;
        
        //r[15] -> pc,  r[14] -> sp
        int r[16] = {0};     
        int status = 0;        //System regs
        int handle = 0;
        int cause = 0;      
        int psw;
        std::map<int, unsigned char> memory;
        int start_address = 0x40000000;
                            

};





#endif