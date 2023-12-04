#include "../inc/Assembler.hpp"


//Default log file
std::ofstream Assembler::log_file("assembler.log");
std::string Assembler::current_section_name = "UND";


//Parsing rules from more specific to less specific order
std::map<Token_type, std::regex> Assembler::token_parser =
    {
        { Token_type::COMMA, std::regex(R"(^\,$)")},
        { Token_type::END, std::regex(R"(^(\.end)$)")},
        { Token_type::DIRECTIVE, std::regex("^\\.(global|extern|section|word|skip)$")},
        { Token_type::OPERAND_REG_IND, std::regex(R"(^\[.*\]$)")},
        { Token_type::OPERAND_REG, std::regex("^%r([0-9]{1,2})$")},
        { Token_type::OPERAND_REG_SPEC, std::regex("^%(pc|sp)")},
        { Token_type::OPERAND_REG_STATUS_CONTROL, std::regex("^%(status|handler|cause)")},
        { Token_type::OPERAND_DECIMAL_INDIRECT, std::regex("^(\\d+)$")},
        { Token_type::OPERAND_HEX_INDIRECT, std::regex("^(0x[0-9a-fA-F]+)$")},
        { Token_type::OPERAND_DECIMAL, std::regex("^\\$(\\d+)$")},
        { Token_type::OPERAND_HEX, std::regex(R"(^\$(0x[0-9a-fA-F]+)$)")},
        { Token_type::INSTRUCTION, std::regex("^(halt|int|ret|call|iret|jmp|beq|bne|bgt|push|pop|xchg|add|sub|mul|div|not|and|or|xor|shl|shr|ld|st|csrrd|csrwr)(eq|ne|gt|ge|lt|le|al)?(s)?$")},
        { Token_type::LABEL, std::regex("^([a-zA-Z_][a-zA-Z0-9_]*):$")},
        { Token_type::SYMBOL_INDIRECT, std::regex("^\\$([a-zA-Z_][a-zA-Z0-9_]*)$")},
        { Token_type::SYMBOL, std::regex("^([a-zA-Z_][a-zA-Z0-9_]*)$")}
        
    };


//Parameter types for instructions
std::unordered_set<Token_type> Assembler::reg_type = {Token_type::OPERAND_REG, Token_type::OPERAND_REG_SPEC};

std::unordered_set<Token_type> Assembler::status_control_reg_type = {Token_type::OPERAND_REG_STATUS_CONTROL};

std::unordered_set<Token_type> Assembler::operand_type = 
  {
    Token_type::OPERAND_REG_IND, Token_type::OPERAND_REG,
    Token_type::OPERAND_REG_SPEC, Token_type::OPERAND_REG_STATUS_CONTROL, Token_type::OPERAND_DECIMAL, Token_type::OPERAND_HEX, Token_type::OPERAND_DECIMAL_INDIRECT,
    Token_type::OPERAND_HEX_INDIRECT, Token_type::SYMBOL, Token_type::SYMBOL_INDIRECT
  };

std::unordered_set<Token_type> Assembler::comma = {Token_type::COMMA}; 



Directive_type Assembler::Enumerate_directive(const std::string& directive_name){
  if (directive_name == ".global") return Directive_type::GLOBAL;
  else if (directive_name == ".extern") return Directive_type::EXTERN;
  else if (directive_name == ".section") return Directive_type::SECTION;
  else if (directive_name == ".word") return Directive_type::WORD;
  else if (directive_name == ".skip") return Directive_type::SKIP;
   
  return Directive_type::NONE;
}


Instruction Assembler::Enumerate_instruction(const std::string& instruction_name){
  if (instruction_name == "halt") return Instruction::HALT;
  if (instruction_name == "int") return Instruction::INT;
  if (instruction_name == "iret") return Instruction::IRET;
  if (instruction_name == "ret") return Instruction::RET;
  if (instruction_name == "call") return Instruction::CALL;
  if (instruction_name == "jmp") return Instruction::JMP;
  if (instruction_name == "beq") return Instruction::BEQ;
  if (instruction_name == "bne") return Instruction::BNE;
  if (instruction_name == "bgt") return Instruction::BGT;
  if (instruction_name == "push") return Instruction::PUSH;
  if (instruction_name == "pop") return Instruction::POP;
  if (instruction_name == "xchg") return Instruction::XCHG;
  if (instruction_name == "add") return Instruction::ADD;
  if (instruction_name == "sub") return Instruction::SUB;
  if (instruction_name == "mul") return Instruction::MUL;
  if (instruction_name == "div") return Instruction::DIV;
  if (instruction_name == "not") return Instruction::NOT;
  if (instruction_name == "and") return Instruction::AND;
  if (instruction_name == "or") return Instruction::OR;
  if (instruction_name == "xor") return Instruction::XOR;
  if (instruction_name == "shl") return Instruction::SHL;
  if (instruction_name == "shr") return Instruction::SHR;
  if (instruction_name == "ld") return Instruction::LD;
  if (instruction_name == "st") return Instruction::ST;
  if (instruction_name == "csrrd") return Instruction::CSRRD;
  if (instruction_name == "csrwr") return Instruction::CSRWR;
   
  return Instruction::NONE;
}

  


std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << '(' <<token.name <<"  "<< Token_type_name.at(token.type) << ')'; 
  return os;
}


Token_type Assembler::parse_token(std::string string_token)
{
    Token_type type = Token_type::NONE;

    for (const auto &parser: token_parser){

      std::regex parse_rule =  parser.second;

      if (std::regex_search(string_token, parse_rule))
      {
          type = (Token_type)parser.first;
          break;
      }
    }

    return type;
}


bool Assembler::is_jump_instruction(const Instruction& instruction){
  return instruction == Instruction::CALL || instruction == Instruction::BEQ || instruction == Instruction::BGT
  || instruction == Instruction::BNE || instruction == Instruction::JMP;
}


