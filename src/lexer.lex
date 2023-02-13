%option noyywrap

%{
#include "parser.hh"
#include <string>
#include <map>
using namespace std;
extern int yyerror(string msg);
// map<string, string> macros;
%}

/* #def[ ]+[a-zA-Z]+[ ]+[^\n]+ { 
            string key = string(yytext);
            size_t space = key.find(" ");
            key = key.substr(space+1);
            space = key.find(" ");
            string value = key.substr(space+1);
            // value = value.substr(0, value.size()-1);
            key = key.substr(0, space);
            macros[key] = value;
            // printf("key = %s value = %s", key, value);
            cout << "key " << key << " value " << value << endl;
        }
[a-zA-Z]+ {
            if (macros.find(string(yytext)) != macros.end()) {
                yylval.lexeme = macros[string(yytext)];
                cout << "key here " << string(yytext) << " value here " << macros[string(yytext)] << endl;
            } 
            else {
                yylval.lexeme = string(yytext);
            }
                return TIDENT;
            
        } */

%%

"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { return TLPAREN; }
")"       { return TRPAREN; }
"="       { return TEQUAL; }
"dbg"     { return TDBG; }
"let"     { return TLET; }
[0-9]+    { yylval.lexeme = string(yytext); return TINT_LIT; }
                
[a-zA-Z]+ { yylval.lexeme = string(yytext); return TIDENT; }

[ \t\n]   { /* skip */ }
"//".*    { }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { }
[/][*]    {yy_fatal_error("Error : Comment not terminated.");}
[*][/]    {yy_fatal_error("Error : Termination of unopened comment.");}
.         { yyerror("Unknown character."); }

%%

string token_to_string(int token, const char *lexeme) {
    string s;
    switch (token) {
        case TPLUS: s = "TPLUS"; break;
        case TDASH: s = "TDASH"; break;
        case TSTAR: s = "TSTAR"; break;
        case TSLASH: s = "TSLASH"; break;
        case TSCOL: s = "TSCOL"; break;
        case TLPAREN: s = "TLPAREN"; break;
        case TRPAREN: s = "TRPAREN"; break;
        case TEQUAL: s = "TEQUAL"; break;
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;
        
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;
    }

    return s;
}