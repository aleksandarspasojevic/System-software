#include "../inc/Emulator.hpp"


std::ofstream Emulator::log_file("emulator.log");


Emulator::Emulator(){
    //Load PC start address
    r[15] = this->start_address;

    this->instruction_handlers = {
        {Instruction::INT, [&](unsigned char op_code) {
            log_file << "INT " << std::hex << "0x" << (int)op_code << std::endl;

            // push status; push pc; cause<=4; status<=status&(~0x1); pc<=handle;

            push(status);
            push(r[15]);
            cause = 4;
            status = status &(~0x1); 
            r[15] = handle;

        }},
        {Instruction::IRET, [&](unsigned char op_code) {
            log_file << "IRET " << std::hex << "0x" << (int)op_code << std::endl;

            //pop pc; pop status;

            pop(r[15]);
            pop(status);

        }},
        {Instruction::RET, [&](unsigned char op_code) {
            log_file << "RET " << std::hex << "0x" << (int)op_code << std::endl;

            //pop pc; 

            pop(r[15]);

        }},
        {Instruction::CALL, [&](unsigned char op_code) {
            log_file << "CALL " << std::hex << "0x" << (int)op_code << std::endl;

            //direct
            if(op_code == 0x20){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                //push pc; pc <= operand;
                //push pc; pc<=gpr[A]+gpr[B]+D;

                push(r[15]);
                r[15] = D;

            }

            //over memory
            else if(op_code == 0x21){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                //push pc; pc <= operand;
                // push pc; pc<=mem32[gpr[A]+gpr[B]+D];

                push(r[15]);
                r[15] = read_memory_32(D);
            }

        }},
        {Instruction::JMP, [&](unsigned char op_code) {
            log_file << "JMP " << std::hex << "0x" << (int)op_code << std::endl;

            if(op_code == 0x30){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                inc_pc(); 
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                r[15] = D; 

            }

            //memory
            else if(op_code == 0x38){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                inc_pc(); 
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                r[15] = read_memory_32(D); 
            }

        }},
        {Instruction::BEQ, [&](unsigned char op_code) {
            log_file << "BEQ " << std::hex << "0x" << (int)op_code << std::endl;


            // direct
            if(op_code == 0x31){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
                if(regB == regC) r[15] = D;

            }

            //memory
            else if(op_code == 0x39){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
                if(regB == regC) r[15] = read_memory_32(D);
            }


        }},
        {Instruction::BNE, [&](unsigned char op_code) {
            log_file << "BNE " << std::hex << "0x" << (int)op_code << std::endl;

            // direct
            if(op_code == 0x32){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
                if(regB != regC) r[15] = D;

            }

            //memory
            else if(op_code == 0x3A){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
                if(regB != regC) r[15] = read_memory_32(D);
            }

        }},
        {Instruction::BGT, [&](unsigned char op_code) {
            log_file << "BGT " << std::hex << "0x" << (int)op_code << std::endl;

            // direct
            if(op_code == 0x33){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
                if(regB > regC) r[15] = D;

            }

            //memory
            else if(op_code == 0x3B){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
                if(regB > regC) r[15] = read_memory_32(D);
            }

        }},
        {Instruction::PUSH, [&](unsigned char op_code) {
            log_file << "PUSH " << std::hex << "0x" << (int)op_code << std::endl;

            //1st byte
            unsigned char byte= read_memory_byte(r[15]);
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            inc_pc();

            // push %gpr sp <= sp - 4; mem32[sp] <= gpr; 
            //mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C];

            push(regC);

        }},
        {Instruction::POP, [&](unsigned char op_code) {
            log_file << "POP " << std::hex << "0x" << (int)op_code << std::endl;


            //1st byte
            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            inc_pc();
            inc_pc();
            inc_pc();

            // gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
            // gpr <= mem32[sp]; sp <= sp + 4;

            pop(regA);

        }},
        {Instruction::XCHG, [&](unsigned char op_code) {
            log_file << "XCHG " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();

            //temp<=gpr[B]; gpr[B]<=gpr[C]; gpr[C]<=temp

            int temp = regB;
            regB = regC;
            regC = temp;


        }},
        {Instruction::ADD, [&](unsigned char op_code) {
            log_file << "ADD " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB + regC;


        }},
        {Instruction::SUB, [&](unsigned char op_code) {
            log_file << "SUB " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB - regC;

        }},
        {Instruction::MUL, [&](unsigned char op_code) {
            log_file << "MUL " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB * regC;

        }},
        {Instruction::DIV, [&](unsigned char op_code) {
            log_file << "DIV " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB / regC;

        }},
        {Instruction::NOT, [&](unsigned char op_code) {
            log_file << "NOT " << std::hex << "0x" << (int)op_code << std::endl;
            
            //2 BYTE instruction
            //GET A, B register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            
            regA = ~regB;

        }},
        {Instruction::AND, [&](unsigned char op_code) {
            log_file << "AND " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB & regC;

        }},
        {Instruction::OR, [&](unsigned char op_code) {
            log_file << "OR " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB | regC;

        }},
        {Instruction::XOR, [&](unsigned char op_code) {
            log_file << "XOR " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB ^ regC;

        }},
        {Instruction::SHL, [&](unsigned char op_code) {
            log_file << "SHL " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB << regC;

        }},
        {Instruction::SHR, [&](unsigned char op_code) {
            log_file << "SHR " << std::hex << "0x" << (int)op_code << std::endl;

            //GET A, B, C register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int& regB = r[byte & 0x0F];
            inc_pc();
            byte= read_memory_byte(r[15]);
            int& regC = r[(byte & 0xF0) >> 4];
            inc_pc();
            
            regA = regB >> regC;

        }},
        {Instruction::LD, [&](unsigned char op_code) {
            log_file << "LD " << std::hex << "0x" << (int)op_code << std::endl;

            // direct
            if(op_code == 0x91){
                //1st byte
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc(); 
                inc_pc();
                inc_pc();

                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                regA = regB + D; 

            }

            //memory
            else if(op_code == 0x92){
                //1st byte
                //GET A, B, C register references
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                int& regB = r[byte & 0x0F];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();


                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value

                //gpr[A]<=mem32[gpr[B]+gpr[C]+D];
                regA = read_memory_32(regB + regC + D);
            }


        }},
        {Instruction::ST, [&](unsigned char op_code) {
            log_file << "ST " << std::hex << "0x" << (int)op_code << std::endl;

            // direct
            if(op_code == 0x80){
                //1st byte
                //GET A, B, C register references
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();


                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value


                // mem32[gpr[A]+gpr[B]+D]<=gpr[C];
                write_memory_32(regA + D, regC);

            }
            
            else if(op_code == 0x82){           //**MODIFIED**
                //1st byte
                //GET A, B, C register references
                unsigned char byte= read_memory_byte(r[15]);
                int& regA = r[(byte & 0xF0) >> 4];
                inc_pc();
                byte= read_memory_byte(r[15]);
                int& regC = r[(byte & 0xF0) >> 4];
                inc_pc();
                inc_pc();


                //2nd byte
                //literal pool below the instruction
                unsigned char b1= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b2= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b3= read_memory_byte(r[15]);
                inc_pc();
                unsigned char b4= read_memory_byte(r[15]);
                inc_pc();
                //b1|b2|b3|b4
                
                uint32_t D = 0;
                D |= static_cast<uint32_t>(b1) << 24;  
                D |= static_cast<uint32_t>(b2) << 16;  
                D |= static_cast<uint32_t>(b3) << 8;  
                D |= static_cast<uint32_t>(b4); 
                
                //D is value from second byte - its the symbol value


                // mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C];
                write_memory_32(read_memory_32(regA + D), regC);   
                // write_memory_32(regA + D, regC);
            }

        }},
        {Instruction::CSRRD, [&](unsigned char op_code) {
            log_file << "CSRRD " << std::hex << "0x" << (int)op_code << std::endl;

            //2 BYTE instruction
            //GET A, B register references
            unsigned char byte= read_memory_byte(r[15]);
            int& regA = r[(byte & 0xF0) >> 4];
            int b = (byte & 0x0F);
            inc_pc();

            //csrrd %csr, %gpr
            // gpr <= csr  a b
            
           
            if(b == 0){   //status
                regA = status;
            }else if(b == 1){   //handle
                regA = handle;
            }else if(b == 2){   //cause
                regA = cause;
            }



        }},
        {Instruction::CSRWR, [&](unsigned char op_code) {
            log_file << "CSRWR " << std::hex << "0x" << (int)op_code << std::endl;

            //2 BYTE instruction
            //GET A, B register references
            unsigned char byte= read_memory_byte(r[15]);
            int a = ((byte & 0xF0) >> 4);
            int& regB = r[(byte & 0x0F)];
            inc_pc();

            //csrwr %gpr, %csr
            
            if(a == 0){   //status
                status = regB;
            }else if(a == 1){   //handle
                handle = regB;
            }else if(a == 2){   //cause
                cause = regB;
            }

        }}

    };

    
}
    