Operand Assembler::get_operand(const Token& token, const Instruction& instruction){
  std::smatch match;

  if(this->is_jump_instruction(instruction)){

    if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_DECIMAL_INDIRECT)) 
      || std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_HEX_INDIRECT))){

      std::string val = match[1];
      uint32_t offset = std::stoull(val, nullptr, 
        (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_DECIMAL_INDIRECT)) ? 10 : 16));

      return Operand(Address_mode::IMMEDIATE_ADR, 0, offset); 
    }

    if (std::regex_search(token.name, match, token_parser.at(Token_type::SYMBOL))){
      Symbol* symbol = symbol_table.get_symbol_by_name(match[1]);
      int offset;
      if (symbol) offset = symbol->value;
      else throw UndefinedSymbolError(match[1], this->line_counter);

      //rellocation entry
      int location  = this->sections.at(Assembler::current_section_name).get_writing_location();  
      relocation_table.add_relocation(new Relocation(token.name, Usage_type::SYMBOL, location + 4, Assembler::current_section_name));

      return Operand(Address_mode::SYMBOLIC_ADR, 0, offset); 
    }

    throw JumpInstructionOperandNotSUpported(token.name, this->line_counter);

    return Operand(Address_mode::IMMEDIATE_ADR, 0);
  }
  
  if (std::regex_search(token.name, match, std::regex(R"(^\[(%r[0-9]{1,2}|%pc|%sp)\]$)"))){
    uint32_t reg_num = this->reg_num(Token(match[1], Token_type::NONE));    //Token type is not important here

    return Operand(Address_mode::REGISTER_INDIRECT_ADR, reg_num); 
  }


  if (std::regex_search(token.name, match, std::regex(R"(^\[(%r[0-9]{1,2}|%pc|%sp)\s*\+\s*(0x[0-9a-fA-F]+)\]$)"))){
    uint32_t reg_num = this->reg_num(Token(match[1], Token_type::NONE));

    std::string val = match[2];
    int offset = std::stoull(val, nullptr, 16);       //hex value conversion
    return Operand(Address_mode::REGISTER_INDIRECT_OFFSET_LITERAL_ADR, reg_num, offset); 
  }

  if (std::regex_search(token.name, match, std::regex(R"(^\[(%r[0-9]{1,2}|%pc|%sp)\s*\+\s*(\d+)\]$)"))){
    uint32_t reg_num = this->reg_num(Token(match[1], Token_type::NONE));

    std::string val = match[2];
    int offset = std::stoull(val);       //NOTE: signed offset
    return Operand(Address_mode::REGISTER_INDIRECT_OFFSET_LITERAL_ADR, reg_num, offset); 
  }


  if (std::regex_search(token.name, match, std::regex(R"(^\[(%r[0-9]{1,2}|%pc|%sp)\s*\+\s*([a-zA-Z_][a-zA-Z0-9_]*)\]$)"))){
    uint32_t reg_num = this->reg_num(Token(match[1], Token_type::NONE));

    Symbol* symbol = symbol_table.get_symbol_by_name(match[2]);
    int offset;
    if (symbol) offset = symbol->value;
    else throw UndefinedSymbolError(match[2], this->line_counter);


    if(offset > std::pow(2, 12)-1 || offset < -std::pow(2, 12))
      throw BigOperandError(token.name, this->line_counter);  //Throw error if operand takes more than 32 bits


    //rellocation entry
    int location  = this->sections.at(Assembler::current_section_name).get_writing_location(); 
    relocation_table.add_relocation(new Relocation(symbol->name, Usage_type::SYMBOL, location+4, Assembler::current_section_name));

    return Operand(Address_mode::REGISTER_INDIRECT_OFFSET_SYMBOL_ADR, reg_num, offset); 
  }
  

  //INDIRECTS JUST HAVE $ IN FRONT OF IT, PARSER ALRDY REMOVED IT
  // if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_DECIMAL)) 
  //     || std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_HEX))){


  //   uint32_t reg_num = std::stoull(match[1], nullptr, (std::regex_search(token.name, match, 
  //     token_parser.at(Token_type::OPERAND_DECIMAL))) ? 10:16);

  //   return Operand(Address_mode::IMMEDIATE_ADR, reg_num); 
  // }

  
   if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_DECIMAL_INDIRECT)) 
      || std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_HEX_INDIRECT))){

    std::string val = match[1];
    uint32_t offset = std::stoull(val, nullptr, (std::regex_search(token.name, match, 
      token_parser.at(Token_type::OPERAND_DECIMAL_INDIRECT))) ? 10:16);

    return Operand(Address_mode::DIRECT_ADR, 0, offset); 
  }

  if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_REG))){
    std::string val = match[1];
    uint32_t reg_num = std::stoull(val);
    return Operand(Address_mode::REGISTER_DIRECT_ADR, reg_num); 
  }

  if (std::regex_search(token.name, match, token_parser.at(Token_type::SYMBOL))){
    Symbol* symbol = symbol_table.get_symbol_by_name(match[1]);
    int offset;
    if (symbol) offset = symbol->value;
    else throw UndefinedSymbolError(match[1], this->line_counter);

    //rellocation entry
    int location  = this->sections.at(Assembler::current_section_name).get_writing_location(); 
    relocation_table.add_relocation(new Relocation(token.name, Usage_type::SYMBOL_INDIRECT, location+4, Assembler::current_section_name));

    return Operand(Address_mode::SYMBOLIC_INDIRECT_ADR, 0, offset); 
  }

  if (std::regex_search(token.name, match, token_parser.at(Token_type::SYMBOL_INDIRECT))){
    Symbol* symbol = symbol_table.get_symbol_by_name(match[1]);
    int offset;
    if (symbol) offset = symbol->value;
    else throw UndefinedSymbolError(match[2], this->line_counter);

    //rellocation entry
    int location  = this->sections.at(Assembler::current_section_name).get_writing_location(); 
    relocation_table.add_relocation(new Relocation(token.name, Usage_type::SYMBOL, location+4, Assembler::current_section_name));

    return Operand(Address_mode::SYMBOLIC_ADR, 0, offset); 
  }


  return Operand(Address_mode::IMMEDIATE_ADR, 0, 0); 
}


