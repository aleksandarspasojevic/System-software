#include "../inc/Linker.hpp"


std::ofstream Linker::log_file("linker.log");



Linker::Linker(bool hex_option, std::map<std::string, uint64_t> section_places): hex_option(hex_option), section_places(section_places){}

Linker::~Linker(){
  this->log_file.close();
}

//Loads all relevant data for each object into Object_file structure in its deserialized form
void Linker::decompose_input_files(std::vector<std::string> input_file_names){

  std::ifstream input_file;
  std::string line;

  for(std::string file_name : input_file_names){
    input_file = std::ifstream(file_name);

    if (!input_file.is_open())
      throw FileNameError(file_name);



    object_files.insert({file_name, Object_file(file_name)});
    object_files_order.push_back(file_name);               //push file_name in order list

    ObjectFileReadingType currentObjectType = ObjectFileReadingType::ENTERING_FILE;
    while (getline(input_file, line)){

      //empty line
      if (line.empty() || line == "\n" || line.find_first_not_of(' ') == std::string::npos) {   
        continue;
      }

      //REMOVE HEADERS
      if (line.find("Symbols Table") != std::string::npos) {
        currentObjectType = ObjectFileReadingType::SYMBOL_TABLE;
        //remove header
        getline(input_file, line);
        continue;
      }

      //SECTIONS CODE BEGIN
      if (line.find("Sections code") != std::string::npos) {
        currentObjectType = ObjectFileReadingType::SECTIONS;
        continue;
      }

      //Section header
      //Section: section_name
      if (line.find("Section:") != std::string::npos) {
        std::istringstream iss(line);
        std::string section_name;
        iss>>section_name>>section_name;      //read second word

        // this->log_file<<section_name<<std::endl;

        object_files.at(file_name).addSection(section_name);

        continue;
      }

      //SECTIONS CODE END
      //RELOCATION ENTRIES BEGIN
      if (line.find("Relocations Table") != std::string::npos) {
        currentObjectType = ObjectFileReadingType::RELOCATION_TABLE;

        //remove header
        getline(input_file, line);


        continue;
      }


      //BASED ON currentObjectType deserialize object file parts

      switch (currentObjectType){
      case ObjectFileReadingType::SYMBOL_TABLE:

        object_files.at(file_name).symbol_table.add_symbol(Symbol::deserialize_symbol(line));

        break;
      case ObjectFileReadingType::SECTIONS:

        Section::deserialize_line_section_code(object_files.at(file_name).sections.at(object_files.at(file_name).current_section_name), line);

        break;
      case ObjectFileReadingType::RELOCATION_TABLE:

        object_files.at(file_name).relocation_table.add_relocation(Relocation::deserialize_relocation_entry(line));
        
        break;
      
      default:
        break;
      }


    }
    

    input_file.close();
  }

  // DEBUGGING:
  // this->log_file<<object_files.at("./tests/javni_test/main.o").symbol_table;
  // this->log_file<<object_files.at("./tests/javni_test/main.o").sections.at("my_code");
  // this->log_file<<object_files.at("./tests/javni_test/main.o").relocation_table;
  
}



void Linker::fill_symbol_table() {

  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);


    for(auto it: object_file->symbol_table.table){
      Symbol* symbol = it.second;

      
      auto linker_section = this->linker_sections.find(std::make_pair(symbol->name, object_file_name));
      if (linker_section == this->linker_sections.end()){ 
        if(symbol->is_section())  
          this->linker_sections.insert({std::make_pair(symbol->name, object_file_name), symbol});
      }

      Symbol* linker_table_symbol = this->linker_symbol_table.get_symbol_by_name(symbol->name);
      if(linker_table_symbol != nullptr){
        //Symbol exists in linker symbol table

        //Found symbol who is defining symbol for linker table
        if (linker_table_symbol->is_extern && symbol->is_global) { 
          linker_table_symbol->set_symbol(symbol->name, symbol->section_name, symbol->value,
            symbol->is_global, symbol->is_extern, symbol->size, symbol->defined, object_file_name);
        }

        //SECTION LABEL CONFLICT
        else if(linker_table_symbol->is_section() && !symbol->is_section() || !linker_table_symbol->is_section() && symbol->is_section()){
          throw LabelSymbolConflict(symbol->name);
        }

        //GLOBAL SYMBOLS CONFLICT
        else if(linker_table_symbol->is_global && symbol->is_global){
          throw MultipleSymbolDefinition(symbol->name);
        }
        

      }
      else{
        if(symbol->is_extern || symbol->is_global){                  //Do not insert local symbols
          Symbol* s = new Symbol(*symbol);
          s->file_name = object_file_name;
          this->linker_symbol_table.add_symbol(s);
        }

      }

    }

  }
}


