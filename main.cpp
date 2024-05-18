#include <iostream>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <iomanip>
#include <sstream>

//Register file
int16_t reg[8] = {0,1,2,3,4,5,6,7};
std::map<int, int16_t> mem;


struct Instruction{
    std::string op;
    int16_t rs1;
    int16_t rs2;
    int16_t rd;
    int16_t imm;
    int inst_index;
    

};

struct Reservation_Station{
    bool busy = false;
    bool finished_exec=false;
    bool finished_wb;
    bool not_allowed=false;
    int16_t Vj, Vk,A;
    std::string Qj, Qk, op;
    int numOfCyclesRemaining = 0;//for execution
    Instruction inst;
    bool branch_issue=false;
    // Tracking cycles each stage
    int issueCycle = 0;
    int startExecCycle = 0;
    int endExecCycle = 0;
    int writeBackCycle = 0;
    int RS_index;
};
struct Register_State{
    std::string Qi;
    int index;
};
class Memory{
public:
    std::map<int,int16_t>data;
    void write(int address, int16_t value)
    {
        data[address]=value;
    }
    int16_t read(int address) {
            return data[address];
        }
};
class Tomasulo_Algorithm{
public:
    
    std::map<std::string, std::string> op_to_RS = {
        {"ADD", "ADD/ADDI"},
        {"ADDI", "ADD/ADDI"},
        {"NAND", "NAND"},
        {"MUL", "MUL"},
        {"Store","Store"},
        {"Load","Load"},
        {"BEQ","BEQ"},
        {"Call","Call/RET"},
        {"Ret","Call/RET"}
    };
    std::map<std::string, std::vector<Reservation_Station>> RS;
    std::vector<Register_State> dest_register;
    std::queue<int> store_load_index;
    int idx=0;

    Tomasulo_Algorithm() {

        dest_register.resize(8);
         
    }
    
    //bonus
    
    void Initialize_Reservation_station(int l,int s,int b, int cr, int a, int n, int m){
        RS["Load"].resize(l);
        RS["Store"].resize(s);
        RS["BEQ"].resize(b);
        RS["CALL/RET"].resize(cr);
        RS["ADD/ADDI"].resize(a);
        RS["NAND"].resize(n);
        RS["MUL"].resize(m);
        
    }

    void Initialize_numberOfCycles() {
        for (auto &station : RS["Load"]) { station.numOfCyclesRemaining = 6; }
        for (auto &station : RS["Store"]) { station.numOfCyclesRemaining = 6; }
        for (auto &station : RS["BEQ"]) { station.numOfCyclesRemaining = 1; }
        for (auto &station : RS["CALL/RET"]) { station.numOfCyclesRemaining = 1; }
        for (auto &station : RS["ADD/ADDI"]) { station.numOfCyclesRemaining = 2; }
        for (auto &station : RS["NAND"]) { station.numOfCyclesRemaining = 1; }
        for (auto &station : RS["MUL"]) { station.numOfCyclesRemaining = 8; }
    }
    