std::vector<Token> Assembler::tokenize_line(std::string line){
  std::vector<std::string> string_tokens;
  std::istringstream* ss = new std::istringstream(line);
  std::string string_token;


  while (*ss >> string_token) {
    if(string_token.empty()) continue;

    if (string_token == ":") {
        string_tokens.back().append(string_token);
        continue;
    }

    if(string_token.back() == ',' && string_token.size() > 1){        //If , is at the end of the word
      string_token.pop_back();
      string_tokens.push_back(string_token);
      string_tokens.push_back(",");
      continue;
    }


    if (string_token.front() == '[') {
        std::smatch match;
        std::regex brackets_regex(R"(\[([^\]]*)\](.*)$)");

        if (std::regex_search(line, match, brackets_regex)) {
            string_tokens.push_back("[" + match[1].str() + "]");

            delete ss;
            ss = new std::istringstream(match[2]);                  //Replace the stringstream source

            // this->log_file<< "CAUGHT: "<<std::endl << match[2] << std::endl;
        }

        continue;
    }

    string_tokens.push_back(string_token);
  }
  delete ss;



  //Classify tokens
  std::vector<Token> tokens;
  for(std::string string_token: string_tokens){
    Token_type type = this->parse_token(string_token);
    if (type == Token_type::NONE)
      throw UnknownToken(string_token, this->line_counter);
    
    if(type == Token_type::LABEL)   //remove : from the label after it is classified
      string_token.pop_back();

    if(type == Token_type::OPERAND_DECIMAL || type == Token_type::OPERAND_HEX || type == Token_type::SYMBOL_INDIRECT)   //remove $ from the token after it is classified  
      string_token = string_token.substr(1);

    tokens.push_back(Token(string_token, type)); 
  }

  
  return tokens;
}