void Linker::undefined_symbol_check(){
  for(auto& it: this->linker_symbol_table.table) {
    Symbol* symbol = it.second;

    if (symbol->is_extern || symbol->section_name == "UND") 
      throw SymbolUnresolvedError(symbol->name);
  }
}


void Linker::Link(std::vector<std::string> input_file_names, std::string output_file_name){

  try{
      this->log_file << "Loading input files started:\n\n";
      this->decompose_input_files(input_file_names);

      this->fill_symbol_table();

      this->log_file << "Linker symbol table\n";
      this->log_file << std::left <<
      std::setw(25) << "SymbolName" <<
      std::setw(25) << "SectionName" <<
      std::setw(15) << "Value" <<
      std::setw(15) << "IsGlobal" <<
      std::setw(15) << "IsExtern" <<
      std::setw(15) << "Number" <<
      std::setw(15) << "Size" <<
      std::setw(15) << "Defined" <<
      std::setw(50) << "File" <<
      std::endl;
      this->log_file<<this->linker_symbol_table;
      
      this->undefined_symbol_check();

      this->formSections();

      this->check_sections_overlapping();

      this->resolve_relocations();

      this->log_file << "\n\nWriting Output File:\n";
      std::ofstream output_file(output_file_name);

      //WRITE OUTPUT FILE
      if(this->hex_option) this->write_output_file(output_file);


      output_file.close();
      this->log_file << "Writing Output File completed\n";

      std::cout<<"Linking succeed!\n";
  }
  catch(std::exception& e)
  {
    std::cout << e.what() << std::endl;
    this->log_file << "Error: " << e.what() << std::endl;
  }
}



