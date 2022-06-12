#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

class Macro {
public:
    char name[2048];
    char type[2048];
    int x;
    int y;
    char dir[2048];
    Macro(char _name[2048], char _type[2048], int _x, int _y, char _dir[2048]){
        strcpy(name, _name);
        strcpy(type, _type);
        x = _x;
        y = _y;
        strcpy(dir, _dir);
    }
};

vector<Macro> MacroList;

void myDEFParse(char* F){
    FILE *DEF_file = fopen(F, "r"); 

    const char* comp = "COMPONENTS\0";
    int comp_num;
    char s[2048];
    while(fscanf(DEF_file, "%s", s) != EOF) { 
        if(strcmp(s, comp)==0){
            fscanf(DEF_file, "%s", s);
            comp_num = atoi(s);
            fscanf(DEF_file, "%s", s);
            break;
        }
    }

    for(int i=0;i<comp_num;i++){
        fscanf(DEF_file, "%s", s); // "-"
        fscanf(DEF_file, "%s", s); // "name"
        char name[2048];
        strcpy(name, s);
        fscanf(DEF_file, "%s", s); // "type"
        char type[2048];
        strcpy(type, s);
        fscanf(DEF_file, "%s", s); // "+"
        fscanf(DEF_file, "%s", s); // "PLACED/FIXED"
        fscanf(DEF_file, "%s", s); // "("
        fscanf(DEF_file, "%s", s); // "x"
        int x;
        x = atoi(s);
        fscanf(DEF_file, "%s", s); // "y"
        int y;
        y = atoi(s);
        fscanf(DEF_file, "%s", s); // ")"
        fscanf(DEF_file, "%s", s); // "DIRECTION"
        char dir[2048];
        strcpy(dir, s);
        fscanf(DEF_file, "%s", s); // ";"
        Macro new_macro(name, type, x, y, dir);
        MacroList.push_back(new_macro);
        printf("%s %s %d %d %s\n", new_macro.name, new_macro.type, new_macro.x, new_macro.y, new_macro.dir);
    }
    
    fclose(DEF_file);
};

int main(int argc, char *argv[]) {
    
    // Parse DEF file
    myDEFParse(argv[1]);
    cout << endl << MacroList.size() << endl;
    
    return 0;
}


// strcmp(s, obj)==0