Assembler::Assembler(){

  this->Directive_handlers = {
    {Directive_type::GLOBAL, [&](std::vector<Token>& tokens, const Pass pass){
        if(pass == SECOND_PASS) return;
        //Debug info:
        log_file<<"GLOBAL "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;

        Token_type expected_type = Token_type::SYMBOL;             //Expected token type at starting position

        //.public SYMBOL COMMA SYMBOL COMMA ..
        for(int i = 0; i < tokens.size(); i++){
          auto token = tokens[i];
          if(token.type != expected_type) throw SyntaxError(this->line_counter);
          if(token.type == Token_type::COMMA && i == tokens.size()-1) throw SyntaxError(this->line_counter);     //Last token should not be comma

          //Set next expected token type
          expected_type = expected_type == Token_type::SYMBOL ? Token_type::COMMA : Token_type::SYMBOL;
          if(token.type == Token_type::COMMA) continue;

          //Actual param handling
          Symbol* symbol = symbol_table.get_symbol_by_name(token.name);
          if (symbol) {
            if (symbol->is_extern) throw GlobalExternSymbolError(symbol->name, this->line_counter);
            symbol->is_global = true;
          } 
          else{
            // name, section_name, value, is_global, is_extern, size, defined
            symbol_table.add_symbol(new Symbol(token.name, current_section_name, 0, true, false, -1, false));
          }
            
          
        }


      }
    }, 

    {Directive_type::EXTERN, [&](std::vector<Token>& tokens, const Pass pass){
        if(pass == SECOND_PASS) return; 
        //Debug info:
        log_file<<"EXTERN "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;

        Token_type expected_type = Token_type::SYMBOL;             //Expected token type at starting position

        //.extern SYMBOL COMMA SYMBOL COMMA ..
        for(int i = 0; i < tokens.size(); i++){
          auto token = tokens[i];
          if(token.type != expected_type) throw SyntaxError(this->line_counter);
          if(token.type == Token_type::COMMA && i == tokens.size()-1) throw SyntaxError(this->line_counter);     //Last token should not be comma

          //Set next expected token type
          expected_type = expected_type == Token_type::SYMBOL ? Token_type::COMMA : Token_type::SYMBOL;
          if(token.type == Token_type::COMMA) continue;

          //Actual param handling
          Symbol* symbol = symbol_table.get_symbol_by_name(token.name);

          if (symbol) {
            if (symbol->is_global) throw GlobalExternSymbolError(token.name, this->line_counter);
            if (symbol->defined) throw ImportingDefinedSymbolErorr(token.name, this->line_counter);
            symbol->is_global = true;
          } 
          else{
            // name, section_name, value, is_global, is_extern, size, defined
            symbol_table.add_symbol(new Symbol(token.name, current_section_name, 0, false, true, -1, false)); 
          }
          
          
        }

      }
    }, 

    {Directive_type::SECTION, [&](std::vector<Token>& tokens, const Pass pass){
        if(pass == FIRST_PASS)
        {
          //Debug info:
          log_file<<"SECTION "<< tokens.size()<<std::endl;
          for(auto& it : tokens)
            log_file<<it<<" ";
          log_file<<std::endl;

          //.section SYMBOL 
          if(tokens[0].type != Token_type::SYMBOL) throw SyntaxError(this->line_counter);
          if (symbol_table.get_symbol_by_name(tokens[0].name)) throw SectionAlreadyDefinedError(tokens[0].name, this->line_counter);


          if (Assembler::current_section_name != "UND")
            symbol_table.update_section_size(Assembler::current_section_name, this->location_counter);
          

          // name, section_name, value, is_global, is_extern, size, defined
          symbol_table.add_symbol(new Symbol(tokens[0].name, tokens[0].name, 0, false, false, 0, true));
          
          
          Assembler::current_section_name = tokens[0].name;
          sections.insert({tokens[0].name, Section(tokens[0].name)});
          sections_order.push_back(tokens[0].name);

          this->location_counter = 0;
        }
        else if(pass == SECOND_PASS){
          Assembler::current_section_name = tokens[0].name;
        }

      }
    }, 

    //WORD TAKES 4Bytes for each operand/literal
    {Directive_type::WORD, [&](std::vector<Token>& tokens, const Pass pass){

        if(pass == FIRST_PASS)
        {
          //Debug info:
          log_file<<"WORD "<< tokens.size()<<std::endl;
          for(auto& it : tokens)
            log_file<<it<<" ";
          log_file<<std::endl;
          
          if(current_section_name == "UND") throw NoSectionError(".word", this->line_counter);

          std::unordered_set<Token_type> expected_types = {Token_type::SYMBOL, Token_type::OPERAND_DECIMAL_INDIRECT, Token_type::OPERAND_HEX_INDIRECT};  //Expected token types at starting position


          //.word SYMBOL|(OPERAND_DECIMAL_INDIRECT | OPERAND_HEX_INDIRECT) COMMA SYMBOL|(OPERAND_DECIMAL_INDIRECT | OPERAND_HEX_INDIRECT) .. 
          for(int i = 0; i < tokens.size(); i++){
            auto token = tokens[i];

            if (expected_types.find(token.type) == expected_types.end()) throw SyntaxError(this->line_counter);
            if (token.type == Token_type::COMMA && i == tokens.size() - 1) throw SyntaxError(this->line_counter); //Last token should not be comma
            


            //Set next expected token type
            if (token.type == Token_type::OPERAND_DECIMAL_INDIRECT || token.type == Token_type::OPERAND_HEX_INDIRECT || token.type == Token_type::SYMBOL) {
                expected_types = {Token_type::COMMA};
            } else if (token.type == Token_type::COMMA) {
                expected_types = {Token_type::SYMBOL, Token_type::OPERAND_DECIMAL_INDIRECT, Token_type::OPERAND_HEX_INDIRECT};
                continue;
            }
            if(token.type == Token_type::COMMA) continue;

            //Actual param handling

            uint32_t value = 0; 
            if(token.type == Token_type::OPERAND_DECIMAL_INDIRECT || token.type == Token_type::OPERAND_HEX_INDIRECT){
              try {
                uint32_t value = std::stoull(token.name, nullptr, 
                  (token.type == Token_type::OPERAND_DECIMAL_INDIRECT) ? 10 : 16);

              } catch(const std::exception& e){
                throw UnexpectedError(this->line_counter); 
              }

              if (value > std::pow(2, 32)-1) throw BigOperandError(token.name, this->line_counter);  //Throw error if operand takes more than 32 bits
            }
            else{             //Token is symbol type
               Symbol* symbol = this->symbol_table.get_symbol_by_name(token.name);
               if (symbol == nullptr){
                // name, section_name, value, is_global, is_extern, size, defined
                symbol_table.add_symbol(new Symbol(token.name, current_section_name, this->location_counter, false, false, -1, false));
               }
                  
               relocation_table.add_relocation(new Relocation(token.name, Usage_type::SYMBOL, this->location_counter, current_section_name));

            }


            this->location_counter += 4;
          }
        }
        else if(pass == SECOND_PASS)
        {

          for(int i = 0; i < tokens.size(); i++){
            auto token = tokens[i];
            if(token.type == Token_type::COMMA) continue;

            unsigned int val;
            if(token.type == Token_type::OPERAND_DECIMAL_INDIRECT || token.type == Token_type::OPERAND_HEX_INDIRECT){
              try {
                val = std::stoull(token.name, nullptr, (token.type == Token_type::OPERAND_DECIMAL) ? 10 : 16);

                //Write 4 bytes 
                this->sections.at(Assembler::current_section_name).append_code_byte((val & 0xFF000000) >> 3*8);
                this->sections.at(Assembler::current_section_name).append_code_byte((val & 0x00FF0000) >> 2*8);
                this->sections.at(Assembler::current_section_name).append_code_byte((val & 0x0000FF00) >> 1*8);
                this->sections.at(Assembler::current_section_name).append_code_byte(val & 0x000000FF);

              }
              catch(const std::exception& e) { throw UnexpectedError(this->line_counter); }
            }
            else if(token.type == Token_type::SYMBOL){

              Symbol* symbol = this->symbol_table.get_symbol_by_name(token.name);
              if (symbol == nullptr){
                
                //Write 4 bytes of 0s
                this->sections.at(Assembler::current_section_name).append_code_byte(0x00);
                this->sections.at(Assembler::current_section_name).append_code_byte(0x00);
                this->sections.at(Assembler::current_section_name).append_code_byte(0x00);
                this->sections.at(Assembler::current_section_name).append_code_byte(0x00);

              }else{
                unsigned int val = symbol->value;

                //Write 4 bytes 
                this->sections.at(Assembler::current_section_name).append_code_byte((val & 0xFF000000) >> 3*8);
                this->sections.at(Assembler::current_section_name).append_code_byte((val & 0x00FF0000) >> 2*8);
                this->sections.at(Assembler::current_section_name).append_code_byte((val & 0x0000FF00) >> 1*8);
                this->sections.at(Assembler::current_section_name).append_code_byte(val & 0x000000FF);

              }
              
            }

          }

        }

        
        
      }
    }, 

    {Directive_type::SKIP, [&](std::vector<Token>& tokens, const Pass pass){
        //Debug info:
        if(pass == FIRST_PASS)
        {
          log_file<<"SKIP "<< tokens.size()<<std::endl;
          for(auto& it : tokens)
            log_file<<it<<" ";
          log_file<<std::endl;  


          if(current_section_name == "UND") throw NoSectionError(".skip", this->line_counter);

          //.skip OPERAND_HEX|OPERAND_DECIMAL
          if(tokens[0].type != Token_type::OPERAND_DECIMAL && tokens[0].type != Token_type::OPERAND_HEX) throw SyntaxError(this->line_counter);;

          try {
            unsigned int skip_value = std::stoull(tokens[0].name, nullptr, (tokens[0].type == Token_type::OPERAND_DECIMAL) ? 10 : 16);

            this->location_counter += skip_value;

          } catch(const std::exception& e){
            throw UnexpectedError(this->line_counter);
          }

        }
        else if(pass == SECOND_PASS)
        {
          try {
            unsigned int skip_value = std::stoull(tokens[0].name, nullptr, (tokens[0].type == Token_type::OPERAND_DECIMAL) ? 10 : 16);

            for(int i = 0; i < skip_value; i++)
              this->sections.at(Assembler::current_section_name).append_code_byte(0x00);

          } catch(const std::exception& e){
            throw UnexpectedError(this->line_counter);
          }
        }

      }
    }

  };


  this->Instruction_handlers = {
    {Instruction::HALT, [&](std::vector<Token>& tokens){
        //HALT NO_ARGS
        if(tokens.size()) throw SyntaxError(this->line_counter);

        this->sections.at(Assembler::current_section_name).append_code_byte(instruction_op_codes.at(Instruction::HALT));
      }
    },

    {Instruction::INT, [&](std::vector<Token>& tokens){
        //INT NO_ARGS
        if(tokens.size()) throw SyntaxError(this->line_counter);

        this->sections.at(Assembler::current_section_name).append_code_byte(instruction_op_codes.at(Instruction::INT));
      }
    },

    {Instruction::IRET, [&](std::vector<Token>& tokens){
        //IRET NO_ARGS
        if(tokens.size()) throw SyntaxError(this->line_counter);

        this->sections.at(Assembler::current_section_name).append_code_byte(instruction_op_codes.at(Instruction::IRET));
      }
    },

    {Instruction::RET, [&](std::vector<Token>& tokens){
        //RET NO_ARGS
        if(tokens.size()) throw SyntaxError(this->line_counter);

        this->sections.at(Assembler::current_section_name).append_code_byte(instruction_op_codes.at(Instruction::RET));
      }
    },

    {Instruction::CALL, [&](std::vector<Token>& tokens){
        //DEBUG INFO
        log_file<<"CALL "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;
        
        //CALL OPERAND
        std::vector<std::unordered_set<Token_type>> expected_params = {Assembler::operand_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::CALL);

        if(tokens[0].type == Token_type::OPERAND_REG || tokens[0].type == Token_type::OPERAND_REG_SPEC
          || tokens[0].type == Token_type::OPERAND_DECIMAL_INDIRECT || tokens[0].type == Token_type::OPERAND_HEX_INDIRECT 
          || tokens[0].type == Token_type::SYMBOL)
          op_code |= 0x00; 
        else
          op_code |= 0x01;
        
        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool

        Operand op = this->get_operand(tokens[0], Instruction::CALL); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, op.reg_num, 0, 0, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);


      }
    },

    {Instruction::JMP, [&](std::vector<Token>& tokens){
        //JMP OPERAND
        //DEBUG INFO
        log_file<<"JMP "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;

        std::vector<std::unordered_set<Token_type>> expected_params = {Assembler::operand_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::JMP);

        if(tokens[0].type == Token_type::OPERAND_REG || tokens[0].type == Token_type::OPERAND_REG_SPEC
          || tokens[0].type == Token_type::OPERAND_DECIMAL_INDIRECT || tokens[0].type == Token_type::OPERAND_HEX_INDIRECT 
          || tokens[0].type == Token_type::SYMBOL)
          op_code |= 0x00; 
        else
          op_code |= 0x08;
        

        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool

        Operand op = this->get_operand(tokens[0], Instruction::JMP); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, op.reg_num, 0, 0, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);


      }
    },

    {Instruction::BEQ, [&](std::vector<Token>& tokens){
        log_file<<"BEQ "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;

        //BEQ %gpr1 COMMA %gpr2 COMMA operand

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type, comma, operand_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);


        uint8_t op_code = instruction_op_codes.at(Instruction::BEQ);
        uint8_t gpr1 = this->reg_num(tokens[0]);
        uint8_t gpr2 = this->reg_num(tokens[2]);


        if(tokens[4].type == Token_type::OPERAND_REG || tokens[4].type == Token_type::OPERAND_REG_SPEC
          || tokens[4].type == Token_type::OPERAND_DECIMAL_INDIRECT || tokens[4].type == Token_type::OPERAND_HEX_INDIRECT 
          || tokens[4].type == Token_type::SYMBOL)
          op_code |= 0x01; 
        else
          op_code |= 0x09;
        

        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool
        Operand op = this->get_operand(tokens[4], Instruction::BEQ); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, op.reg_num, gpr1, gpr2, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);

      }
    },

    {Instruction::BNE, [&](std::vector<Token>& tokens){
        //BNE %gpr1 COMMA %gpr2 COMMA operand

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type, comma, operand_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);


        uint8_t op_code = instruction_op_codes.at(Instruction::BNE);
        uint8_t gpr1 = this->reg_num(tokens[0]);
        uint8_t gpr2 = this->reg_num(tokens[2]);


        if(tokens[4].type == Token_type::OPERAND_REG || tokens[4].type == Token_type::OPERAND_REG_SPEC
          || tokens[4].type == Token_type::OPERAND_DECIMAL_INDIRECT || tokens[4].type == Token_type::OPERAND_HEX_INDIRECT 
          || tokens[4].type == Token_type::SYMBOL)
          op_code |= 0x02; 
        else
          op_code |= 0x0A;
        

        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool

        Operand op = this->get_operand(tokens[4], Instruction::BNE); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, op.reg_num, gpr1, gpr2, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);

      }
    },

    {Instruction::BGT, [&](std::vector<Token>& tokens){
        //BGT %gpr1 COMMA %gpr2 COMMA operand

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type, comma, operand_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);


        uint8_t op_code = instruction_op_codes.at(Instruction::BGT);
        uint8_t gpr1 = this->reg_num(tokens[0]);
        uint8_t gpr2 = this->reg_num(tokens[2]);


        if(tokens[4].type == Token_type::OPERAND_REG || tokens[4].type == Token_type::OPERAND_REG_SPEC
          || tokens[4].type == Token_type::OPERAND_DECIMAL_INDIRECT || tokens[4].type == Token_type::OPERAND_HEX_INDIRECT
          || tokens[4].type == Token_type::SYMBOL)
          op_code |= 0x03; 
        else
          op_code |= 0x0B;
        

        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool

        Operand op = this->get_operand(tokens[4], Instruction::BGT); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, op.reg_num, gpr1, gpr2, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);

      }
    },

    {Instruction::PUSH, [&](std::vector<Token>& tokens){
        //PUSH %gpr

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::PUSH);
        uint8_t gpr = this->reg_num(tokens[0]);
        uint8_t sp = this->reg_num(Token("%sp", Token_type::NONE));
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, sp, 0, gpr, 1);  //D is 1

      }
    },

    //PAY ATTENTION ON NEGATIVE D value
    {Instruction::POP, [&](std::vector<Token>& tokens){
        //POP %gpr
        
        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::POP);
        uint8_t gpr = this->reg_num(tokens[0]);
        uint8_t sp = this->reg_num(Token("%sp", Token_type::NONE)); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gpr, sp, 0, -1);  //D is -1

      }
    },

    {Instruction::XCHG, [&](std::vector<Token>& tokens){
        //xchg %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::XCHG);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, 0, gprS, gprD);

      }
    },

    {Instruction::ADD, [&](std::vector<Token>& tokens){
        //add %gprS, %gprD

        log_file<<"ADD "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::ADD);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);

        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);

      }
    },

    {Instruction::SUB, [&](std::vector<Token>& tokens){
        //sub %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::SUB);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);
      }
    },

    {Instruction::MUL, [&](std::vector<Token>& tokens){
        //mul %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::MUL);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);

      }
    },

    {Instruction::DIV, [&](std::vector<Token>& tokens){
        //div %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::DIV);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);

      }
    },

    {Instruction::NOT, [&](std::vector<Token>& tokens){
        //not %gpr

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::NOT);
        uint8_t gpr = this->reg_num(tokens[0]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gpr, gpr);


      }
    },

    {Instruction::AND, [&](std::vector<Token>& tokens){
        //and %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::AND);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);

      }
    },

    {Instruction::OR, [&](std::vector<Token>& tokens){
        //or %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::OR);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);


      }
    },

    {Instruction::XOR, [&](std::vector<Token>& tokens){
        //xor %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::XOR);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);


      }
    },

    {Instruction::SHL, [&](std::vector<Token>& tokens){
        //shl %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::SHL);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);


      }
    },

    {Instruction::SHR, [&](std::vector<Token>& tokens){
        //shr %gprS, %gprD

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::SHR);
        uint8_t gprS = this->reg_num(tokens[0]);
        uint8_t gprD = this->reg_num(tokens[2]);
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gprD, gprD, gprS);


      }
    },

    {Instruction::LD, [&](std::vector<Token>& tokens){
        log_file<<"LD "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;  

        //ld operand, %gpr

        std::vector<std::unordered_set<Token_type>> expected_params = {operand_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::LD);
        uint8_t gpr = this->reg_num(tokens[2]);


        if(tokens[0].type == Token_type::OPERAND_REG || tokens[0].type == Token_type::OPERAND_REG_SPEC
          || tokens[0].type == Token_type::OPERAND_DECIMAL || tokens[0].type == Token_type::OPERAND_HEX 
          || tokens[0].type == Token_type::SYMBOL_INDIRECT)
          op_code |= 0x01; 
        else
          op_code |= 0x02;
        

        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool

        Operand op = this->get_operand(tokens[0], Instruction::LD); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gpr, op.reg_num, 0, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);

      }
    },

    {Instruction::ST, [&](std::vector<Token>& tokens){
        //st %gpr, operand

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, operand_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);


        uint8_t op_code = instruction_op_codes.at(Instruction::ST);
        uint8_t gpr = this->reg_num(tokens[0]);


        if(tokens[2].type == Token_type::OPERAND_REG || tokens[2].type == Token_type::OPERAND_REG_SPEC
          || tokens[2].type == Token_type::OPERAND_DECIMAL || tokens[2].type == Token_type::OPERAND_HEX 
          || tokens[2].type == Token_type::SYMBOL)
          op_code |= 0x00; 
        else
          op_code |= 0x02;          //**MODIFIED**
        

        //OP_CODE|MMMM|AAAA|BBBB|CCCC|DDDD|DDDD|DDDD
        //           4B literal pool

        Operand op = this->get_operand(tokens[2], Instruction::ST); 
        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, op.reg_num, 0, gpr, 0);   
        this->sections.at(Assembler::current_section_name).write_word(op.offset);
        

      }
    },

    {Instruction::CSRRD, [&](std::vector<Token>& tokens){

        log_file<<"CSRRD "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl; 

        //csrrd %csr, %gpr

        log_file<<"CSRRD "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;  

        std::vector<std::unordered_set<Token_type>> expected_params = {status_control_reg_type, comma, reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::CSRRD);
        uint8_t csr = this->reg_num(tokens[0]);
        uint8_t gpr = this->reg_num(tokens[2]);

        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, gpr, csr);   

      }
    },

    {Instruction::CSRWR, [&](std::vector<Token>& tokens){

        log_file<<"CSRWR "<< tokens.size()<<std::endl;
        for(auto& it : tokens)
          log_file<<it<<" ";
        log_file<<std::endl;  

        //csrwr %gpr, %csr

        std::vector<std::unordered_set<Token_type>> expected_params = {reg_type, comma, status_control_reg_type};
        if(!this->syntax_param_check(expected_params, tokens)) throw SyntaxError(this->line_counter);

        uint8_t op_code = instruction_op_codes.at(Instruction::CSRWR);
        uint8_t gpr = this->reg_num(tokens[0]);
        uint8_t csr = this->reg_num(tokens[2]);

        this->sections.at(Assembler::current_section_name).write_instruction((op_code>>4)&0x0F, op_code&0x0F, csr, gpr);   



      }
    }

    

  };

}