void Linker::formSections(){

  std::map<std::string, int> placed_sections;

  //Placing sections with -place attribute
  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);

    for(auto it: object_file->sections_order){
      Section& section = object_file->sections.at(it);
      // std::cout<<it<<" "<<section.section_code.size()<<std::endl;

      //Exists section with same name, that is already placed
      if (placed_sections.find(section.name) != placed_sections.end()) {
        int current_section_size = this->linker_sections.at(std::make_pair(section.name, object_file_name))->size;

        section.location = placed_sections[section.name];  //write section location and place it
        section.placed = true;

        this->linker_sections.at(std::make_pair(section.name, object_file_name))->value = placed_sections[section.name];
        placed_sections[section.name] += current_section_size;

        // std::cout<<"SECTION "<<placed_sections[section.name];
      }

      //SEARCH FOR PLACEMENT LOCATIONS
      auto section_place = this->section_places.find(section.name);
      if (section_place != this->section_places.end()){
        // std::cout<<it<<" "<<section_place->second<<std::endl;

        section.location = section_place->second;  //write section location and place it
        section.placed = true;


        if (placed_sections.find(section.name) == placed_sections.end()) {
          int current_section_size = this->linker_sections.at(std::make_pair(section.name, object_file_name))->size;

          placed_sections.insert({section.name, section.location + current_section_size});
          
          this->linker_sections.at(std::make_pair(section.name, object_file_name))->value = section.location;
        }

        //Remove section place after using it
        this->section_places.erase(section_place);
      }

    }
  }

  

  //Find the higest section location placed with -place attribute
  int placing_location = -INT32_MAX;
  for(auto it : placed_sections){
    if(it.second > placing_location) placing_location = it.second;
  }

  //Default placing location is 0
  if(placed_sections.empty()) placing_location = 0;

  //Placing other sections
  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);
    for(auto it: object_file->sections_order){
      Section& section = object_file->sections.at(it);

      if(!section.placed){
        section.location = placing_location;  //write section location and place it
        section.placed = true;
        this->linker_sections.at(std::make_pair(section.name, object_file_name))->value = placing_location;

        placing_location += section.section_code.size();
      }

    }
  }


  //Fill the output sections references
  this->log_file<<"\nSECTION ORDER\n";
  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);
    for(auto it: object_file->sections_order){
      Section* section = &object_file->sections.at(it);
      this->log_file<<it<<" "<<std::dec<<section->section_code.size()<<" "<<std::hex<<section->location<<std::endl;
      
      this->output_sections.insert({std::make_pair(section->name, object_file_name), section});
      this->output_sections_order.push_back(std::make_pair(section->name, object_file_name));  //push full section naming data in order list
    }
  }


  //ORDER THE OUPUT SECTIONS BY SECTION LOCATIONS
  std::sort(this->output_sections_order.begin(), this->output_sections_order.end(), [this](const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
      return this->output_sections.at(a)->location < this->output_sections.at(b)->location;
  });


  //LOGGING THE OUTPTUT SECTION ORDERING
  this->log_file<<"\nOutput section ordering\n";
  for(auto it : this->output_sections_order){
    this->log_file<<it.second<<" LOCATION:"<<this->output_sections.at(it)->location<<std::endl;
  }

  
  //LOGGING linker sections
  for (const auto& entry : this->linker_sections) {
    const std::pair<std::string, std::string>& keyPair = entry.first;
    const Symbol* symbol = entry.second;
    this->log_file << "Section: " << keyPair.first << ", " << keyPair.second << std::endl<< *symbol<< std::endl;
  }



}


inline bool inside(int num, int a, int b) {
  return num > a && num < b;
}

void Linker::check_sections_overlapping(){
  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);
    for(auto it: object_file->sections_order){
      Section sec1 = object_file->sections.at(it);
    
      for (std::string object_file_name : this->object_files_order){
        Object_file* object_file = &this->object_files.at(object_file_name);
        for(auto it: object_file->sections_order){
          Section sec2 = object_file->sections.at(it);

          // std::cout<<sec1.name<<" "<<sec2.name<<std::endl;
          //PAIRS sec1, sec2 

          if (sec1.placed && sec2.placed && sec1.name != sec2.name) {
            if (inside(sec2.location, sec1.location, sec1.location + sec1.section_code.size()) ||
            inside(sec2.location + sec2.section_code.size(), sec1.location, sec1.location + sec1.section_code.size()))
              throw SectionsOverlapError(sec1.name, sec2.name);
          }

        }
      }

    }
  }
}


