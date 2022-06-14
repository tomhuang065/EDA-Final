#include <iostream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
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
int UNITS_DISTANCE_MICRONS;

void myDEFParse(char* F){
    FILE *DEF_file = fopen(F, "r");
    char s[128];
    for(int j=0;j<10;j++) fscanf(DEF_file, "%s", s);
    UNITS_DISTANCE_MICRONS = atoi(s);

    const char* die = "DIEAREA\0";
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
        // printf("%s %s %d %d %s\n", new_macro.name, new_macro.type, new_macro.x, new_macro.y, new_macro.dir);
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
                MacroList[i].size_x = x*UNITS_DISTANCE_MICRONS;
                MacroList[i].size_y = y*UNITS_DISTANCE_MICRONS;
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
            Pin new_pin(name, x*UNITS_DISTANCE_MICRONS, y*UNITS_DISTANCE_MICRONS);
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

int count = 0; // the number of macros
int number[256][10000]; //first column : number of nets connected to the macro, after : name of all nets connected to the macro (max : 1024)
int pin[256][10000];
int samenet[50000][3]; //store pairs of macros connected by the same net
char store_name[1000][100]; // store the name of every macros
int index1 = 0; //used when finding same nets
int counter = 0; //count the total number of paired macros
char output_name_1[1000][150]; //gives the names of paired macros
char output_name_2[1000][150]; 
int relevance[256][256]; // the relevance matrix(start from (1,1))

void myVParse(char* F)
{   
    string line;
    ifstream myFile;
    myFile.open(F);

    while(getline(myFile, line)) 
    {   if(line[0] == 'b') //check if it is a block/macro
        {   int q = 0;
            // cout << "true" << endl; 
            while(line[q] != '(')
            {
                store_name[count][q] = line[q]; //if the sentence starts with 'b', store its name
                q++;
            }
            int nets = 0;           
            for(int i = 0; i < line.length(); i++) //go through the sentence and store all the nets
            {   
                if(line[i] == ' ' && line[i+1] == '.') //the feature of a net
                {   
                    nets ++;
                    if(line[i+5] == 'n' && line[i+11] == ')') // ex : ' .o2(n12345)
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                        pin[count][nets] = (line[i+2]-48)*10000 + line[i+3]-48;
                    }
                    if(line[i+5] == 'n' && line[i+12] == ')') // ex : ' .o2(n123456)
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        int num6 = line[i+11];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;
                        pin[count][nets] = (line[i+2]-48)*10000 + line[i+3]-48;
                    }
                    if(line[i+5] == 'n' && line[i+13] == ')') // ex : ' .o2(n1234567)
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        int num6 = line[i+11];
                        int num7 = line[i+12];
                        number[count][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                        pin[count][nets] = (line[i+2]-48)*10000 + line[i+3]-48;
                    }
                    if(line[i+6] == 'n' && line[i+12] == ')') // ex : ' .o12(n12345)
                    {   int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*10 + line[i+4]- 48;
                    }
                    if(line[i+6] == 'n' && line[i+13] == ')')
                    {
                        int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        int num6 = line[i+12];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 + num6-48;
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*10 + line[i+4]- 48;
                    }
                    if(line[i+6] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        int num6 = line[i+12];
                        int num7 = line[i+13];
                        number[count][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*10 + line[i+4]- 48;
                    }
                    if(line[i+7] == 'n' && line[i+13] == ')')// ex : ' .o123(n12345)
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*100 + (line[i+4]- 48)*10 +(line[i+5]-48);
                    }
                    if(line[i+7] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        int num6 = line[i+13];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;  
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*100 + (line[i+4]- 48)*10 +(line[i+5]-48);     
                    }
                    if(line[i+7] == 'n' && line[i+15] == ')')
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        int num6 = line[i+13];
                        int num7 = line[i+14];
                        number[count][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*100 + (line[i+4]- 48)*10 +(line[i+5]-48);
                    }
                    if(line[i+8] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+9];
                        int num2 = line[i+10];
                        int num3 = line[i+11];
                        int num4 = line[i+12];
                        int num5 = line[i+13];
                        number[count][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*1000 + (line[i+4]- 48)*100 + (line[i+5]- 48)*10 + (line[i+6]- 48);
                    }
                    if(line[i+8] == 'n' && line[i+15] == ')')
                    {   int num1 = line[i+9];
                        int num2 = line[i+10];
                        int num3 = line[i+11];
                        int num4 = line[i+12];
                        int num5 = line[i+13];
                        int num6 = line[i+14];
                        number[count][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;  
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*1000 + (line[i+4]- 48)*100 + (line[i+5]- 48)*10 + (line[i+6]- 48)  ;       
                    }
                    if(line[i+8] == 'n' && line[i+16] == ')')
                    {   int num1 = line[i+9];
                        int num2 = line[i+10];
                        int num3 = line[i+11];
                        int num4 = line[i+12];
                        int num5 = line[i+13];
                        int num6 = line[i+14];
                        int num7 = line[i+15];
                        number[count][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                        pin[count][nets] = (line[i+2]-48)*10000 + (line[i+3]-48)*1000 + (line[i+4]- 48)*100 + (line[i+5]- 48)*10 + (line[i+6]- 48);
                    }
                }
            }
            number[count][0] = nets;// store the number of nets of the macro in the first column
            count ++; //current number of macros
        }   
    }

    while(index1 < count) //check if there are same nets for different macros
    {
        for(int i = 0; i < number[index1][0]; i ++) // starting from the first one
        {
            for(int j = index1 + 1; j < count; j++)
            {
                for(int k = 1; k < number[j][0]; k++) 
                {
                    if(number[index1][i] == number[j][k] && number[index1][i] != 0) // chck if the i-th net of the index macro and the k-th net of macro j are the same, j>index
                    {
                        samenet[counter][0] = index1; //if two nets are found the same, store the number of each macro and the name of the net
                        samenet[counter][1] = j;
                        samenet[counter][2] = number[index1][i];
                        counter++; 
                    }
                }
            }
        }
        index1 ++;
    }
    std::cout << count << endl;

    for(int t = 0; t < 1000; t++) //count the relevance of each pair of macro
    {
        // cout << samenet[t][0] << " " << samenet[t][1] << " " << samenet[t][2] << endl;
        relevance[samenet[t][0]][samenet[t][1]]+=1;
        relevance[samenet[t][1]][samenet[t][0]]+=1;
        output_name_1[t][0] = samenet[t][0];
        output_name_2[t][0] = samenet[t][1];

        for(int o = 1; o < 100; o++)
        {
            output_name_1[t][o] = store_name[samenet[t][0]][o]; //store the names of paired macros
            output_name_2[t][o] = store_name[samenet[t][1]][o];
        }
    }
    myFile.close();
};

double count_length(Macro M_1, Macro M_2, int DIR1, int DIR2)
{   
    if(DIR1 == 4)
    {
        for(int i = 0; i < M_1.PinList.size(); i++)
        {
            M_1.PinList[i].pinx = 2*M_1.x - M_1.PinList[i].pinx;
        }
    }
    else if(DIR1 == 6)
    {
        for(int i = 0; i < M_1.PinList.size(); i++)
        {
            M_1.PinList[i].piny = 2*M_1.y + 2*M_1.size_y - M_1.PinList[i].piny;
        }
    }
    else if(DIR1 == 2)
    {
        for(int i = 0; i < M_1.PinList.size(); i++)
        {
            M_1.PinList[i].piny = 2*M_1.y + 2*M_1.size_y - M_1.PinList[i].piny;
        }
        for(int i = 0; i < M_1.PinList.size(); i++)
        {
            M_1.PinList[i].pinx = 2*M_1.x - M_1.PinList[i].pinx;
        }
    }

    if(DIR2 == 4)
    {
        for(int i = 0; i < M_2.PinList.size(); i++)
        {
            M_2.PinList[i].pinx = 2*M_2.x - M_2.PinList[i].pinx;
        }
    }
    else if(DIR2 == 6)
    {
        for(int i = 0; i < M_2.PinList.size(); i++)
        {
            M_2.PinList[i].piny = 2*M_2.y + 2*M_2.size_y - M_2.PinList[i].piny;
        }
    }
    else if(DIR2 == 2)
    {
        for(int i = 0; i < M_2.PinList.size(); i++)
        {
            M_2.PinList[i].pinx = 2*M_2.x - M_2.PinList[i].pinx;
        }
        for(int i = 0; i < M_2.PinList.size(); i++)
        {
            M_2.PinList[i].piny = 2*M_2.y + 2*M_2.size_y - M_2.PinList[i].piny;
        }
        
    }

    // turn the type and the name of the two macros into strings

    strcat(M_1.type, " ");
    strcat(M_1.type, M_1.name);
    strcat(M_1.type, " ");
    strcat(M_2.type, " ");
    strcat(M_2.type, M_2.name);
    strcat(M_2.type, " ");
    int num1 = 0;
    int num2 = 0;
    for(int i = 0; i < 1000; i ++)
    {   
        if(strcmp(M_1.type, store_name[i])==0)
        {   
            num1 = i;
        }
        if(strcmp(M_2.type, store_name[i])==0)
        {   
            num2 = i;
        }
    }
    

    //find connected nets in samenet[][]

    int count_pin = 0;
    int pinlist[50];
    for(int i = 0; i < 1000; i ++)
    {   
        if(num1 == samenet[i][0] && num2 == samenet[i][1])
        {
            pinlist[count_pin] = samenet[i][2];
            count_pin++;
        }
        if(num1 == samenet[i][1] && num2 == samenet[i][0])
        {
            pinlist[count_pin] = samenet[i][2];
            count_pin++;
        }
    }

    cout << count_pin << endl;
    for(int i = 0; i < count_pin; i ++)
    {
        cout << pinlist[i] << endl;
    }

    //find the corresponding pin name in the number array and pin array

    char pin_1[100][100];
    char pin_2[100][100];
    int ind = 0;
    while(ind < count_pin )
    {
        for(int j = 0; j < 10000; j ++)
        {
            if(pinlist[ind]!= 0 && pinlist[ind] == number[num1][j])
            {
                pin_1[ind][0] = (pin[num1][j]/10000)+48;
                if(pin[num1][j]%10000 >= 1000)
                {   
                    // cout << pin[num1][j] << " 1 " << ind << endl;
                    pin_1[ind][1] = (pin[num1][j]%10000)/1000+48;
                    pin_1[ind][2] = (pin[num1][j]%10000)%1000/100+48;
                    pin_1[ind][3] = (pin[num1][j]%10000)%100/10+48;
                    pin_1[ind][4] = (pin[num1][j]%10000)%1048;
                }
                else if(pin[num1][j]%10000 < 1000 && pin[num1][j]%10000 >= 100)
                {   
                    // cout << pin[num1][j] << " 2 "<< ind <<endl;
                    pin_1[ind][1] = (pin[num1][j]%10000)/100+48;
                    pin_1[ind][2] = (pin[num1][j]%10000)%100/10+48;
                    pin_1[ind][3] = (pin[num1][j]%10000)%10+48;
                }
                else if(pin[num1][j]%10000 < 100 && pin[num1][j]%10000 >= 10)
                {   
                    // cout << pin[num1][j] << " 3" << ind <<endl;
                    pin_1[ind][1] = (pin[num1][j]%10000)/10+48;
                    pin_1[ind][2] = (pin[num1][j]%10000)%10+48;
                }
                else if(pin[num1][j]%10000 < 10)
                {   
                    // cout << pin[num1][j] << " 4 " << ind <<endl;
                    pin_1[ind][1] = (pin[num1][j]%10000)+48;
                }
            }
            if(pinlist[ind]!= 0 && pinlist[ind] == number[num2][j])
            {
                pin_2[ind][0] = (pin[num2][j]/10000)+48;
                if(pin[num2][j]%10000 >= 1000)
                {   
                    // cout << pin[num2][j] << " 5 " << ind <<endl;
                    pin_2[ind][1] = (pin[num2][j]%10000)/1000+48;
                    pin_2[ind][2] = (pin[num2][j]%10000)%1000/100+48;
                    pin_2[ind][3] = (pin[num2][j]%10000)%100/10+48;
                    pin_2[ind][4] = (pin[num2][j]%10000)%10+48;
                }
                else if(pin[num2][j]%10000 < 1000 && pin[num2][j]%10000 >= 100)
                {   
                    // cout << pin[num2][j] << " 6 " << ind <<endl;
                    pin_2[ind][1] = (pin[num2][j]%10000)/100+48;
                    pin_2[ind][2] = (pin[num2][j]%10000)%100/10+48;
                    pin_2[ind][3] = (pin[num2][j]%10000)%10+48;
                }
                else if(pin[num2][j]%10000 < 100 && pin[num2][j]%10000 >= 10)
                {   
                    // cout << pin[num2][j] << " 7 " << ind <<endl;
                    pin_2[ind][1] = (pin[num2][j]%10000)/10+48;
                    pin_2[ind][2] = (pin[num2][j]%10000)%10+48;
                }
                else if(pin[num2][j]%10000 < 10)
                {   
                    // cout << pin[num2][j] << " 8 " << ind <<endl;
                    pin_2[ind][1] = (pin[num2][j]%10000)+48;
                }
            }
        }
        ind++;
    }
    for(int i = 0; i < count_pin; i++ )
    {
        cout << pin_1[i] << " ";
        
    }
    cout << endl;
    for(int i = 0; i < count_pin; i++ )
    {
        cout << pin_2[i] << " ";
        
    }
    cout << endl;
    
    //find the corresponding location of the pinin Macrolist

    double x1_tmp[100];
    double y1_tmp[100];
    double x2_tmp[100];
    double y2_tmp[100];

    for(int i = 0; i < count_pin; i ++)
    {
        for(int j = 0; j < M_1.PinList.size(); j++)
        {
            
            if(strcmp(pin_1[i], M_1.PinList[j].pinName) == 0)
            {
                x1_tmp[i] = M_1.PinList[j].pinx;
                y1_tmp[i] = M_1.PinList[j].piny;
            }
        }
        for(int j = 0; j < M_2.PinList.size(); j++)
        {
            if(strcmp(pin_2[i], M_2.PinList[j].pinName) == 0)
            {
                x2_tmp[i] = M_2.PinList[j].pinx;
                y2_tmp[i] = M_2.PinList[j].piny;
            }
        }   
    }

    for(int i = 0; i < count_pin; i++)
    {
        cout << "( " << x1_tmp[i] << ", " << y1_tmp[i] << " )  " <<  "( " << x2_tmp[i] << ", " << y2_tmp[i] << " )" << endl;
    }
    
        

    // //find the distance
    double HPWL = 0;
    double x_max = -100000000;
    double x_min = 100000000;
    double y_max = -100000000; 
    double y_min = 100000000;
    for(int i = 0; i < count_pin; i ++)
    {
        if(x1_tmp[i] > x_max)
        {
            x_max = x1_tmp[i];
        }
        else if(x1_tmp[i] < x_min)
        {
            x_min = x1_tmp[i];
        }
        if(x2_tmp[i] > x_max)
        {
            x_max = x2_tmp[i];
        }
        else if(x2_tmp[i] < x_min)
        {
            x_min = x2_tmp[i];
        }
        if(y1_tmp[i] > y_max)
        {
            y_max = y1_tmp[i];
        }
        else if(y1_tmp[i] < y_min)
        {
            y_min = y1_tmp[i];
        }
        if(y2_tmp[i] > y_max)
        {
            y_max = y2_tmp[i];
        }
        else if(y2_tmp[i] < y_min)
        {
            y_min = y2_tmp[i];
        }
    }
    // cout << "( " << x_max << ", " << y_max << " )  " <<  "( " << x_min << ", " << y_min << " )" << endl;
    if(count_pin > 0)
    {
        return(fabs(x_max - x_min)+fabs(y_max-y_min));
    }
    else
    {
        return 0;
    }
    

};

double HPWL_total(vector<Macro>macro_list)
{
    double total_HPWL = 0;
    int number = macro_list.size();
    int dir_list[300];
    cout << number << " +++";
    
    for(int i = 0; i < number; i++)
    {
        if(macro_list[i].name[0] == 'F')
        {
            if(macro_list[i].name[0] == 'N')
            {
                dir_list[i] = 4;
            }
            else if(macro_list[i].name[0] == 'S')
            {
                dir_list[i] = 6;
            }
        }
        else if(macro_list[i].name[0] == 'S')
        {
            dir_list[i] = 2;
        }
    }

     
    for(int i = 0; i < number; i ++)
    {
        for(int j = i+1; j < number; j++)
        {
            total_HPWL += count_length(macro_list[i], macro_list[j], dir_list[i], dir_list[i]);
        }
    }
    return total_HPWL;
    // N = 0, S = 2, FN = 4, FS = 6
}

int main(int argc, char *argv[]) {

    // Parse Verilog file
    myVParse(argv[1]);
    
    // Parse DEF file
    myDEFParse(argv[4]);
    
    // Pasre LEF file
    myLEFParse(argv[2]);

    
    // for(int i=0;i<count; i++){
    //     for(int j=0;j<count;j++){
    //         cout << relevance[i][j] << " ";
    //     }
    //     cout << endl;
    // }
    // Write the output

    writeDMP(argv[4], argv[6]);

    cout << "The HPWL is : " << endl;
    //cout << count_length(MacroList[83], MacroList[81], 0, 0) << endl;
    cout << HPWL_total(MacroList) << endl;

    return 0;
}