Assembler::~Assembler(){
  
}


bool Assembler::syntax_param_check(const std::vector<std::unordered_set<Token_type>>& paramTypes, std::vector<Token>& tokens){
  if(paramTypes.size() != tokens.size()) return false;

  for(int i = 0; i < paramTypes.size(); i++){
    auto expected_param_type = paramTypes[i];
    auto token_type = tokens[i].type;
    if(expected_param_type.find(token_type) == expected_param_type.end()) return false;
  }

  return true;
}


uint8_t Assembler::reg_num(const Token& token){
  std::smatch match;
  std::string reg_num_string; 
  uint8_t reg_num = 0;
  std::string reg_name; 

  if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_REG))){
    reg_num_string = match[1].str();
    try {
      reg_num = std::stoi(reg_num_string);
    }
    catch(const std::exception& e) { throw UnexpectedError(this->line_counter); }
  }
  else if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_REG_SPEC))){
    reg_name = match[1].str();
    if(reg_name == "sp") reg_num = 14;
    else if(reg_name == "pc") reg_num = 15;
  }
  else if (std::regex_search(token.name, match, token_parser.at(Token_type::OPERAND_REG_STATUS_CONTROL))){
    reg_name = match[1].str();
    
    if(reg_name == "status") reg_num = 0; 
    else if(reg_name == "handler") reg_num = 1;
    else if(reg_name == "cause") reg_num = 2;
  }
  


  return reg_num;

}