    void operations_exec(const Instruction &inst,Memory &memory)
    {
        if(inst.op=="ADD" && inst.rd!=0)
        {
            reg[inst.rd]=reg[inst.rs1]+reg[inst.rs2];
        }
        else if(inst.op=="ADDI" && inst.rd!=0)
        {
            reg[inst.rd]=reg[inst.rs1]+inst.imm;
        }else if(inst.op=="MUL" && inst.rd!=0)
        {
            reg[inst.rd]=reg[inst.rs1]*reg[inst.rs2];
        }else if(inst.op=="NAND" && inst.rd!=0){
            reg[inst.rd]=~(reg[inst.rs1]&reg[inst.rs2]);
        }else if(inst.op=="Load" ){
            reg[inst.rd]=memory.read(inst.rs1);
        }else if(inst.op=="Store" ){
            memory.write(inst.rs2, reg[inst.rs1]);
        }else if(inst.op=="Call" ){
            
        }else if(inst.op=="Ret" )
        {
            
        }else if(inst.op=="BEQ" )
        {
            
        }
        
    }
    bool issue( const Instruction &inst, int cycles){
        bool issue=false;
        Instruction copy=inst;
        if(inst.op=="ADD" ||inst.op=="ADDI"||inst.op=="MUL"||inst.op=="NAND"){
            const std::string& rs_name = op_to_RS[inst.op];
            for( auto &station : RS[rs_name]){
                if (!station.busy) {
                    station.busy = true;
                    station.op = inst.op;
                    station.issueCycle = cycles;
                    
                    copy.inst_index= ++idx;
                    if (!dest_register[inst.rs1].Qi.empty()) {
                        station.Qj = dest_register[inst.rs1].Qi;
                        station.Vj = 0;
                    } else {
                        station.Vj = reg[inst.rs1];
                        station.Qj = "";
                    }

                    if (!dest_register[inst.rs2].Qi.empty()) {
                        station.Qk = dest_register[inst.rs2].Qi;
                        station.Vk = 0;
                    } else {
                        station.Vk = reg[inst.rs2];
                        station.Qk = "";
                    }

                    dest_register[inst.rd].Qi =rs_name;
                    dest_register[inst.rd].index=copy.inst_index;
                    issue = true;
                    std::cout<<"Counter:"<<idx<<":Station_RS_index:"<<copy.inst_index<<":dest_register_index"<<dest_register[inst.rd].index<<std::endl;
                    break;
                }
            }
        }
        else if (inst.op == "Load") {
               const std::string& rs_name = op_to_RS[inst.op];
               for (auto &station : RS[rs_name]) {
                   if (!station.busy) {
                       station.issueCycle = cycles;
                       station.op = inst.op;
                       copy.inst_index= ++idx;
                       store_load_index.push(copy.inst_index);//while execution i will execute store,load in order based on the index
                       if (!dest_register[inst.rs1].Qi.empty()) {
                           station.Qj = dest_register[inst.rs1].Qi;
                           station.Vj = 0;
                       } else {
                           station.Vj = reg[inst.rs1];
                           station.Qj = "";
                       }
                       station.A = inst.imm;
                       station.busy = true;
                       dest_register[inst.rd].Qi = rs_name;//worked
                       dest_register[inst.rd].index=copy.inst_index;
                       issue = true;
                       std::cout<<"Counter:"<<idx<<":Station_RS_index:"<<copy.inst_index<<":dest_register_index"<<dest_register[inst.rd].index<<std::endl;
                       break;
                   }
               }
           } else if (inst.op == "Store") {
               const std::string& rs_name = op_to_RS[inst.op];
               for (auto &station : RS[rs_name]) {
                   
                   if (!station.busy) {
                       station.busy = true;
                       copy.inst_index= ++idx;
                       station.issueCycle = cycles;//Check if it is station or inst
                       station.op = inst.op;
                       
                       store_load_index.push(copy.inst_index);//for execution purpose
                       
                       if (!dest_register[inst.rs1].Qi.empty()) {
                           station.Qj = dest_register[inst.rs1].Qi;
                           station.Vj = 0;
                       } else {
                           station.Vj = reg[inst.rs1];
                           station.Qj = "";
                       }
                       if (!dest_register[inst.rs2].Qi.empty()) {
                           station.Qk = dest_register[inst.rs2].Qi;
                           station.Vk = 0;
                       } else {
                           station.Vk = reg[inst.rs2];
                           station.Qk = "";
                       }
                       station.A = inst.imm; // Assuming rd holds the memory address
//                       dest_register[inst.rs2].Qi = rs_name; (check if nothing depend on store)
//                       dest_register[inst.rs2].index=station.RS_index;
                       issue = true;
                       
                       
                       break;
                   }
               }
           }
        else if(inst.op=="BEQ")
        {
            
            const std::string& rs_name = op_to_RS[inst.op];
            for (auto &station : RS[rs_name]) {
                
                if (!station.busy) {
                    station.busy = true;
                    
                    copy.inst_index= ++idx;
                    station.issueCycle = cycles;//Check if it is station or inst
                    station.op = inst.op;
                    if (!dest_register[inst.rs1].Qi.empty()) {
                        station.Qj = dest_register[inst.rs1].Qi;
                        station.Vj = 0;
                    } else {
                        station.Vj = reg[inst.rs1];
                        station.Qj = "";
                    }
                    if (!dest_register[inst.rs2].Qi.empty()) {
                        station.Qk = dest_register[inst.rs2].Qi;
                        station.Vk = 0;
                    } else {
                        station.Vk = reg[inst.rs2];
                        station.Qk = "";
                    }
                    station.A = inst.imm; // Assuming rd holds the memory address
                    dest_register[inst.rs2].Qi = rs_name; //(check if nothing depend on store)
                    dest_register[inst.rs2].index= copy.inst_index;
                    issue = true;
                    
                    
                    break;
                }
            }
        }
        return issue;
    }
    
