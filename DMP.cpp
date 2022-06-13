#include <iostream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <fstream>
#include <string>
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

void myVParse(char* F)
{   int count = 0;
    int number[175][10000];
    int samenet[50000][3];
    char store_name[1000][100];
    int index = 0;
    int counter = 0;
    char output_name[1000][150];
    int relevance[176][176];
    string line;
    ifstream myVerilog;
    myVerilog.open(F);

    while(getline(myVerilog, line)) 
    {   if(line[0] == 'b')
        {   int q = 0;
            while(line[q] != '(')
            {
                store_name[count][q] = line[q];
                q++;
            }
            int nets = 0;           
            for(int i = 0; i < line.length(); i++)
            {   
                if(line[i] == ' ' && line[i+1] == '.')
                {   
                    nets ++;
                    if(line[i+5] == 'n' && line[i+11] == ')')
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+5] == 'n' && line[i+12] == ')')
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        int num6 = line[i+11];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;
                    }
                    if(line[i+6] == 'n' && line[i+12] == ')')
                    {   int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+6] == 'n' && line[i+13] == ')')
                    {
                        int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        int num6 = line[i+12];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;
                    }
                    if(line[i+7] == 'n' && line[i+13] == ')')
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+7] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        int num6 = line[i+13];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;              
                    }
                }
            }
            number[count][0] = nets;// nets : total number of nets connected to the macro
            count ++; //current number of macros
        }   
    }

    while(index < 175)
    {
        for(int i = 0; i < number[index][0]; i ++)
        {
            for(int j = index + 1; j < 175; j++)
            {
                for(int k = 1; k < number[j][0]; k++)
                {
                    if(number[index][i] == number[j][k] && number[index][i] != 0)
                    {
                        samenet[counter][0] = i;
                        samenet[counter][1] = j;
                        samenet[counter][2] = number[index][i];
                        counter++; 

                    }
                }
            }
        }
        index ++;
    }
    for(int t = 0; t < 1000; t++)
    {
        // cout << samenet[t][0] << " " << samenet[t][1] << " " << samenet[t][2] << endl;
        relevance[samenet[t][0]][samenet[t][1]]+=1;
        relevance[samenet[t][1]][samenet[t][0]]+=1;
        for(int o = 0; o <76; o++)
        {
            output_name[t][o] = store_name[samenet[t][0]][o];
        }
        for(int o = 76; o <150; o++)
        {
            output_name[t][o] = store_name[samenet[t][1]][o-76];
        } 

    }
    // for(int y = 0; y < 176 ; y++)
    // {
    //     if(y < 10)
    //     {
    //         cout << "     " << y;
    //     }
    //     if(10 <=y && y < 100)
    //     {
    //         cout << "    " << y;
    //     }
    //     if(100 <=y && y < 176)
    //     {
    //         cout << "   " << y;
    //     }
        
    // }
    // cout << endl;

    // for(int n = 0; n < 176; n++)
    // {
    //     for(int m = 0; m < 176; m++)
    //     {
    //         if(relevance[n][m] <= 100)
    //         {
    //             cout << "     " << relevance[n][m];
    //         }   
    //     }
    //     cout << endl;
    // }
    myVerilog.close();
}

int main(int argc, char *argv[]) {

    // Parse Verilog file
    myVParse(argv[1]);

    // Parse DEF file
    myDEFParse(argv[4]);
    
    // Pasre LEF file
    myLEFParse(argv[2]);

    // Write the output
    writeDMP(argv[4], argv[6]);

    return 0;
}