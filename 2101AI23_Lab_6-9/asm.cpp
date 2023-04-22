/*-------------------------------------------------------------------------------
Name: Pragya Harsh
Roll Number: 2101AI23

Subject: Computer Architecture Lab
Subject Code: CS210
Labs: 6-9
Title: Source Code for Assembler (Language Used: C++)
File Name: asm.cpp

Declaration of Authorship
This asm.cpp file, is part of the lab work (assignment) of CS210 at the
Computer Science and Engineering dept., IIT Patna.
-------------------------------------------------------------------------------*/

#include <bits/stdc++.h>
using namespace std;

int pos;
int curr_line = 0;
int lit_counter = 10000;
int PC = 0; // Program Counter

// defining a struct that would separate the code as follows
struct listing_of_code
{
    string label = "";
    string mnemonic = "";
    string operand = "";
    int pc;
};

// mnemonics table
/*{"ldc":0, "adc":1, "ldl":2, "stl":3, "ldnl":4,
    "stnl":5, "add":6, "sub":7, "shl":8, "shr":9,
    "adj":10, "a2sp":11 ,"sp2a":12 ,"call":13 ,"return":14
    "brz":15 ,"brlz":16 ,"br":17 ,"HALT":18 ,"data":19 ,"SET":20},*/
vector<string> mnemonic{"ldc", "adc", "ldl", "stl", "ldnl",
                        "stnl", "add", "sub", "shl", "shr",
                        "adj", "a2sp", "sp2a", "call", "return",
                        "brz", "brlz", "br", "HALT", "data", "SET"};

// symbol Table
unordered_map<string, int> sym_tab;
// Literal Table (0: value, 1: address)
vector<pair<string, int>> lit_tab;

// to check for labels
bool is_label(string label)
{
    if (label[label.length() - 1] == ':' && ((label[0] <= 'z' && label[0] >= 'a') || (label[0] <= 'Z' && label[0] >= 'A')))
        return true;
    return false;
}

// to check for functions with zero operands
bool zero_op(string temp)
{
    vector<string> zmnemonic{"add", "sub", "shl", "shr", "a2sp", "sp2a", "return", "HALT"};
    if (find(zmnemonic.begin(), zmnemonic.end(), temp) != zmnemonic.end())
        return true;
    return false;
}

// to check decimal
bool is_decimal(string temp)
{
    if (temp[0] == '-' || temp[0] == '+')
    {
        temp = temp.substr(1, temp.length());
    }

    if (temp[0] == '0' && temp.length() == 1)
    {
        return true;
    }
    else if (temp[0] == '0')
    {
        return false;
    }

    for (int i = 0; i < temp.length(); i++)
    {
        if (!isdigit(temp[i]))
        {
            return false;
        }
    }
    return true;
}

