#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <bitset>
#include <iomanip>
#include <sstream>
#include <cstring>

using namespace std;

void trim(string &s);
void toMachineCode(const string &str, map<string,int>& table);
void output(int outputBase, const map<string,int>& table);

int main(int argc, char *argv[]){
    if(argc != 3 || (strcmp(argv[2], "2") && strcmp(argv[2], "16"))){       //判斷參數 strcmp() 比較相等回傳0
        cout << "usage: ./term1.exe filename {2,16}" << endl;
        return 0;
    }

    string fileName = argv[1];
    int outputBase = std::stoi(argv[2]);
    //cout << fileName << "  " << outputBase << endl;

    ifstream inputfile(fileName);
    
    if(!inputfile.is_open()){
        cout << "Failed to open the file." << endl;
        return 0;
    }


    string str;
    int pos = 0;


    //開始讀入資料
    while(getline(inputfile, str)){
        //cout << str  << "." << endl;
        map<string,int> table;

        trim(str);      //消除前後空白 or Tab
        if((pos = str.find('#')) != string::npos){  // 過濾comment
            str = str.substr(0, pos);
        }
        if(pos = str.find('.') == 0) continue;      // 跳過directive
        trim(str);      //消除前後空白 or Tab
        if(str == "") continue;     //跳過空行或是單純註解

        //cout << str  << "." << endl;
        toMachineCode(str, table);

        // for(const auto &it : table) {    //look for map
        //     cout << "Map : " << it.first << " " << it.second << endl;
        // }

        output(outputBase, table);

        table.clear();
    }

    inputfile.close();

    return 0;
}


//擷取各個field轉成machine code
void toMachineCode(const string& str, map<string, int>& table) {
    map <string, int> optable = {{"add", 0},{"sub", 0},{"addi", 8},{"slti", 10},{"lw", 36},{"sw",44}};       //opcode's machine code
    map <string, int> functtable = {{"add", 32},{"sub", 34}};                           //funct's machine code
    string opcode, rs, rt, rd, immed, sub;
    int pos = 0;
    

    pos = str.find(' ');
    opcode = str.substr(0, pos);        //opcode
    sub = str.substr(pos + 1);
    trim(sub);

    //cout << opcode << endl;
    table["opcode"] = optable[opcode];      //add to table

    if(opcode == "add" || opcode == "sub"){     //R format
        pos = sub.find(",");
        rd = sub.substr(1, pos - 1);    //rd (result)
        sub = sub.substr(pos + 1);
        trim(sub);

        pos = sub.find(",");
        rs = sub.substr(1, pos);        //rs (source 1)
        sub = sub.substr(pos + 1);
        trim(sub);
        
        rt = sub.substr(1);        //rt (siurce 2)

        //cout << "rd: " << rd << " rs: " << rs << " rt: " << rt << endl;
        table["rd"] = stoi(rd);             //add to table
        table["rs"] = stoi(rs);
        table["rt"] = stoi(rt);
        table["funct"] = functtable[opcode];
        table["shamt"] = 0;


    }else if(opcode == "addi" || opcode == "slti"){     //I format
        pos = sub.find(",");
        rt = sub.substr(1, pos - 1);    //rt (target、result)
        sub = sub.substr(pos + 1);
        trim(sub);

        pos = sub.find(",");
        rs = sub.substr(1, pos);        //rs (source)
        sub = sub.substr(pos + 1);
        trim(sub);
        
        immed = sub;        //immediate

        //cout << "rt: " << rt << " rs: " << rs << " immediate: " << immed << endl;
        table["rs"] = stoi(rs);             //add to table
        table["rt"] = stoi(rt);
        table["immediate"] = stoi(immed);

    }else if(opcode == "lw" || opcode == "sw"){
        pos = sub.find(",");
        rt = sub.substr(1, pos - 1);    //rt (target、result)
        sub = sub.substr(pos + 1);
        trim(sub);

        pos = sub.find("(");
        immed = sub.substr(0, pos);        //rs (source)
        sub = sub.substr(pos + 2);
        trim(sub);
        //cout << "sub " << sub.substr(sub.length()-1)<< endl;
        
        rs = sub.substr(0, sub.length()-1);        //immediate

        //cout << "rt: " << rt << " rs: " << rs << " immediate: " << immed << endl;
        table["rs"] = stoi(rs);             //add to table
        table["rt"] = stoi(rt);
        table["immediate"] = stoi(immed);

    }

    return;
}

//輸出
void output(int outputBase, const map<string,int>& table){
    unsigned int num = 0;
    if(table.at("opcode") == 0){                                                //R format
        num = table.at("opcode");
        //cout << "opcode " << bitset<6>(table.at("opcode")) << endl;
        num <<= 5;
        num += table.at("rs");
        num <<= 5;
        num += table.at("rt");
        num <<= 5;
        num += table.at("rd");
        num <<= 5;
        num += table.at("shamt");
        num <<= 6;
        num += table.at("funct");

        if (outputBase == 2) {
            cout << "R-format: " << bitset<6>(table.at("opcode")) << " " << bitset<5>(table.at("rs")) << " " << bitset<5>(table.at("rt")) << " "
                 << bitset<5>(table.at("rd")) << " " << bitset<5>(table.at("shamt")) << " " << bitset<6>(table.at("funct")) << endl;
        }

        else if(outputBase == 16){
            cout << "R-format: " << setfill('0') << setw(8) << hex << num << endl;
        }

    }else if (table.at("opcode") == 8 || table.at("opcode") == 10 || table.at("opcode") == 36 || table.at("opcode") == 44){             //I format
        num = table.at("opcode");
        num <<= 5;
        num += table.at("rs");
        num <<= 5;
        num += table.at("rt");
        num <<= 16;
        //cout << "Test(空出): " << bitset<32>(num) << endl;
        num += table.at("immediate") & 0x0000FFFF;          //只取最低 16 bits (-32768 ~ 32767)
        //cout << "Test: " << bitset<32>(num) << endl;
        //cout << "Immediate : " << bitset<32>(table.at("immediate")) << endl;

        if (outputBase == 2){
            cout << "I-format: " << bitset<6>(table.at("opcode")) << " " << bitset<5>(table.at("rs")) << " " << bitset<5>(table.at("rt")) << " "
                 << bitset<16>(table.at("immediate")) << endl;
        }
        else if(outputBase == 16){
            cout << "I-format: " << setfill('0') << setw(8) << hex << num << endl;
        }
    }

}

void trim(string &s) {
    if(s.empty())   return;
    s.erase(s.find_last_not_of(" \t") + 1);
    s.erase(0, s.find_first_not_of(" \t"));

    return;
}