    void Execute(const Instruction &inst, int cycles){
            const std::string& rs_name = op_to_RS[inst.op];
            for (auto &station : RS[rs_name]) {
                   
                  
                  if (station.busy && station.Qj.empty() && station.Qk.empty() ) {
                      station.startExecCycle = cycles;  // Mark the start of execution
                  }

                  if (station.startExecCycle > 0 && station.endExecCycle == 0) {
                      
                      if (station.numOfCyclesRemaining > 0) {
                          station.numOfCyclesRemaining--;
                      }
                      if (station.numOfCyclesRemaining == 0) {
                          station.endExecCycle = cycles;
                          station.finished_exec=1;
                          
                          
                      }
                  }
              }

    }
    
    int16_t FP_operation(const Instruction &inst){
        int16_t result=0;
        if(inst.op=="ADD" && inst.rd!=0)
        {
            result=reg[inst.rs1]+reg[inst.rs2];
        }
        else if(inst.op=="ADDI" && inst.rd!=0)
        {
            result=reg[inst.rs1]+inst.imm;
        }else if(inst.op=="MUL" && inst.rd!=0)
        {
            result=reg[inst.rs1]*reg[inst.rs2];
        }else if(inst.op=="NAND" && inst.rd!=0){
            result=~(reg[inst.rs1]&reg[inst.rs2]);
        }
        return result;
    }
    void Update_Rs(std::string name, int16_t result)
    {
        for (auto& rs_entry : dest_register) {
            if (rs_entry.index != 0 && rs_entry.Qi == name) {
                reg[rs_entry.index] = result;
                rs_entry.Qi = "";
                rs_entry.index=-1;
            }
        }
        // Pass the result to other reservation stations
        for (auto& rs : RS) {
            for (auto& station : rs.second) {
                if (station.Qj == name) {
                    station.Vj = result;
                    station.Qj = "";
                }
                if (station.Qk == name) {
                    station.Vk = result;
                    station.Qk = "";
                }
            }
        }
    }
    
    
    void Write_back(const Instruction &inst, int cycles, Memory &mem)
    {
        bool CDP=true;//this means that cdp is free for writing
        if(inst.op=="NAND"||inst.op=="ADD"||inst.op=="ADDI"||inst.op=="MUL"){
            const std::string& rs_name = op_to_RS[inst.op];
            for (auto &station : RS[rs_name]) {
                if(CDP==1 && station.finished_exec==1){
                    if(station.RS_index==dest_register[inst.rd].index){
                        
                        station.busy=0;
                        station.finished_wb=1;
                        station.writeBackCycle=cycles;
                        int16_t res=FP_operation(inst);
                        Update_Rs(rs_name, res);
                        CDP=0;
                    }
                }
                if (station.finished_wb) {
                                    std::cout << "Instruction: " << inst.op << " (index " << inst.inst_index << ") "
                                              << "Issue: " << station.issueCycle << ", "
                                              << "Start Exec: " << station.startExecCycle << ", "
                                              << "End Exec: " << station.endExecCycle << ", "
                                              << "Write-back: " << station.writeBackCycle << std::endl;
                                }
                
            }
        }
        
                        
    }
        
//        if(inst.op=="Load"){
//            
//            
//        }
//        if(inst.op=="Store"){
//            
//        }
 //   }
    
    
};