// loaded_input_file

//Tokenizer removes comments
void Assembler::load_input_file(std::ifstream& inputFile){
  std::string line;

  while (getline(inputFile, line))
    this->loaded_input_file.push_back(line);
  
}


void Assembler::Assemble(std::ifstream& inputFile, std::ofstream& outputFile)
{
    try{
        this->log_file << "Tokenization starting:\n\n";
        this->load_input_file(inputFile);

        this->log_file << "First Pass starting:\n\n";
        this->first_pass();

        this->log_file << "Second Pass starting:\n\n";
        this->second_pass();

        this->log_file << "Writing Output File:\n\n";
        this->write_output_file(outputFile);
        this->log_file << "\nWriting Output File completed\n";

        std::cout<<"Uspesno asembliranje!\n";
    }
    catch(std::exception& e)
    {
      std::cout << e.what() << std::endl;
      this->log_file << "Error: " << e.what() << std::endl;
    }
}



void Assembler::first_pass(){

  std::string line;
  std::smatch match;
  std::regex comment(R"(^\s*#.*\s*$)");
  std::regex comment_remover(R"(^\s*(.*?)(?=\s*#|$))");

  for(auto line: loaded_input_file) {
      this->line_counter++; 

      if (line.empty() || line == "\n" || line.find_first_not_of(' ') == std::string::npos || regex_search(line, comment)) {
        continue;
      }
      if (std::regex_search(line, match, comment_remover)) 
        line = match[1].str();                                            //Line doesnt have comments


      //tokenizing current line 
      
      // this->log_file<<this->line_counter<<" "<<line<<std::endl;
      std::vector<Token> tokens = this->tokenize_line(line);     //Tokens in one line



      if(tokens[0].type == Token_type::END){
        //UPDATE SIZE OF LAST OPENED SECTION
        if (Assembler::current_section_name != "UND")
          symbol_table.update_section_size(Assembler::current_section_name, this->location_counter);

        break;
      }

      Instruction instruction;

      switch (tokens[0].type)
      {
      case Token_type::LABEL:
        this->Handle_label(tokens, FIRST_PASS);
        break;

      case Token_type::DIRECTIVE:
        this->Handle_directive(tokens, FIRST_PASS);
        break;

      case Token_type::INSTRUCTION:
        //TODO: Check this pls
        instruction = Enumerate_instruction(tokens[0].name);

        if(instruction_sizes.find(instruction) != instruction_sizes.end())
          this->location_counter += instruction_sizes.at(instruction);
        else 
          throw UnexpectedError(this->line_counter);
        break;
      
      default:
        throw IllegalLineStarting(tokens[0].name, this->line_counter);
        break;
        
    }
  }
}


