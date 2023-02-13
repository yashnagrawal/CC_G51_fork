#include <bits/stdc++.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <iostream>

#include "ast.hh"
#include "llvmcodegen.hh"
#include "parser.hh"

extern FILE* yyin;
extern int yylex();
extern char* yytext;

using namespace std;

NodeStmts* final_values;
map<string, string> macros;

#define ARG_OPTION_L 0
#define ARG_OPTION_P 1
#define ARG_OPTION_S 2
#define ARG_OPTION_O 3
#define ARG_FAIL -1

// map<string, string> macros;

FILE* preprocessor(FILE* input);
bool detect_cycle(const map<string, string>& map, string start);

int parse_arguments(int argc, char* argv[])
{
    if (argc == 3 || argc == 4) {
        if (strlen(argv[2]) == 2 && argv[2][0] == '-') {
            if (argc == 3) {
                switch (argv[2][1]) {
                case 'l':
                    return ARG_OPTION_L;

                case 'p':
                    return ARG_OPTION_P;

                case 's':
                    return ARG_OPTION_S;
                }
            } else if (argv[2][1] == 'o') {
                return ARG_OPTION_O;
            }
        }
    }

    std::cerr << "Usage:\nEach of the following options halts the compilation process at the corresponding stage and prints the intermediate output:\n\n";
    std::cerr << "\t`./bin/base <file_name> -l`, to tokenize the input and print the token stream to stdout\n";
    std::cerr << "\t`./bin/base <file_name> -p`, to parse the input and print the abstract syntax tree (AST) to stdout\n";
    std::cerr << "\t`./bin/base <file_name> -s`, to compile the file to LLVM assembly and print it to stdout\n";
    std::cerr << "\t`./bin/base <file_name> -o <output>`, to compile the file to LLVM bitcode and write to <output>\n";
    return ARG_FAIL;
}

int main(int argc, char* argv[])
{
    int arg_option = parse_arguments(argc, argv);
    if (arg_option == ARG_FAIL) {
        exit(1);
    }

    std::string file_name(argv[1]);
    FILE* source = fopen(argv[1], "r");

    if (!source) {
        std::cerr << "File does not exists.\n";
        exit(1);
    }

    // yyin = source;
    yyin = preprocessor(source);

    if (arg_option == ARG_OPTION_L) {
        extern std::string token_to_string(int token, const char* lexeme);

        while (true) {
            int token = yylex();
            if (token == 0) {
                break;
            }

            std::cout << token_to_string(token, yytext) << "\n";
        }
        fclose(yyin);
        return 0;
    }

    final_values = nullptr;
    yyparse();

    fclose(yyin);

    if (final_values) {
        if (arg_option == ARG_OPTION_P) {
            std::cout << final_values->to_string() << std::endl;
            return 0;
        }

        llvm::LLVMContext context;
        LLVMCompiler compiler(&context, "base");
        compiler.compile(final_values);
        if (arg_option == ARG_OPTION_S) {
            compiler.dump();
        } else {
            compiler.write(std::string(argv[3]));
        }
    } else {
        std::cerr << "empty program";
    }

    return 0;
}

bool detect_cycle(const map<string, string>& map, string start)
{
    unordered_set<string> visited;
    while (map.count(start) > 0) {
        if (visited.count(start) > 0) {
            return true;
        }
        visited.insert(start);
        start = map.at(start);
    }
    return false;
}

string check_macro(string line)
{
    // modify the function such that it replaces the macros until the result does not contain any macro
    // for example, if the macros are:
    // #def A B
    // #def B C
    // then the line "A" should be replaced to "C" and not "B"

    // for (auto macro : macros) {
    //     // line = macro.first + " // " + macro.second;
    //     size_t pos = 0;
    //     while ((pos = line.find(macro.first, pos)) != string::npos) {

    //         if ((pos > 0 && isalpha(line[pos - 1])) || (pos + macro.first.size() < line.size() && isalpha(line[pos + macro.first.size()]))) {
    //             pos += macro.second.length();
    //             continue;
    //         }

    //         line.replace(pos, macro.first.length(), macro.second);
    //         pos += macro.second.length();
    //     }
    // }
    // return line;

    string result = line;
    bool changed = true;

    while (changed) {
        changed = false;
        for (auto macro : macros) {
            size_t pos = 0;
            while ((pos = result.find(macro.first, pos)) != string::npos) {

                if ((pos > 0 && isalpha(result[pos - 1])) || (pos + macro.first.size() < result.size() && isalpha(result[pos + macro.first.size()]))) {
                    pos += macro.second.length();
                    continue;
                }

                result.replace(pos, macro.first.length(), macro.second);
                pos += macro.second.length();
                changed = true;
            }
            cout << " inside " << endl;
        }
        cout << " outside " << endl;
    }

    return result;
}

FILE* preprocessor(FILE* input)
{
    char buffer[1024];
    string output;
    while (fgets(buffer, 1024, input)) {
        string line(buffer);

        // Check if this line is a #def directive
        size_t defPos = line.find("#def");
        size_t undefPos = line.find("#undef");
        if (defPos != string::npos) {
            // Extract the macro name and value

            // format is: #def <name> <value>

            // get the first space encapsulated token after #def
            size_t nameStart = line.find_first_of(" ") + 1;
            size_t nameEnd = line.find_first_of(" ", nameStart);
            string name = line.substr(nameStart, nameEnd - nameStart);

            // get the value (till line end)
            size_t valueStart = nameEnd + 1;
            string value = line.substr(valueStart, line.length() - 1 - valueStart);
            if (value.length() == 0)
                value = "1";
            // Add the macro mapping to the macros table
            macros[name] = value;

            if (detect_cycle(macros, name)) {
                std::cerr << "Error: Macro cycle detected.\n";
                exit(1);
            }

        } else if (undefPos != string::npos) {
            // Extract the macro name and value

            // format is: #undef <name>

            // get the first space encapsulated token after #def
            size_t nameStart = line.find_first_of(" ") + 1;
            size_t nameEnd = line.length() - 1;
            string name = line.substr(nameStart, nameEnd - nameStart);

            // Add the macro mapping to the macros table
            macros.erase(name);
        } else {
            // Replace macros in the output string
            line = check_macro(line);
            // Add the line to the output string
            output += line;
        }
    }
    // Create a temporary file for the output
    FILE* temp = tmpfile();
    fputs(output.c_str(), temp);
    // save this to file called test.txt
    ofstream myfile;
    myfile.open("test.txt");
    myfile << output.c_str();
    myfile.close();

    rewind(temp);

    return temp;
}