//Read Instruction
std::vector<Instruction> readInstructionsFromInput() {
    std::vector<Instruction> instructions;
    std::string line;
    
    std::cout << "Enter instructions (one per line, press Enter after each line, enter -1 to finish):\n";
    
    while (std::getline(std::cin, line)) {
        if (line == "-1") {
            break;  // Stop taking instructions if the user enters -1
        }
        
        std::istringstream iss(line);
        Instruction instr;
        
        // Parse operation
        if (!(iss >> instr.op)) {
            std::cerr << "Error: Invalid input format" << std::endl;
            continue;
        }
        
        // Parse other fields based on operation type
        if (instr.op == "ADD" || instr.op == "MUL" || instr.op == "NAND") {
            std::string rs1_str, rs2_str, rd_str;
            if (!(iss >> rs1_str >> rs2_str >> rd_str)) {
                std::cerr << "Error: Invalid input format" << std::endl;
                continue;
            }
            instr.rs1 = std::stoi(rs1_str);
            instr.rs2 = std::stoi(rs2_str);
            instr.rd = std::stoi(rd_str);
        } else if (instr.op == "ADDI") {
            std::string rs1_str, imm_str, rd_str;
            if (!(iss >> rs1_str >> rd_str  >> imm_str)) {
                std::cerr << "Error: Invalid input format" << std::endl;
                continue;
            }
            instr.rs1 = std::stoi(rs1_str);
            instr.rd = std::stoi(rd_str);
            instr.imm = std::stoi(imm_str);
        } else if (instr.op == "Load" ) {
            std::string rd_str, rs_str, imm_str;
            if (!(iss >> rd_str >>rs_str >> imm_str)) {
                std::cerr << "Error: Invalid input format" << std::endl;
                continue;
            }
            instr.rd = std::stoi(rd_str);
            instr.rs1 = std::stoi(rs_str);
            instr.imm = std::stoi(imm_str);
        } else if(instr.op == "Store"||instr.op=="BEQ"){
            std::string rs1_str, rs2_str, imm_str;
            if (!(iss >> rs1_str >>rs2_str >> imm_str)) {
                std::cerr << "Error: Invalid input format" << std::endl;
                continue;
            }
            instr.rs1 = std::stoi(rs1_str);
            instr.rs2 = std::stoi(rs2_str);
            instr.imm = std::stoi(imm_str);
        }
        else {
            std::cerr << "Error: Unknown operation '" << instr.op << "'" << std::endl;
            continue;
        }

        instructions.push_back(instr);
    }

    return instructions;
}
void testExecuteFunction() {
    Tomasulo_Algorithm simulator;
    //cin
    simulator.Initialize_Reservation_station(4, 1, 1, 1, 2, 2, 1);
    simulator.Initialize_numberOfCycles(); // Initialize the cycles and any other required state
    
    bool issued;
    std::vector<Instruction> instructions = readInstructionsFromInput();
    int currCycle = 1;
    for (const auto& inst : instructions) {
        issued = simulator.issue(inst, currCycle);
        if (issued) {
            std::cout << "Issued " << inst.op << std::endl;
            std::cout << "Reservation Stations State:\n";
            std::cout << "RS Type | Busy | Op | Vj | Vk | Qj | Qk | A| Index | Issue Cycle\n";
            for (const auto& type : simulator.RS) {
                for (const auto& station : type.second) {
                    std::cout << std::setw(8) << type.first << " | "
                    << station.busy << " | "
                    << std::setw(3) << station.op << " | "
                    << std::setw(2) << station.Vj << " | "
                    << std::setw(2) << station.Vk << " | "
                    << std::setw(2) << station.Qj << " | "
                    << std::setw(2) << station.Qk << " | "
                    << std::setw(3) << station.A << " | "
        
                    << station.issueCycle << '\n';
                }
            }
            
            // Execute instructions for the current cycle
            //            for (const auto& type : simulator.RS) {
            //                for (auto& station : type.second) {
            //                    simulator.Execute(inst, currCycle);
        }
        currCycle++;
    }
}

            // Print updated state after execution
//            std::cout << "Reservation Stations State after execution:\n";
//            std::cout << "RS Type | Busy | Op | Issue Cycle | Start Exec Cycle | End Exec Cycle\n";
//            for (const auto& type : simulator.RS) {
//                for (const auto& station : type.second) {
//                    std::cout << std::setw(8) << type.first << " | "
//                              << station.busy << " | "
//                              << std::setw(3) << station.op << " | "
//                              << station.issueCycle << " | "
//                              << station.startExecCycle << " | "
//                              << station.endExecCycle << '\n';
//                }
//            }
//        }
//        currCycle++;
//    }
//}

void processCycles(Tomasulo_Algorithm &simulator, std::vector<Instruction> &instructions) {
    Memory memory;
    int cycle = 1;
    int i = 0;
    int counter = 1;
    bool issue_flag = false;

    while (counter <= instructions.size()) {
        if (i >= instructions.size()) {
            simulator.Execute(instructions[i - 1], cycle);
            simulator.Write_back(instructions[i - 1], cycle, memory);
            ++cycle;
        } else {
            issue_flag = simulator.issue(instructions[i], cycle);
            simulator.Execute(instructions[i - 1], cycle);
            simulator.Write_back(instructions[i - 1], cycle, memory);
            ++cycle;
            i = (issue_flag) ? i : i - 1;
            ++i;
        }
        ++counter;
    }
    --cycle;
    std::cout << "Total cycles: " << cycle << std::endl;
}

int main() {
//    Tomasulo_Algorithm simulator;
//    simulator.Initialize_Reservation_station(4, 1, 1, 1, 2, 2, 1);
//    simulator.Initialize_numberOfCycles();
//
//    // Placeholder for reading instructions from input
//    std::vector<Instruction> instructions = readInstructionsFromInput();
//
//    processCycles(simulator, instructions);
    testExecuteFunction();
    return 0;
}