void Linker::resolve_relocations(){
  //DEBUG INFO
  this->log_file<<"\n\nRELOCATION ENTRIES:\n";
  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);
    this->log_file<<object_file->name<<std::endl;
    this->log_file<<object_file->relocation_table;
  }

  this->log_file<<"\n\nRELOCATION SYMBOLS\n";
  for (std::string object_file_name : this->object_files_order){
    Object_file* object_file = &this->object_files.at(object_file_name);
    for(auto it: object_file->relocation_table.table){
      Relocation* relocation = it.second;

      Symbol* symbol = object_file->symbol_table.get_symbol_by_name(relocation->symbol_name);
      
      if(symbol->defined){
        //Symbol is locally defined

        //RELOCATE

        //TEST!!!!!!!!!!
        Section& section = object_file->sections.at(symbol->section_name);
        uint32_t symbol_val = symbol->value + section.location;   

        //Iterate through the relocation entries and correct the machine code
        for (auto r : object_file->relocation_table.get_relocations(symbol->name)) {
          //Get section where symbol is used - written in relocation entry
          Section& section = object_file->sections.at(r->section_name);

          this->log_file<<"DEFINED SYMBOL "<<std::endl;
          this->log_file<<*symbol;
          this->log_file<<section.name<<" "<<section.location<<std::endl<<std::endl;
          // std::cout<<object_file_name<<" "<<r->symbol_name<<" "<<r->location<<" "<<section.section_code.size()<<std::endl;
          
          section.section_code[r->location] = ((symbol_val & 0xFF000000) >> 3*8);
          section.section_code[r->location + 1] = ((symbol_val & 0x00FF0000) >> 2*8);
          section.section_code[r->location + 2] = ((symbol_val & 0x0000FF00) >> 1*8);
          section.section_code[r->location + 3] = symbol_val & 0x000000FF;
        }


      }
      else{
        //Symbol is in global linker table
        symbol = this->linker_symbol_table.get_symbol_by_name(symbol->name);
        

        //RELLOCATE 

        //TEST!!!!!!!
        Symbol* section_symbol = this->linker_sections.at(std::make_pair(symbol->section_name, symbol->file_name));
        uint32_t symbol_val = symbol->value + section_symbol->value; 


        for (auto r : object_file->relocation_table.get_relocations(symbol->name)) {
          //Get section where symbol is used - written in relocation entry
          Section& section = object_file->sections.at(r->section_name);

          this->log_file<<"LOCALLY UNDEFINED SYMBOL "<<std::endl;
          this->log_file<<*symbol;
          this->log_file<<section.name<<" "<<section.location<<std::endl<<std::endl;
          // std::cout<<object_file_name<<" "<<r->symbol_name<<" "<<r->location<<" "<<section.section_code.size()<<std::endl;
          
          section.section_code[r->location] = ((symbol_val & 0xFF000000) >> 3*8);
          section.section_code[r->location + 1] = ((symbol_val & 0x00FF0000) >> 2*8);
          section.section_code[r->location + 2] = ((symbol_val & 0x0000FF00) >> 1*8);
          section.section_code[r->location + 3] = symbol_val & 0x000000FF;
        }

      }

    }
  }

}


void Linker::write_output_file(std::ofstream& output_file){
  
  for(auto it : this->output_sections_order)
    this->output_sections.at(it)->hex_output(output_file); 
  

  // for (std::string object_file_name : this->object_files_order){
  //   Object_file* object_file = &this->object_files.at(object_file_name);
  //   for(auto it: object_file->sections){
      
  //   }
  // }
}



int main(int argc, char* argv[]) {
  try {

    bool hex_option = false;
    std::regex input_file_regex(R"(^.*\.o$)");
    std::regex section_place_regex(R"(^\s*-place=(\w+)@(\d+|0x[0-9a-fA-F]+)\s*$)"); 
    std::smatch match;
    std::vector<std::string> input_file_names;
    std::string output_file_name;
    std::map<std::string, uint64_t> section_places;
    
    for(uint16_t i = 1; i < argc; i++){
      std::string token = argv[i];

      //-o
      if (token.find("-o") != std::string::npos) {
        output_file_name = argv[++i];
        continue;
      }

      //-hex
      if (token.find("-hex") != std::string::npos) {
        hex_option = true;
        continue;
      }

      if (std::regex_search(token, input_file_regex)) {
        input_file_names.push_back(token);
        continue;
      }

      if (regex_search(token, match, section_place_regex)) {
        
        std::string section_name = match[1];
        std::string location = match[2];
        section_places.insert({section_name, stoull(location, nullptr, 0)});      //automatic base converting

        //DEBUG INFO
        // std::cout<<section_name<<" "<<section_places.at(section_name)<<std::endl;

        continue;
      }


      //If checking propagated to this point, no hanlders handled it - THROW 
      throw InvalidLinkerCmdArgs(token);
    }


    Linker* linker = new Linker(hex_option, section_places);
    linker->Link(input_file_names, output_file_name);
    delete linker;

  }

  catch(const std::exception& e) {
    std::cout << e.what() << '\n';
  }


  return 0;
}
