#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cerrno>   // for errno in strtol
#include <climits>  // for INT_MAX

std::string CodeFilename = "Code";
short BoradX = 24;
short BoradY = 18;
std::string BianLiangName[1000] = {};
std::string BianLiangMingstring[1000] = {};
short Borad[24][18] = {0};
std::string line;
int Linenumber = 0;
const int MAX_LINES = 10000;          // 与数组大小一致
std::string Code[MAX_LINES];
std::string colorPart = "";
int currentLine = 0;
std::string outputCommand = "";

// 安全字符串转整数（不添加新功能，仅用标准库函数替换手写循环）
bool safeStoi(const std::string& s, int& out) {
    if (s.empty()) return false;
    // 只允许纯数字（与原有行为一致）
    for (char c : s) {
        if (c < '0' || c > '9') return false;
    }
    // 使用 strtol 检查溢出
    const char* str = s.c_str();
    char* endptr;
    errno = 0;
    long val = std::strtol(str, &endptr, 10);
    if (errno == ERANGE || val > INT_MAX || val < 0) return false; // 坐标不可能为负
    if (endptr != str + s.length()) return false; // 未完全转换
    out = static_cast<int>(val);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg.size() >= 3 && arg.substr(arg.size() - 3, 3) == ".fw") {
            CodeFilename = arg.substr(0, arg.size() - 3);
        }
    }

    std::ifstream read(CodeFilename + ".fw");
    if (!read) {
        std::cout << "找不到文件，使用默认 Code.fw" << std::endl;
        read.open("Code.fw");
        if (!read) {
            std::cout << "打不开文件" << std::endl;
            std::cin.get();
            return 1;
        }
    }

    // 一次读取完成统计行数和存储代码，避免重读不一致
    Linenumber = 0;
    while (Linenumber < MAX_LINES && std::getline(read, line)) {
        // 去除空格（与原有行为一致）
        std::string clean = "";
        for (char c : line) {
            if (c != ' ' && c != '\r') clean += c;
	}
        Code[Linenumber] = clean;
        Linenumber++;
    }
    if (Linenumber == MAX_LINES && std::getline(read, line)) {
        std::cerr << "错误！文件行数超过 " << MAX_LINES << " 行限制" << std::endl;
        std::cin.get();
        return 1;
    }
    read.close();

    std::cout << "正在编译..." << std::endl;

    if (Linenumber < 1) {
        std::cerr << "错误！文件为空" << std::endl;
        std::cin.get();
        return 1;
    }

    // 处理第一行初始化
    std::string firstLine = Code[0];
    if (!firstLine.empty() && firstLine.back() == ';') {
        firstLine = firstLine.substr(0, firstLine.size() - 1);
    }
    while (!firstLine.empty() && firstLine[0] == ' ') firstLine.erase(0, 1);

    int fillValue = -1;
    if (firstLine == "Borad(0)" || firstLine == "borad(0)") fillValue = 0;
    else if (firstLine == "Borad(1)" || firstLine == "borad(1)") fillValue = 1;

    if (fillValue == -1) {
        std::cerr << "错误！第一行必须是 Borad(0); 或 Borad(1);" << std::endl;
        std::cin.get();
        return 1;
    }

    for (int i = 0; i < 24; i++)
        for (int j = 0; j < 18; j++)
            Borad[i][j] = fillValue;

    // 分号检查（跳过空行和注释）
    for (int i = 1; i < Linenumber; i++) {
        std::string l = Code[i];
        if (l.empty() || l.find("//") == 0) continue;
        if (l.back() != ';') {
            std::cerr << "错误！第" << i + 1 << "行末尾需加分号" << std::endl;
            std::cin.get();
            return 1;
        }
    }

    // 解析指令
    for (int i = 1; i < Linenumber; i++) {
        std::string l = Code[i];
        if (l.empty() || l.find("//") == 0) continue;
        if (l.back() == ';') l = l.substr(0, l.size() - 1);

        // 变量定义
        if (l.find("All") == 0 || l.find("all") == 0) {
            std::size_t equalPos = l.find('=');
            if (equalPos != std::string::npos) {
                std::string varPart = l.substr(3, equalPos - 3);
                std::string varValue = l.substr(equalPos + 1);
                std::string varName = "";
                for (char c : varPart) {
                    if (c != ' ' && c != '(' && c != ')')
                        varName += c;
                }
                if (!varValue.empty() && varValue.back() == ';') {
                    varValue = varValue.substr(0, varValue.size() - 1);
                }
                // 存入变量表（检查是否已满）
                bool stored = false;
                for (int j = 0; j < 1000; j++) {
                    if (BianLiangName[j] == "") {
                        BianLiangName[j] = varName;
                        BianLiangMingstring[j] = varValue;
                        stored = true;
                        break;
                    }
                }
                if (!stored) {
                    std::cerr << "错误！变量定义超过最大数量 1000" << std::endl;
                    std::cin.get();
                    return 1;
                }
            }
            continue;
        }

        // 输出语句（保留但不使用，维持原有行为）
        if (l.find("output") == 0) {
            std::size_t startQuote = l.find('"');
            std::size_t endQuote = l.rfind('"');
            if (startQuote != std::string::npos && endQuote != std::string::npos && endQuote > startQuote) {
                std::string content = l.substr(startQuote + 1, endQuote - startQuote - 1);
                outputCommand += "    cout << \"" + content + "\";\n";
            }
            continue;
        }

        // 画点指令 <<
        colorPart = "";
        int pos = -1;
        for (int p = 0; p < (int)l.size(); p++) {
            if (l[p] == '<' && p + 1 < (int)l.size() && l[p + 1] == '<') {
                pos = p;
                break;
            }
        }

        if (pos == -1) continue;

        for (int p = 0; p < pos; p++)
            if (l[p] != ' ') colorPart += l[p];

        std::string coordPart = "";
        for (int p = pos + 2; p < (int)l.size(); p++)
            if (l[p] != ' ' && l[p] != '(' && l[p] != ')')
                coordPart += l[p];

        int commaPos = -1;
        for (int p = 0; p < (int)coordPart.size(); p++)
            if (coordPart[p] == ',') { commaPos = p; break; }

        if (commaPos == -1 || colorPart.empty()) continue;

        std::string xStr = coordPart.substr(0, commaPos);
        std::string yStr = coordPart.substr(commaPos + 1);

        // 变量替换（坐标）
        for (int v = 0; v < 1000; v++) {
            if (!BianLiangName[v].empty() && BianLiangName[v] == xStr) {
                xStr = BianLiangMingstring[v];
                break;
            }
        }
        for (int v = 0; v < 1000; v++) {
            if (!BianLiangName[v].empty() && BianLiangName[v] == yStr) {
                yStr = BianLiangMingstring[v];
                break;
            }
        }
        // 颜色部分暂不支持变量（保持原设计）

        int color, x, y;
        if (!safeStoi(colorPart, color) || !safeStoi(xStr, x) || !safeStoi(yStr, y)) {
            std::cerr << "错误！第" << i + 1 << "行颜色或坐标格式无效" << std::endl;
            std::cin.get();
            return 1;
        }

        if (y < 0 || y >= 24 || x < 0 || x >= 18) {
            std::cerr << "错误！第" << i + 1 << "行坐标越界: (" << y << "," << x << ")" << std::endl;
            std::cin.get();
            return 1;
        }

        if (color == 0 || color == 1) {
            Borad[y][x] = color;
        } else {
            std::cerr << "错误！第" << i + 1 << "行颜色值必须是0或1" << std::endl;
            std::cin.get();
            return 1;
        }
    }

    // 写入棋盘文件
    std::ofstream fkFile("runs/cache/" + CodeFilename + ".fk");
    if (!fkFile) {
        std::cerr << "错误！无法创建输出文件 runs/cache/" << CodeFilename << ".fk" << std::endl;
        std::cin.get();
        return 1;
    }

    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 18; j++) {
            fkFile << Borad[i][j];
        }
        fkFile << std::endl;
    }
    fkFile.close();

    // 显示棋盘
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 18; j++) {
            std::cout << (Borad[i][j] == 1 ? "\u25A1" : "\u25A0");
        }
        std::cout << "\n";
    }

    std::cin.get();
    return 0;
}
