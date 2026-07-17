#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
using namespace std;
string CodeFilename = "Code";
short BoradX = 24;
short BoradY = 18;
string BianLiangName[1000] = {};
string BianLiangMingstring[1000] = {};
short Borad[24][18] = {0};  // 改回 int32_t
string line;
int Linenumber = 0;
string Code[10000];
string colorPart = "";
int currentLine = 0;
string outputCommand = "";

int main(int argc, char* argv[]){
    if (argc > 1){
        string arg = argv[1];
        if (arg.size() >= 3 && arg.substr(arg.size()-3, 3) == ".fw"){
            CodeFilename = arg.substr(0, arg.size()-3);
        }
    }
    
    // 打开 .fw 文件
    ifstream read(CodeFilename + ".fw");
    if (!read) {
        cout << "找不到文件，使用默认 Code.fw" << endl;
        read.open("Code.fw");
        if (!read){
            cout << "打不开文件" << endl;
            cin.get();
            return 1;
        }
    }
    
    // 第一次读取，数行数
    while (getline(read, line)){
    	Linenumber++;
	}
    read.close();
    cout << "正在编译..." << endl;
    
    // 第二次读取，存代码并去除空格
	read.open(CodeFilename + ".fw");
	while (getline(read, line)){
    	string clean = "";
    	for (char c : line){
        	if (c != ' '){
        		clean += c;
			}
    	}
    	line = clean;
    	Code[currentLine++] = line;
	}
	read.close();
    
    if (Linenumber < 1){
        cerr << "错误！文件为空" << endl;
        cin.get();
        return 1;
    }
    // 处理第一行初始化
    string firstLine = Code[0];
    if (!firstLine.empty() && firstLine.back() == ';') {
        firstLine = firstLine.substr(0, firstLine.size()-1);
    }
    while (!firstLine.empty() && firstLine[0] == ' ') firstLine.erase(0, 1);
    
    int fillValue = -1;
    if (firstLine == "Borad(0)" || firstLine == "borad(0)") fillValue = 0;
    else if (firstLine == "Borad(1)" || firstLine == "borad(1)") fillValue = 1;
    
    if (fillValue == -1) {
        cerr << "错误！第一行必须是 Borad(0); 或 Borad(1);" << endl;
        cin.get();
        return 1;
    }
    
    // 初始化棋盘
    for (int i = 0; i < 24; i++)
        for (int j = 0; j < 18; j++)
            Borad[i][j] = fillValue;
    
    // 分号检查
    for (int i = 1; i < Linenumber; i++) {
        string l = Code[i];
        if (l.empty() || l.find("//") == 0){
        	continue;
		}
        if (l.back() != ';') {
            cerr << "错误！第" << i+1 << "行末尾需加分号" << endl;
            cin.get();
            return 1;
        }
    }
    
    // 解析画点指令
    for (int i = 1; i < Linenumber; i++){
        string l = Code[i];
        if (l.empty() || l.find("//") == 0) continue;
        if (l.back() == ';') l = l.substr(0, l.size()-1);
        
        //检测是否是变量语句
		if(l.find("All") == 0 || l.find("all") == 0){
    		size_t equalPos = l.find('=');
    		if(equalPos != string::npos){
        		// 格式: all a = 1; 或 all (a) = 1;
        		string varPart = l.substr(3, equalPos - 3); // "all"后面到等号
        		string varValue = l.substr(equalPos + 1);
        		// 去掉空格和括号
        		string varName = "";
        		for(char c : varPart){
            		if(c != ' ' && c != '(' && c != ')')
                		varName += c;
        		}
        		// 去掉值后面的分号
        		if(!varValue.empty() && varValue.back() == ';'){
        			varValue = varValue.substr(0, varValue.size()-1);
				}
        		for(int j = 0; j < 1000; j++){
            		if(BianLiangName[j] == ""){
                		BianLiangName[j] = varName;
                		BianLiangMingstring[j] = varValue;
                		break;
            		}
        		}
    		}
    		continue;
		}
        
        // 判断是否是输出语句 - 使用 output 避免冲突
        if(l.find("output") == 0){
    		size_t startQuote = l.find('"');
    		size_t endQuote = l.rfind('"');
    		if(startQuote != string::npos && endQuote != string::npos && endQuote > startQuote){
        		string content = l.substr(startQuote + 1, endQuote - startQuote - 1);
        		outputCommand += "    cout << \"" + content + "\";\n";
    		}
    		continue;
		}
        
        colorPart = "";
        int pos = -1;
        for (int p = 0; p < l.size(); p++){
            if (l[p] == '<' && p+1 < l.size() && l[p+1] == '<'){
                pos = p;
                break;
            }
        }
        
        if (pos == -1){
        	continue;
		}
        
        // 取颜色
        for (int p = 0; p < pos; p++)
            if (l[p] != ' ') colorPart += l[p];
        
        // 取坐标
        string coordPart = "";
        for (int p = pos+2; p < l.size(); p++)
            if (l[p] != ' ' && l[p] != '(' && l[p] != ')')
                coordPart += l[p];
        
        int commaPos = -1;
        for (int p = 0; p < coordPart.size(); p++)
            if (coordPart[p] == ',') { commaPos = p; break; }
        
        if (commaPos == -1 || colorPart.empty()) continue;
        
        string xStr = coordPart.substr(0, commaPos);
        string yStr = coordPart.substr(commaPos + 1);
        
        // 变量替换
        for(int v = 0; v < 1000; v++){
            if(!BianLiangName[v].empty() && BianLiangName[v] == xStr){
                xStr = BianLiangMingstring[v];
                break;
            }
        }
        for(int v = 0; v < 1000; v++){
            if(!BianLiangName[v].empty() && BianLiangName[v] == yStr){
                yStr = BianLiangMingstring[v];
                break;
            }
        }
        
        int color = 0, x = 0, y = 0;
        for (char c : colorPart) color = color * 10 + (c - '0');
        for (char c : xStr) x = x * 10 + (c - '0');
        for (char c : yStr) y = y * 10 + (c - '0');
        
        if (y < 0 || y >= 24 || x < 0 || x >= 18) {
            cerr << "错误！第" << i+1 << "行坐标越界: (" << y << "," << x << ")" << endl;
            cin.get();
            return 1;
        }
        
        if (color == 0 || color == 1) {
            Borad[y][x] = color;
        } else {
            cerr << "错误！第" << i+1 << "行颜色值必须是0或1" << endl;
            cin.get();
            return 1;
        }
    }
    
    // 写 .fk 文件
    ofstream fkFile("runs/cache/" + CodeFilename + ".fk");
    for (int i = 0; i < 24; i++){
        for (int j = 0; j < 18; j++){
            fkFile << Borad[i][j];
        }
        fkFile << endl;
    }
    fkFile.close();
    //显示棋盘
    
	for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 18; j++) {
            cout << (Borad[i][j] == 1 ? "□" : "■");
        }
        cout << "\n";
    }

    cin.get();
    return 0;
}