// to check octal
bool is_octal(string temp)
{
    if (temp[0] == '0')
    {
        string octt = "01234567";
        for (int i = 1; i < temp.length(); i++)
        {
            if (octt.find(temp[i]) == string::npos)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// to check for number
bool is_number(string temp)
{
    // check for 0
    if (temp == "0")
    {
        return true;
    }

    // check for hexadecimal
    if (temp[0] == '0' && temp[1] == 'x')
    {
        string hexx = "abcdefABCDEF0123456789";
        for (int i = 2; i < temp.length(); i++)
        {
            if (hexx.find(temp[i]) == string::npos)
            {
                return false;
            }
        }
        return true;
    }

    // check for octal
    if (temp[0] == '0')
    {
        string octt = "01234567";
        for (int i = 1; i < temp.length(); i++)
        {
            if (octt.find(temp[i]) == string::npos)
            {
                return false;
            }
        }
        return true;
    }

    // check for decimal
    for (int i = 0; i < temp.length(); i++)
    {
        if (!isdigit(temp[i]))
        {
            return false;
        }
    }
    return true;
}

int first_pass(ifstream &asm_file, vector<string> &codelist, vector<string> &errors, vector<string> &warnings, vector<listing_of_code> &sep_code, unordered_map<string, int> &data)
{
    // if the file is non-empty
    if (asm_file.is_open())
    {
        string line;
        // used to extract lines from the file
        while (getline(asm_file, line))
        {
            line = line.substr(0, line.find(";", 0)); // to remove comments, if any

            // a structure that would break the code into parts
            struct listing_of_code code_line;
            pos = 0;
            code_line.pc = PC;

            // separating the code line
            stringstream s(line);
            string temp, temp1;
            s >> temp;

            // to take care of a case of label
            int check = 0;
            if (temp.size() > 0)
            {
                for (int i = 0; i < temp.size() - 1; i++)
                {
                    if (temp[i] == ':')
                    {
                        temp1 = temp.substr(i + 1, temp.size() - 1);
                        temp = temp.substr(0, i + 1);
                        check = 1;
                        break;
                    }
                }
            }
            if (check == 0)
            {
                s >> temp1;
            }

            // checking if the line is empty
            if (temp == "")
            {
                curr_line++;
                continue;
            }
            // checking if it is a label
            else if (is_label(temp))
            {
                code_line.label = temp;
                // checking if the label already exists
                // if it doesn't exist, adding it to the symbol table
                if (sym_tab.find(temp) == sym_tab.end())
                {
                    sym_tab[temp] = PC;
                }
                // if it exists in the symbol table
                else
                {
                    // checking if an unassigned label is in the table
                    if (sym_tab[temp] == -1)
                    {
                        sym_tab[temp] = PC;
                    }
                    // otherwise raising error
                    else
                    {
                        errors.push_back("Error: Repetition of Label at line " + to_string(curr_line));
                    }
                }

                // checking if second string is empty
                if (temp1.length() == 0)
                {
                    // issuing a warning for empty label
                    warnings.push_back("Warning: Unused label at line " + to_string(curr_line));
                    sep_code.push_back(code_line); // adding it to the list of codes
                    curr_line++;
                    continue; // continue with next line as we are done with this one
                }
                // checking if second string is a mnemonic
                else if (find(mnemonic.begin(), mnemonic.end(), temp1) != mnemonic.end())
                {
                    code_line.mnemonic = temp1;
                    string temp2, temp3;
                    s >> temp2;
                    s >> temp3;
                    // to check if there are more than one operans after mnemonic
                    if (temp3.length() > 0)
                    {
                        errors.push_back("Error: Extra operand at line " + to_string(curr_line));
                        curr_line++;
                        PC++;
                        continue; // continue with next line as we are done with this one
                    }
                    else
                    {
                        // checking if there should be 0 operands after mnemonic or not
                        if (zero_op(temp1))
                        {
                            if (temp2.length() > 0)
                            {
                                errors.push_back("Error: Extra operand at line " + to_string(curr_line));
                                curr_line++;
                                PC++;
                                continue; // continue with next line as we are done with this one
                            }
                        }
                        // if there should be 1 operand
                        else
                        {
                            code_line.operand = temp2;
                            // checking if operand is empty
                            if (temp2.length() == 0)
                            {
                                errors.push_back("Error: Missing operand at line " + to_string(curr_line));
                            }
                            // checking if label is declared after mnemonic :
                            else if (is_label(temp2))
                            {
                                errors.push_back("Error: Label declared after mnemonic at line " + to_string(curr_line));
                            }
                            // checking if there is a refrence to a label
                            else if ((temp2[0] <= 'z' && temp2[0] >= 'a') || (temp2[0] <= 'Z' && temp2[0] >= 'A'))
                            {
                                // incase it is not already in table
                                if (sym_tab.find(temp2 + ":") == sym_tab.end())
                                {
                                    sym_tab[temp2 + ":"] = -1;
                                }
                                // incase it is in table
                                else
                                {
                                    sep_code.push_back(code_line);
                                    curr_line++;
                                    PC++;
                                    continue; // continue with next line as we are done with this one
                                }
                            }
                            // literal table
                            else if (is_number(temp2) || ((temp2[0] == '-' || temp2[0] == '+') && is_number(temp2.substr(1, temp2.length()))))
                            {
                                lit_tab.push_back({temp2, lit_counter++}); // for number
                            }
                            // else raising an error
                            else
                            {
                                errors.push_back("Error: Wrong Syntax found at line " + to_string(curr_line));
                            }
                        }
                    }
                }
                // else raising an error
                else
                {
                    errors.push_back("Error: Wrong Syntax found at line " + to_string(curr_line));
                }
            }
            // checking if it's a mnemonic
            else if (find(mnemonic.begin(), mnemonic.end(), temp) != mnemonic.end())
            {
                code_line.mnemonic = temp;
                string temp2;
                s >> temp2;
                // to check if there are more than one operands after mnemonic
                if (temp2.length() > 0)
                {
                    errors.push_back("Error: Extra operand at line " + to_string(curr_line));
                    curr_line++;
                    PC++;
                    continue; // continue with next line as we are done with this one
                }
                else
                {
                    // checking if there should be 0 operands after mnemonic or not
                    if (zero_op(temp))
                    {
                        if (temp1.length() > 0)
                        {
                            errors.push_back("Error: Extra operand at line " + to_string(curr_line));
                            curr_line++;
                            PC++;
                            continue; // continue with next line as we are done with this one
                        }
                    }
                    // if there should be 1 operand
                    else
                    {
                        code_line.operand = temp1;
                        // checking if operand is empty
                        if (temp1.length() == 0)
                        {
                            errors.push_back("Error: Missing operand at line " + to_string(curr_line));
                        }
                        // checking if label is declared after mnemonic :
                        else if (is_label(temp1))
                        {
                            errors.push_back("Error: Label declared after mnemonic at line " + to_string(curr_line));
                        }
                        // checking if there is a refrence to a label
                        else if ((temp1[0] <= 'z' && temp1[0] >= 'a') || (temp1[0] <= 'Z' && temp1[0] >= 'A'))
                        {
                            // incase it is not already in table
                            if (sym_tab.find(temp1 + ":") == sym_tab.end())
                            {
                                sym_tab[temp1 + ":"] = -1;
                            }
                            // incase it is in table
                            else
                            {
                                sep_code.push_back(code_line);
                                curr_line++;
                                PC++;
                                continue; // continue with next line as we are done with this one
                            }
                        }
                        // literal table
                        else if (is_number(temp1) || ((temp1[0] == '-' || temp1[0] == '+') && is_number(temp1.substr(1, temp1.length()))))
                        {
                            lit_tab.push_back({temp1, lit_counter++}); // for number
                        }
                        // else raising an error
                        else
                        {
                            errors.push_back("Error: Wrong Syntax found at line " + to_string(curr_line));
                        }
                    }
                }
            }
            // otherwise
            else
            {
                // wrong declaration of label
                if (temp[temp.length() - 1] == ':')
                {
                    errors.push_back("Error: Wrong format for declaring label at line " + to_string(curr_line));
                }
                // wrong syntax
                else
                {
                    errors.push_back("Error: Wrong Syntax found at line " + to_string(curr_line));
                }
            }
            // adding the line to the list
            sep_code.push_back(code_line);
            curr_line++;
            PC++;
        }
    }
    else
    {
        // raise an error in case the file is empty
        errors.push_back("Error: File is Empty");
        return 0;
    }

    // checking for undeclared labels
    for (auto item : sym_tab)
    {
        if (item.second == -1)
        {
            errors.push_back("Error: Undeclared Label " + item.first.substr(0, item.first.length() - 1));
        }
    }

    // for data and set
    for (auto item : sep_code)
    {
        if (item.mnemonic == "data" && item.label != "")
        {
            data[item.label] = 1;
        }
        else if (item.mnemonic == "SET" && item.label != "")
        {
            data[item.label] = 2;
        }
    }
    // first pass completed
    return 0;
}

// second pass
int second_pass(vector<listing_of_code> &sep_code, vector<string> &listing, unordered_map<string, int> &data) // ofstream &log_file,ofstream &object_file,ofstream &listing_file)
{
    // to read all the lines of the code
    for (auto item : sep_code)
    {
        // string to be written in listing file
        string list_item;

        // in order to add PC to the listing file
        stringstream temp;

        // since we need it in hexadecimal
        temp << hex << item.pc;
        string final(temp.str());

        // to write it in 8 bits
        for (int i = 0; i < 8 - final.length(); i++)
        {
            list_item += "0";
        }
        list_item += final + "\t";

        // check if it is a mnemonic or not
        if (item.mnemonic != "")
        {
            // in order to add mnemonic in hex form
            stringstream ss;
            ss << hex << find(mnemonic.begin(), mnemonic.end(), item.mnemonic) - mnemonic.begin();
            string res(ss.str());

            // incase its a zero operand mnemonic
            if (zero_op(item.mnemonic))
            {
                for (int i = 0; i < 8 - res.length(); i++)
                {
                    list_item += "0";
                }
                list_item += res + "\t";
            }
            // if it is a one operand mnemonnic
            else
            {
                // if it is a number
                if (is_number(item.operand) || is_number(item.operand.substr(1, item.operand.length())))
                {

                    // decimal number
                    if (is_decimal(item.operand))
                    {
                        // converting string to integer
                        int dec = stoi(item.operand);
                        // to convert int to hex
                        stringstream ssd;
                        ssd << hex << dec;
                        string resd(ssd.str());

                        // if mnemonic is data/set as they don't have opcode
                        if (item.mnemonic == "data" || item.mnemonic == "SET")
                        {
                            for (int i = 0; i < 8 - resd.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += resd + "\t";
                        }
                        else
                        {
                            // in case it is a negative number
                            if (dec < 0)
                            {
                                resd = resd.substr(2, resd.length());
                            }

                            // adding the hex code for the number
                            for (int i = 0; i < 6 - resd.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += resd;

                            // adding the opcode for the mnemonic as last 8 bits are opcode
                            for (int i = 0; i < 2 - res.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += res + "\t";
                        }
                    }
                    // octal number
                    else if (is_octal(item.operand))
                    {
                        // to store the decimal value
                        int dec;

                        // converting octal to decimal
                        stringstream sso;
                        sso << item.operand;
                        sso >> oct >> dec;

                        // converting decimal to hex
                        stringstream ssd;
                        ssd << hex << dec;
                        string resd(ssd.str());

                        // if mnemonic is data/set as they don't have opcode
                        if (item.mnemonic == "data" || item.mnemonic == "SET")
                        {
                            for (int i = 0; i < 8 - resd.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += resd + "\t";
                        }
                        else
                        {
                            // adding the hex code for the number
                            for (int i = 0; i < 6 - resd.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += resd;

                            // adding the opcode for the mnemonic as last 8 bits are opcode
                            for (int i = 0; i < 2 - res.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += res + "\t";
                        }
                    }
                    // hexadecimal number
                    else
                    {
                        // hexadecimal string
                        string resd = item.operand.substr(2, item.operand.length());

                        // if mnemonic is data/set as they don't have opcode
                        if (item.mnemonic == "data" || item.mnemonic == "SET")
                        {
                            for (int i = 0; i < 8 - resd.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += resd + "\t";
                        }
                        else
                        {
                            // adding the hex code for the number
                            for (int i = 0; i < 6 - resd.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += resd;

                            // adding the opcode for the mnemonic as last 8 bits are opcode
                            for (int i = 0; i < 2 - res.length(); i++)
                            {
                                list_item += "0";
                            }
                            list_item += res + "\t";
                        }
                    }
                }
                // if it is a variable
                else if (data.find(item.operand + ":") != data.end())
                {
                    // writing it in hex
                    stringstream ssv;
                    ssv << hex << sym_tab[item.operand + ":"];
                    string resv(ssv.str());

                    // adding the hex code for the variable
                    for (int i = 0; i < 6 - resv.length(); i++)
                    {
                        list_item += "0";
                    }
                    list_item += resv;

                    // adding the opcode
                    for (int i = 0; i < 2 - res.length(); i++)
                    {
                        list_item += "0";
                    }
                    list_item += res + "\t";
                }
                // if it is a label
                else
                {

                    // if it is a value type mnemonic
                    /*{"ldc":0, "adc":1, "adj":10, "data":19 ,"SET":20},*/
                    string check = item.mnemonic;
                    if (check == "ldc" || check == "adc" || check == "adj" || check == "data" || check == "SET")
                    {
                        // writing it in hex
                        stringstream ssl;
                        ssl << hex << sym_tab[item.operand + ":"];
                        string resl(ssl.str());

                        // adding the hex code for the label
                        for (int i = 0; i < 6 - resl.length(); i++)
                        {
                            list_item += "0";
                        }
                        list_item += resl;

                        // adding the opcode
                        for (int i = 0; i < 2 - res.length(); i++)
                        {
                            list_item += "0";
                        }
                        list_item += res + "\t";
                    }
                    else
                    {
                        // value of label while keeping the program counter in check
                        int val = sym_tab[item.operand + ":"] - item.pc - 1;

                        // converting to hex
                        stringstream ssl;
                        ssl << hex << val;
                        string resl(ssl.str());

                        // incase its a negative number
                        if (val < 0)
                        {
                            resl = resl.substr(2, resl.length());
                        }

                        // adding the hexcode for the label
                        for (int i = 0; i < 6 - resl.length(); i++)
                        {
                            list_item += "0";
                        }
                        list_item += resl;

                        // adding the opcode
                        for (int i = 0; i < 2 - res.length(); i++)
                        {
                            list_item += "0";
                        }
                        list_item += res + "\t";
                    }
                }
            }
        }
        // if there is no mnemonic
        else
        {
            list_item += "        "; // 8 spaces
        }

        // enhancing the listing items
        if (item.label.length() > 0)
        {
            list_item += item.label + "\t";
        }
        if (item.mnemonic.length() > 0)
        {
            list_item += item.mnemonic + "\t";
        }
        if (item.operand.length() > 0)
        {
            list_item += item.operand + "\t";
        }

        // adding it to the list which is to be written in the lisitng file
        listing.push_back(list_item);
    }

    // second pass completed
    return 0;
}

// Main function
int main(int argc, char **argv)
{

    // raising an error in case wrong no. of arguments are given as argc checks number of arguments
    if (!(argc == 2))
    {
        cout << "Invalid Format. Valid format is: ./asm file_name.asm";
        return 0;
    }

    // extracting the file name
    string asm_file_name = string(argv[1]); //.asm
    string file_name = asm_file_name.substr(0, asm_file_name.find(".", 0));

    // opening the file for reading content from it
    ifstream asm_file;
    asm_file.open(asm_file_name, ios::in);

    vector<string> codelist;          // for storing lines of codes
    vector<string> errors;            // for storing errors
    vector<string> warnings;          // for storing warnings
    vector<listing_of_code> sep_code; // stores each type of string seprately
    unordered_map<string, int> data;  // for data

    // First Pass
    first_pass(asm_file, codelist, errors, warnings, sep_code, data);

    // closing the file after first pass
    asm_file.close();

    // list of files
    ofstream log_file, listing_file, object_file;

    // opening log file
    string log_file_name = file_name + ".log"; //.log
    log_file.open(log_file_name, ios::out);

    // writing errors of the first pass
    int err = 1;
    for (auto item : errors)
    {
        log_file << err << ". " << item << endl;
        err++;
    }

    // writing warnings of first pass
    for (auto item : warnings)
    {
        log_file << err << ". " << item << endl;
        err++;
    }

    // in case ther is no error in pass 1
    if (err == 1)
    {
        log_file << "No Error in Pass 1" << endl;
    }

    // if there are errors in the first pass, second can't be performed
    if (errors.size() >= 1)
    {
        // open listing file
        string listing_file_name = file_name + ".lst"; //.lst
        listing_file.open(listing_file_name, ios::out);
        // closing the file
        listing_file.close();

        // open object file in binary mode
        string object_file_name = file_name + ".o"; //.o
        object_file.open(object_file_name, ios::out | ios::binary);
        // closing the file
        object_file.close();
        // closing log file
        log_file.close();

        // returning as second pass can't be performed
        return 0;
    }

    // to keep a list of machine code in binary format
    // vector <string> binary_code;

    // to keep a list of thingd to be printed in listing file
    vector<string> listing;

    // second pass
    second_pass(sep_code, listing, data);

    log_file << "No Error in Pass 2" << endl;

    // closing the log file
    log_file.close();

    // opening listing file
    string listing_file_name = file_name + ".lst"; //.lst
    listing_file.open(listing_file_name, ios::out);

    // opening object file in binary mode
    string object_file_name = file_name + ".o"; //.o
    object_file.open(object_file_name, ios::out | ios::binary);

    for (auto item : listing)
    {
        // writing into lisiting file
        listing_file << item << endl;
        stringstream ss(item);
        string buff, temp;
        ss >> buff;
        ss >> temp;

        // to write in binary format
        if (sym_tab.find(temp) == sym_tab.end())
        {
            unsigned int dec;
            stringstream ssd;
            ssd << hex << temp;
            ssd >> dec;
            static_cast<int>(dec);
            object_file.write((const char *)&dec, sizeof(unsigned));
        }
    }

    // closing listing file
    listing_file.close();
    // closing object file
    object_file.close();

    return 0;
}