void Assembler::second_pass(){
  this->line_counter = 0;
  Assembler::current_section_name = "UND";
  
  std::string line;
  std::smatch match;
  std::regex comment(R"(^\s*#.*\s*$)");
  std::regex comment_remover(R"(^\s*(.*?)(?=\s*#|$))");

  for(auto line: loaded_input_file) {
      this->line_counter++; 

      if (line.empty() || line == "\n" || line.find_first_not_of(' ') == std::string::npos || regex_search(line, comment)) 
        continue;
      
      if (std::regex_search(line, match, comment_remover)) 
        line = match[1].str();                                            //Line doesnt have comments


      //tokenizing current line 
      std::vector<Token> tokens = this->tokenize_line(line);     //Tokens in one line


      if(tokens[0].type == Token_type::END) break;

      switch (tokens[0].type)
      {
      case Token_type::LABEL:
        this->Handle_label(tokens, SECOND_PASS);
        break;

      case Token_type::DIRECTIVE:
        this->Handle_directive(tokens, SECOND_PASS);
        break;

      case Token_type::INSTRUCTION:
        this->Handle_instruction(tokens);
        break;
      
      default:
        throw IllegalLineStarting(tokens[0].name, this->line_counter);
        break;
      
    }
  }

}







void Assembler::Handle_label(std::vector<Token> tokens, const Pass pass){
  if(pass == Pass::SECOND_PASS) return;

  // this->log_file<<"LABEL: "<<tokens[0]<<std::endl;
  std::string label_name = tokens[0].name;
  if(current_section_name == "UND") throw NoSectionError(label_name, this->line_counter);
  
  if (symbol_table.get_symbol_by_name(label_name)){
    Symbol* symbol = symbol_table.get_symbol_by_name(tokens[0].name);

    //Symbol already defined
    if(symbol->defined)
      throw ExistingSymbolError(label_name, this->line_counter);
    
    //Defining extern symbol
    if(symbol->is_extern)
      throw ExternSymbolDefiningErorr(label_name, this->line_counter);


    symbol->value = this->location_counter;
    symbol->section_name = current_section_name;
    symbol->defined = true;
  }
  else{
     // name, section_name, value, is_global, is_extern, size, defined
     symbol_table.add_symbol(new Symbol(label_name, current_section_name,
      this->location_counter, false, false, -1, true));
  }
  

  //If there is instruction after the label, handle it
  if (tokens.size() > 1){
      tokens.erase(tokens.begin()); 

      if(tokens[0].type != Token_type::INSTRUCTION && tokens[0].type != Token_type::DIRECTIVE) throw IllegalLineStarting(tokens[0].name, this->line_counter);
      this->Handle_instruction(tokens);
  }

}