Emulator::~Emulator(){
  this->log_file.close();
}


void Emulator::Emulate(std::ifstream& inputFile){
    this->init_memory(inputFile);

    this->execute();

    this->write_output(std::cout);
}


void Emulator::init_memory(std::ifstream& inputFile){
    std::string line;
    std::string data;
    uint32_t address = 0;

    //FILL THE MEMORY MAP
    while(getline(inputFile, line)) {
        //empty line
        if (line.empty() || line == "\n" || line.find_first_not_of(' ') == std::string::npos) {   
            continue;
        }

        std::stringstream ss(line);

        //Read address
        ss>>data;
        data.pop_back();
        address = stoull("0x"+data, nullptr, 16);

        while(ss >> data){
            try{
                memory[address] = (unsigned char)stoi(data, nullptr, 16); 
                // log_file<<std::hex<<(int)memory[address]<<" "<<address<<std::endl;
            }
            catch (const std::exception& e) {}
            
            address++;
        }
    }


    //ALLOCATE MEMORY MAPPED REGISTERS from address 0xFFFFFF00 of size 256 bytes
    for(int address = 0xFFFFFF00; address<0xFFFFFF00 + 256; address++)
        memory[address] = 0;  //o initialized

}


void Emulator::execute(){
    while(true){
        unsigned char op_code = read_memory_byte(r[15]);
        inc_pc();


        if(instruction_op_codes.at(op_code) == Instruction::HALT) break;

        std::cout<<std::hex<<(int)op_code<<std::endl;

        if(this->instruction_handlers.find(instruction_op_codes.at(op_code)) != this->instruction_handlers.end())
            this->instruction_handlers.at(instruction_op_codes.at(op_code))(op_code);
        else 
            throw UnrecognizedOperactionCode(op_code);

        
        this->log_file<<Instruction_name.at(instruction_op_codes.at(op_code))<<" "<<std::hex<<(int)op_code<<"  ";
        this->log_file<<"handle: "<<handle<<"   cause:"<<cause<<"   status: "<<status<<std::endl;
        write_output(this->log_file);
        this->log_file<<std::endl;
        this->log_file<<std::endl;
        this->log_file<<std::endl;

        this->interrupt_check();
        // break;     //remove this
    }
}


