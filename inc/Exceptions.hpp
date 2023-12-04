#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <string>
#include <exception>


class InvalidArguments : public std::exception {
private:
    std::string error_message;

public:
    explicit InvalidArguments(const std::string& filename)
        : error_message("Usage: " + filename + " inputfile.s -o outputfile.o") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class InvalidLinkerCmdArgs : public std::exception {
private:
    std::string error_message;

public:
    explicit InvalidLinkerCmdArgs(const std::string& token)
        : error_message("Usage: " + token + "-hex -place=section@address -o output.hex input1.o input2.o") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class InvalidEmulatorCmdArgs : public std::exception {
private:
    std::string error_message;

public:
    explicit InvalidEmulatorCmdArgs()
        : error_message("Usage: ./emulator mem_content.hex") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class FileNameError : public std::exception {
private:
    std::string error_message;

public:
    explicit FileNameError(const std::string& filename)
        : error_message("File error: file \"" + filename + "\" does not exist") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class UnknownToken : public std::exception {
private:
    std::string error_message;

public:
    explicit UnknownToken(const std::string& token_name, const uint32_t line)
        : error_message("Illegal token \"" + token_name + "\" at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class UnknownSection : public std::exception {
private:
    std::string error_message;

public:
    explicit UnknownSection(const std::string& section_name)
        : error_message("Unknown section \"" + section_name + "\" for section placement ") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class IllegalLineStarting : public std::exception {
private:
    std::string error_message;

public:
    explicit IllegalLineStarting(const std::string& token_name, const uint32_t line)
        : error_message("Illegal line starting \"" + token_name + "\" at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class ExistingSymbolError : public std::exception {
private:
    std::string error_message;

public:
    explicit ExistingSymbolError(const std::string& symbolName, const uint32_t line)
        : error_message("Symbol already exists in the symbol table \"" + symbolName + "\" at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class SymbolUnresolvedError : public std::exception {
private:
    std::string error_message;

public:
    explicit SymbolUnresolvedError(const std::string& symbolName)
        : error_message("Symbol \"" + symbolName + "\" could not be resolved, due its not defined") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class ExternSymbolDefiningErorr : public std::exception {
private:
    std::string error_message;

public:
    explicit ExternSymbolDefiningErorr(const std::string& symbolName, const uint32_t line)
        : error_message("Defining external symbol \"" + symbolName + "\" at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};



class SyntaxError : public std::exception {
private:
    std::string error_message;

public:
    explicit SyntaxError(const uint32_t line)
        : error_message("Syntax error at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};



class GlobalExternSymbolError : public std::exception {
private:
    std::string error_message;

public:
    explicit GlobalExternSymbolError(const std::string& symbolName, const uint32_t line)
        : error_message("Symbol \"" + symbolName + "\" cannot be global and exern at the same time at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class MultipleSymbolDefinition : public std::exception {
private:
    std::string error_message;

public:
    explicit MultipleSymbolDefinition(const std::string& symbolName)
        : error_message("Symbol \"" + symbolName + "\" cannot be public and defined in multiple files! ") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class LabelSymbolConflict : public std::exception {
private:
    std::string error_message;

public:
    explicit LabelSymbolConflict(const std::string& symbolName)
        : error_message("Symbol \"" + symbolName + "\" can not be label at the same time! ") {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class ImportingDefinedSymbolErorr : public std::exception {
private:
    std::string error_message;

public:
    explicit ImportingDefinedSymbolErorr(const std::string& symbolName, const uint32_t line)
        : error_message("Symbol \"" + symbolName + "\" cannot be imported when its already defined at line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class SectionAlreadyDefinedError : public std::exception {
private:
    std::string error_message;

public:
    explicit SectionAlreadyDefinedError(const std::string& sectionName, const uint32_t line)
        : error_message("Section \"" + sectionName + "\" is already defined. Line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class NoSectionError : public std::exception {
private:
    std::string error_message;

public:
    explicit NoSectionError(const std::string& tokenName, const uint32_t line)
        : error_message("Token \"" + tokenName + "\" should be defined inside a section. Line: " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class SectionsOverlapError : public std::exception {
private:
    std::string error_message;

public:
    explicit SectionsOverlapError(const std::string& section1, const std::string& section2)
        : error_message("Section \"" + section1 + "\" is overlapping with section " + section2) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class BigOperandError : public std::exception {
private:
    std::string error_message;

public:
    explicit BigOperandError(const std::string& tokenName, const uint32_t line)
        : error_message("Operand \"" + tokenName + "\" value is too big at line " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class UndefinedSymbolError : public std::exception {
private:
    std::string error_message;

public:
    explicit UndefinedSymbolError(const std::string& tokenName, const uint32_t line)
        : error_message("Symbol \"" + tokenName + "\" does not exist. Error one the line " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class JumpInstructionOperandNotSUpported : public std::exception {
private:
    std::string error_message;

public:
    explicit JumpInstructionOperandNotSUpported(const std::string& tokenName, const uint32_t line)
        : error_message("Operand \"" + tokenName + "\" is not supported in jumping instructions. Error one the line " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class UnexpectedError : public std::exception {
private:
    std::string error_message;

public:
    explicit UnexpectedError(const uint32_t line)
        : error_message("Unexpected error at line " + std::to_string(line)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};



class UnrecognizedOperactionCode : public std::exception {
private:
    std::string error_message;

public:
    explicit UnrecognizedOperactionCode(const int op_code)
        : error_message("Illegal operation code detected: " + std::to_string(op_code)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};


class MemoryReadViolation : public std::exception {
private:
    std::string error_message;

public:
    explicit MemoryReadViolation(const int address)
        : error_message("Adress violation! Could not read from the address: " + std::to_string(address)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};

class MemoryWriteViolation : public std::exception {
private:
    std::string error_message;

public:
    explicit MemoryWriteViolation(const int address)
        : error_message("Adress violation! Could not write to the address: " + std::to_string(address)) {}

    const char* what() const noexcept override {
        return error_message.c_str();
    }
};



#endif