void Assembler::Handle_directive(std::vector<Token> tokens, const Pass pass){
  // this->log_file<<"DIRECTIVE: "<<tokens[0]<<std::endl;

  Directive_type dir_type = Enumerate_directive(tokens[0].name);
  tokens.erase(tokens.begin()); 
  this->Directive_handlers.at(dir_type)(tokens, pass);

}

void Assembler::Handle_instruction(std::vector<Token> tokens){
  // this->log_file<<"INSTRUCTION: "<<tokens[0]<<std::endl;

  Instruction instruction = Enumerate_instruction(tokens[0].name);

  if(current_section_name == "UND") throw NoSectionError(tokens[0].name, this->line_counter);

  tokens.erase(tokens.begin());
  if(this->Instruction_handlers.find(instruction) != this->Instruction_handlers.end())
    this->Instruction_handlers.at(instruction)(tokens);

}



void Assembler::write_output_file(std::ofstream& output_file){

  output_file << "Symbols Table" << std::endl;
  output_file << std::left <<
      std::setw(25) << "SymbolName" <<
      std::setw(25) << "SectionName" <<
      std::setw(15) << "Value" <<
      std::setw(15) << "IsGlobal" <<
      std::setw(15) << "IsExtern" <<
      std::setw(15) << "Number" <<
      std::setw(15) << "Size" <<
      std::setw(15) << "Defined" <<
      std::endl;

  //PRINT SYMBOL TABLE
  output_file << symbol_table << std::endl;

  output_file << "Sections code" << std::endl;
  for (const std::string& section_name : this->sections_order) 
      output_file << sections.at(section_name) << std::endl;

  
  output_file << "Relocations Table" << std::endl;
  output_file << std::left <<
      std::setw(20) << "SymbolName" <<
      std::setw(20) << "RelocationType" <<
      std::setw(20) << "Location" <<
      std::setw(20) << "SectionName" <<
      std::endl;

  //PRINT RELOCATION TABLE
  output_file << relocation_table; 




  // // DEBUG MACHINE CODE AND RELOCATIONS
  // // INSERT SOMEWHERE IN YOUR CODE bgt %r5, %r6, mathDiv  and test the relocations
  // std::vector<uint8_t> machine_code;
  // for (const std::string& section_name : this->sections_order) 
  //   for(auto it : sections.at(section_name).section_code){
  //     machine_code.push_back(static_cast<uint32_t>(static_cast<int>(it)));
  //   }
      

  // uint32_t write_val = 0xABCDEF23;

  // //Iterate through the relocation entries and correct the machine code
  // for (auto r : this->relocation_table.get_relocations("value4")) {
  //   machine_code[r->location] = ((write_val & 0xFF000000) >> 3*8);
  //   machine_code[r->location + 1] = ((write_val & 0x00FF0000) >> 2*8);
  //   machine_code[r->location + 2] = ((write_val & 0x0000FF00) >> 1*8);
  //   machine_code[r->location + 3] = write_val & 0x000000FF;
  // }


  // int i = 0;
  // for(auto it : machine_code){
  //   if (i % 8 == 0) this->log_file << std::endl;
  //   this->log_file << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast<uint32_t>(static_cast<int>(it)) << " ";
  //   i++;
  // }


}

int main(int argc, char ** argv) {
    try{
      
      if (argc != 4 || (strcmp(argv[1], "-o") != 0 && strcmp(argv[2], "-o") != 0))
        throw InvalidArguments(argv[0]);

      std::string input_file_name;
      std::string output_file_name;

      if (strcmp(argv[2], "-o") == 0) {
          // ./asembler file1.s -o file2.o
          input_file_name = argv[1];
          output_file_name = argv[3];
          
      } else {
          // ./asembler -o file2.o file1.s
          input_file_name = argv[3];
          output_file_name = argv[2];
      }

      std::ifstream input_file(input_file_name);
      std::ofstream output_file(output_file_name);

      if (!input_file.is_open())
          throw FileNameError(input_file_name);
      

      if (!output_file.is_open())
        throw FileNameError(output_file_name);


      Assembler* assembler = new Assembler();
      assembler->Assemble(input_file, output_file);
      delete assembler; 


      input_file.close();
      output_file.close();
    }
    catch(const std::exception& e) {
      std::cout << e.what() << '\n';
    }


    return 0;
}
