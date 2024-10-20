#include <bits/stdc++.h>
using namespace std;

vector<int> GPR(32, 0);     // General Purpose Registers
unordered_map<int, int> DM; // Data Memory

map<string, string> Operation = {
    {"0000", "AND"},
    {"0001", "OR"},
    {"0010", "ADD"},
    {"0110", "SUB"},
    {"1111", "MUL"},
};

string get_type(string &opcode)
{
    if (opcode == "0100011")
    {
        return "S";
    }
    else if (opcode == "1100011")
    {
        return "B";
    }
    else if (opcode == "0110011")
    {
        return "R";
    }
    else if (opcode == "0010011")
    {
        return "I";
    }
    else if (opcode == "0000011")
    {
        return "Ld";
    }
    else if (opcode == "1101111")
    {
        return "U";
    }
    return "";
}
string SignedExtend(string s, int len = 32)
{
    int n = s.length();
    char c = s[0];  // Most significant bit for sign extension
    while (n < len) // Extend to 32 bits or specified length
    {
        s = c + s;
        n++;
    }
    return s;
}

// Convert binary string to decimal (supports negative numbers using 2's complement)
int to_decimal(const string &s)
{
    bitset<32> bits(s);
    return bits.to_ulong() - bits.test(31) * 2 * (1L << 31); // Convert considering signed 2's complement
}

// Immediate Generator
// string ImmGen(string immPart1, string immPart2)
// {
//     string immediate = immPart1 + immPart2; // Combine parts for immediate
//     return SignedExtend(immediate);         // Sign extend to 32 bits
// }

// Instruction Memory Class
class InstructionMemory
{
public:
    map<int, string> IM; // Instruction Memory Map
    int pc;              // Program Counter

    InstructionMemory(vector<string> &machineCode)
    {
        pc = 0;
        for (int i = 0; i < machineCode.size(); i++)
        {
            IM[i * 4] = machineCode[i]; // Store instruction at appropriate memory address
        }
    }

    string instruction(int x, int y)
    {
        if (IM.find(pc) == IM.end())
        {
            return "NOP"; // No operation if no instruction found
        }
        int mini = min(31 - x, 31 - y);
        int maxi = max(31 - x, 31 - y);
        return IM[pc].substr(mini, maxi - mini + 1);
    }

    string instruction(int x)
    {
        if (IM.find(pc) == IM.end())
        {
            return "NOP";
        }
        return string(1, IM[pc][31 - x]); // Return the specific bit as a string
    }
};

string ImmGen(InstructionMemory *IM, string &type)
{
    if (type == "I" || type == "Ld")
    {
        return IM->instruction(31, 20);
    }
    // else if (type == "B" || type == "S")
    // {
    //     return IM->instruction(31, 25) + IM->instruction(11, 7);
    // }
    // else if (type == "J")
    // {
    //     return IM->instruction(31, 12);
    // }
    // return "";

    if (type == "J")
    {
        string imm_for_j = IM->instruction(31) + IM->instruction(19, 12) + IM->instruction(20) + IM->instruction(21, 30);

        return imm_for_j;
    }
    // if(type=="I")
    // {

    // }
    if (type == "B")
    {
        string imm_for_b = IM->instruction(31) + IM->instruction(7) + IM->instruction(25, 30) + IM->instruction(11, 8);

        return imm_for_b;
    }
    return IM->instruction(31, 20);
}

// Control Unit Class
class ControlUnit
{
public:
    string opcode;
    int ALUSrc, Mem2Reg, RegWrite, MemWrite, Branch, jump, ALUOp1, ALUOp0, MemRead;
    string ALUOp;
    ControlUnit()
    {
    }
    ControlUnit(string op, string func3 = "", string func7 = "")
    {
        opcode = op;
        string type = get_type(op);
        ALUSrc = (type == "S" || type == "I" || type == "Ld") ? 1 : 0;
        Mem2Reg = (type == "Ld") ? 1 : 0;
        RegWrite = (type == "R" || type == "I" || type == "Ld") ? 1 : 0;
        MemWrite = (type == "S") ? 1 : 0;
        Branch = (type == "B") ? 1 : 0;
        jump = (type == "U") ? 1 : 0;
        ALUOp1 = (type == "R" || type == "I") ? 1 : 0;
        ALUOp0 = (type == "B") ? 1 : 0;
        MemRead = !MemWrite;
        ALUOp = to_string(ALUOp1) + to_string(ALUOp0);
    }
};

string ALUControl(string ALUOp, string func7, string func3)
{
    // char ins_30 = imm[0];            // Instruction's 30th bit (for R-type operations)
    // string func3 = imm.substr(1, 3); // func3 is 3 bits
    string ALUSelect;

    if (ALUOp == "00")
    {
        return "0010";
    }
    if (ALUOp == "01")
    {
        return "0110";
    }

    if (func7 == "0000000" && func3 == "000")
    {
        return "0010";
    }
    if (func7 == "0100000" && func3 == "000")
    {
        return "0110";
    }
    if (func7 == "0000000" && func3 == "111")
    {
        return "0000";
    }
    if (func7 == "0000000" && func3 == "110")
    {
        return "0001";
    }
    if (func7 == "0000001" && func3 == "000")
    {
        return "1111";
    }

    return ALUSelect;
}

