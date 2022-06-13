#include <iostream>
#include <cstring>
#include <vector>
#include <iomanip>
using namespace std;

class Pin {
public:
    char pinName[128]; // Pin name
    double pinx;          // Pin location
    double piny;
    Pin(char _name[128], double  _x, double _y){
        strcpy(pinName, _name);
        pinx = _x;
        piny = _y;
    }
};

class Macro {
public:
    char name[128];
    char type[128];
    int x;
    int y;
    double size_x;
    double size_y;
    char dir[128];
    vector<Pin> PinList;

    Macro(char _name[128], char _type[128], int _x, int _y, char _dir[128]){
        strcpy(name, _name);
        strcpy(type, _type);
        x = _x;
        y = _y;
        strcpy(dir, _dir);
    }
};

vector<Macro> MacroList;
int diearea_x;
int diearea_y;

void myDEFParse(char* F){
    FILE *DEF_file = fopen(F, "r"); 

    const char* die = "DIEAREA\0";
    char s[128];
    while(fscanf(DEF_file, "%s", s) != EOF) { 
        if(strcmp(s, die)==0){
            for(int j=0;j<6;j++) fscanf(DEF_file, "%s", s);
            diearea_x = atoi(s);
            fscanf(DEF_file, "%s", s);
            diearea_y = atoi(s);
            break;
        }
    }
    //printf("%d %d\n", diearea_x, diearea_y);

    int comp_num;
    for(int j=0;j<4;j++) fscanf(DEF_file, "%s", s);
    comp_num = atoi(s);
    fscanf(DEF_file, "%s", s);     //這裡要讀到分號
    
    for(int i=0;i<comp_num;i++){
        fscanf(DEF_file, "%s", s); // "-"
        fscanf(DEF_file, "%s", s); // "name"
        char name[128];
        strcpy(name, s);
        fscanf(DEF_file, "%s", s); // "type"
        char type[128];
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
        char dir[128];
        strcpy(dir, s);
        fscanf(DEF_file, "%s", s); // ";"
        Macro new_macro(name, type, x, y, dir);
        MacroList.push_back(new_macro);
        //printf("%s %s %d %d %s\n", new_macro.name, new_macro.type, new_macro.x, new_macro.y, new_macro.dir);
    }
    fclose(DEF_file);
};

void myLEFParse(char* F){
    FILE *LEF_file = fopen(F, "r"); 

    for(int i=0;i<MacroList.size();i++){
        char macroType[128];
        strcpy(macroType, MacroList[i].type);
        char s[128];
        while(fscanf(LEF_file, "%s", s) != EOF) { 
            if(strcmp(s, macroType)==0){
                for(int j=0;j<14;j++) fscanf(LEF_file, "%s", s); 
                double x = atof(s);
                fscanf(LEF_file, "%s", s);  //BY
                fscanf(LEF_file, "%s", s);
                double y = atof(s);
                for(int j=0;j<6;j++) fscanf(LEF_file, "%s", s);
                break;
            }
        }
        while(strcmp(s, "OBS\0")!=0){
            fscanf(LEF_file, "%s", s); // Now s is pin name
            char name[128];
            strcpy(name, s);
            for(int j=0;j<12;j++) fscanf(LEF_file, "%s", s);
            double x1 = atof(s);
            fscanf(LEF_file, "%s", s);
            double y1 = atof(s);
            fscanf(LEF_file, "%s", s);
            double x2 = atof(s);
            fscanf(LEF_file, "%s", s);  // Now the last number
            double y2 = atof(s);
            double x = (x1+x2)/2;
            double y = (y1+y2)/2;
            Pin new_pin(name, x, y);
            MacroList[i].PinList.push_back(new_pin);
            for(int j=0;j<5;j++) fscanf(LEF_file, "%s", s);
        }

        rewind(LEF_file);
    }

    fclose(LEF_file);
};

void writeDMP(char* Fin, char* Fout){
    FILE *DEF_file = fopen(Fin, "r");
    FILE *DMP_file = fopen(Fout, "w");
    char buffer[128];
    for(int i=0; i<4; i++){
        fgets(buffer, 128, DEF_file);
        fprintf(DMP_file, "%s", buffer);
    }
    // First 4 lines
    for(int i=0; i<3; i++){
        fgets(buffer, 128, DEF_file);
    }
    fprintf(DMP_file, "DIEAREA ( 0 0 ) ( %d %d ) ;\n\n", diearea_x, diearea_y);
    fprintf(DMP_file, "%s", buffer);
    for(auto i:MacroList){
        fprintf(DMP_file, "\t- %s %s\n", i.name, i.type); 
        fprintf(DMP_file, "\t\t+ PLACED ( %d %d ) %s ;\n", i.x, i.y, i.dir);
    }
    fprintf(DMP_file, "END COMPONENTS\n\n\n\nEND DESIGN\n\n");
    
    fclose(DEF_file);
    fclose(DMP_file);
};

int main(int argc, char *argv[]) {
    
    // Parse DEF file
    myDEFParse(argv[4]);
    
    // Pasre LEF file
    myLEFParse(argv[2]);

    // Write the output
    writeDMP(argv[4], argv[6]);

    return 0;
}