void Emulator::interrupt_check(){

}

void Emulator::write_output(std::ostream& os){
    os << "------------------------------------------------\n";
    os << "Emulated processor executed halt instruction    \n";
    os << "Emulated processor state: \n";
    for (int i = 0; i < 16; i++) {
        if(i % 4 == 0) os<<std::endl;
        os<< 'r' << std::dec << i << "=0x" << std::setfill('0') << std::setw(8) <<std::hex << r[i] << ((i < 10)? "    " : "   ");
    }
    os<<std::endl;
}



//PROCESSOR STATE HANDLING INSTRUCTIONS
void Emulator::inc_pc() {
    r[15]++;
}

unsigned char Emulator::read_memory_byte(int address) {

    // if (memory.find(address) == memory.end()){
    //     std::cout<<"Can not read from address: "<<std::hex<<address<<std::endl;
    //     throw MemoryReadViolation(address);
    // }

    return memory[address];
}


uint32_t Emulator::read_memory_32(int address) {

    // unsigned char b1= read_memory_byte(r[15]);
    // inc_pc();
    // unsigned char b2= read_memory_byte(r[15]);
    // inc_pc();
    // unsigned char b3= read_memory_byte(r[15]);
    // inc_pc();
    // unsigned char b4= read_memory_byte(r[15]);
    // inc_pc();
    // //b1|b2|b3|b4

    //***MODIFIED***
    unsigned char b1= read_memory_byte(address);
    unsigned char b2= read_memory_byte(address+1);
    unsigned char b3= read_memory_byte(address+2);
    unsigned char b4= read_memory_byte(address+3);

    uint32_t D = 0;
    D |= static_cast<uint32_t>(b1) << 24;  
    D |= static_cast<uint32_t>(b2) << 16;  
    D |= static_cast<uint32_t>(b3) << 8;  
    D |= static_cast<uint32_t>(b4); 

    return D;
}



void Emulator::write_memory_byte(int address, unsigned char value) {
    // if (memory.find(address) == memory.end()){
    //     std::cout<<"Can not write to address: "<<std::hex<<address<<std::endl;
    //     throw MemoryReadViolation(address);
    // } 
    

    memory[address] = value;
}

void Emulator::write_memory_32(int address, uint32_t value){
    // write_memory_byte(address, (value & 0xFF000000) >> 24);
    // write_memory_byte(address, (value & 0x00FF0000) >> 16);
    // write_memory_byte(address, (value & 0x0000FF00) >> 8);
    // write_memory_byte(address, value &  0x000000FF);

    //***MODIFIED***
     write_memory_byte(address, (value & 0xFF000000) >> 24);
    write_memory_byte(address+1, (value & 0x00FF0000) >> 16);
    write_memory_byte(address+2, (value & 0x0000FF00) >> 8);
    write_memory_byte(address+3, value &  0x000000FF);

}


void Emulator::push(int& val) {
  r[14] -= 4;
  write_memory_32(r[14], val);
}
void Emulator::pop(int& val) {
  val = read_memory_32(r[14]);
  r[14] += 4;
}


int main(int argc, char* argv[]) {
  try {
   
    if (argc < 2)
        throw InvalidEmulatorCmdArgs();

    std::ifstream input_file(argv[1]);

    if (!input_file.is_open())
        throw FileNameError(argv[1]);

    Emulator* emulator = new Emulator();
    emulator->Emulate(input_file);
    delete emulator;

    input_file.close();
  }
  catch(const std::exception& e) {
    std::cout << e.what() << '\n';
  }

  return 0;
}