// ALU Class
class ALU
{
public:
    int ALUresult;
    bool ALUZeroFlag;

    ALU(string ALUSelect, int rs1, int rs2)
    {
        string op = Operation[ALUSelect];
        if (op == "ADD")
        {
            ALUresult = rs1 + rs2; // Perform addition
        }
        else if (op == "AND")
        {
            ALUresult = rs1 & rs2;
        }
        else if (op == "OR")
        {
            ALUresult = rs1 | rs2;
        }
        else if (op == "SUB")
        {
            ALUresult = rs1 - rs2;
        }
        else if (op == "MUL")
        {
            cout << "10000000000000000000000000000************************************************************0" << endl;
            ALUresult = rs1 * rs2;
        }

        ALUZeroFlag = (rs1 == rs2); // Set Zero Flag if rs1 == rs2
    }
};

class IFID
{
public:
    string IR;
    int DPC;
};

class IDEX
{
public:
    int DPC, JPC;
    string imm;
    string func3;
    int rs1, rs2;
    string rdl;
    ControlUnit CW;
};

class EXMO
{
public:
    int ALUOut;
    ControlUnit CW;
    string rdl;
};

class MOWB
{
public:
    int LDOut;
    ControlUnit CW;
    int ALUOut;
    string rdl;
};

IFID ifid;
IDEX idex;
EXMO exmo;
MOWB mowb;

// CPU Class to run instructions
class CPU
{
public:
    InstructionMemory *IM;

    CPU(InstructionMemory *instrMem)
    {
        IM = instrMem;
    }

    void run()
    {
        while (IM->pc < IM->IM.size() * 4) // Run while there are instructions
        {
            // fetch
            string instruction = IM->instruction(0, 31);
            ifid.IR = instruction;
            ifid.DPC = IM->pc;
            int npc = IM->pc + 4;

            // decode
            string imm_for_j = IM->instruction(31) + IM->instruction(19, 12) + IM->instruction(20) + IM->instruction(21, 30);
            string sign_extend_j = SignedExtend(imm_for_j);

            idex.JPC = IM->pc + to_decimal(sign_extend_j) << 1;
            string opcode = IM->instruction(6, 0);
            idex.DPC = ifid.DPC;
            string type = get_type(opcode);
            idex.imm = ImmGen(IM, type);
            // IM-> should be changed to idex. will see it later
            idex.func3 = IM->instruction(14, 12);
            string func7 = IM->instruction(25, 31);
            idex.rdl = IM->instruction(11, 7);
            string rsl1 = IM->instruction(19, 15);
            string rsl2 = IM->instruction(24, 20);
            idex.rs1 = GPR[to_decimal(rsl1)];
            ControlUnit ControlWord(opcode, idex.func3);
            idex.CW = ControlWord;

            if (idex.CW.ALUSrc)
            {
                string sign_extend = SignedExtend(idex.imm);
                idex.rs2 = to_decimal(sign_extend);
            }
            else
            {
                idex.rs2 = GPR[to_decimal(rsl2)];
            }

            // exmo
            string ALUSelect = ALUControl(idex.CW.ALUOp, func7, idex.func3);
            ALU alu(ALUSelect, idex.rs1, idex.rs2);
            exmo.ALUOut = alu.ALUresult;
            int ALUZeroFlag = alu.ALUZeroFlag;
            exmo.CW = idex.CW;
            exmo.rdl = idex.rdl;
            int bpc = (to_decimal(SignedExtend(idex.imm)) << 1) + IM->pc;

            if (idex.CW.Branch && ALUZeroFlag)
            {
                IM->pc = bpc;
            }
            else
            {
                IM->pc = npc;
            }
            if (idex.CW.jump)
            {
                IM->pc = idex.JPC;
            }

            // memory operations
            if (exmo.CW.MemWrite)
            {
                DM[exmo.ALUOut] = idex.rs2;
            }

            if (exmo.CW.MemRead)
            {
                mowb.LDOut = DM[exmo.ALUOut];
            }
            mowb.ALUOut = exmo.ALUOut;
            mowb.CW = exmo.CW;
            mowb.rdl = exmo.rdl;

            // write back
            if (mowb.CW.RegWrite)
            {
                if (mowb.CW.Mem2Reg)
                {
                    GPR[to_decimal(mowb.rdl)] = mowb.LDOut;
                }
                else
                {
                    GPR[to_decimal(mowb.rdl)] = mowb.ALUOut;
                }
            }
        }
        // Output register values
        cout << "GPR values:\n";
        for (int i = 0; i < 32; i++)
        {
            cout << "x" << i << ": " << GPR[i] << endl;
        }

        cout << "\n\nData Memory:\n";
        for (auto it = DM.begin(); it != DM.end(); it++)
        {
            cout << it->first << " " << it->second << endl;
        }
    }
};

signed main()
{
#ifndef ONLINE_JUDGE
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
#endif
    // DM[4]=12;
    // GPR[2]=5;

    vector<string> machineCodes;
    string line;
    while (getline(cin, line))
    {
        machineCodes.push_back(line);
    }

    InstructionMemory IM(machineCodes);
    CPU cpu(&IM);
    cpu.run();

    return 0;
}