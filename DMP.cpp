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
int UNITS_DISTANCE_MICRONS;
int diearea_x;
int diearea_y;

void myDEFParse(char* F){
    FILE *DEF_file = fopen(F, "r"); 
    char s[128];
    for(int j=0;j<10;j++) fscanf(DEF_file, "%s", s);
    UNITS_DISTANCE_MICRONS = atoi(s);
    // cout << UNITS_DISTANCE_MICRONS << endl;

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
    // printf("%d %d\n", diearea_x, diearea_y);

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

int mcount = 0; // the number of macros
int number[256][10000]; //first column : number of nets connected to the macro, after : name of all nets connected to the macro (max : 1024)
int samenet[50000][3]; //store pairs of macros connected by the same net
char store_name[1000][128]; // store the name of every macros
int index = 0; //used when finding same nets
int counter = 0; //count the total number of paired macros
char output_name[1000][150]; //gives the names of paired macros
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
                store_name[mcount][q] = line[q]; //if the sentence starts with 'b', store its name
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
                        number[mcount][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+5] == 'n' && line[i+12] == ')') // ex : ' .o2(n123456)
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        int num6 = line[i+11];
                        number[mcount][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;
                    }
                    if(line[i+5] == 'n' && line[i+13] == ')') // ex : ' .o2(n1234567)
                    {   int num1 = line[i+6];
                        int num2 = line[i+7];
                        int num3 = line[i+8];
                        int num4 = line[i+9];
                        int num5 = line[i+10];
                        int num6 = line[i+11];
                        int num7 = line[i+12];
                        number[mcount][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                    }
                    if(line[i+6] == 'n' && line[i+12] == ')') // ex : ' .o12(n12345)
                    {   int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        number[mcount][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+6] == 'n' && line[i+13] == ')')
                    {
                        int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        int num6 = line[i+12];
                        number[mcount][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 + num6-48;
                    }
                    if(line[i+6] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+7];
                        int num2 = line[i+8];
                        int num3 = line[i+9];
                        int num4 = line[i+10];
                        int num5 = line[i+11];
                        int num6 = line[i+12];
                        int num7 = line[i+13];
                        number[mcount][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                    }
                    if(line[i+7] == 'n' && line[i+13] == ')')// ex : ' .o123(n12345)
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        number[mcount][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+7] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        int num6 = line[i+13];
                        number[mcount][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;              
                    }
                    if(line[i+7] == 'n' && line[i+15] == ')')
                    {   int num1 = line[i+8];
                        int num2 = line[i+9];
                        int num3 = line[i+10];
                        int num4 = line[i+11];
                        int num5 = line[i+12];
                        int num6 = line[i+13];
                        int num7 = line[i+14];
                        number[mcount][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                    }
                    if(line[i+8] == 'n' && line[i+14] == ')')
                    {   int num1 = line[i+9];
                        int num2 = line[i+10];
                        int num3 = line[i+11];
                        int num4 = line[i+12];
                        int num5 = line[i+13];
                        number[mcount][nets] = (num1-48)*10000 + (num2-48)*1000 + (num3-48)*100 + (num4-48)*10 + (num5-48);
                    }
                    if(line[i+8] == 'n' && line[i+15] == ')')
                    {   int num1 = line[i+9];
                        int num2 = line[i+10];
                        int num3 = line[i+11];
                        int num4 = line[i+12];
                        int num5 = line[i+13];
                        int num6 = line[i+14];
                        number[mcount][nets] = (num1-48)*100000 + (num2-48)*10000 + (num3-48)*1000 + (num4-48)*100+ (num5-48)*10 +num6-48;              
                    }
                    if(line[i+8] == 'n' && line[i+16] == ')')
                    {   int num1 = line[i+9];
                        int num2 = line[i+10];
                        int num3 = line[i+11];
                        int num4 = line[i+12];
                        int num5 = line[i+13];
                        int num6 = line[i+14];
                        int num7 = line[i+15];
                        number[mcount][nets] = (num1-48)*1000000 + (num2-48)*100000 + (num3-48)*10000+ (num4-48)*1000 + (num5-48)*100 + (num6-48)*10 + num7 -48 ;
                    }
                }
            }
            number[mcount][0] = nets;// store the number of nets of the macro in the first column
            mcount ++; //current number of macros
        }   
    }

    while(index < mcount) //check if there are same nets for different macros
    {
        for(int i = 0; i < number[index][0]; i ++) // starting from the first one
        {
            for(int j = index + 1; j < mcount; j++)
            {
                for(int k = 1; k < number[j][0]; k++) 
                {
                    if(number[index][i] == number[j][k] && number[index][i] != 0) // chck if the i-th net of the index macro and the k-th net of macro j are the same, j>index
                    {
                        samenet[counter][0] = index; //if two nets are found the same, store the number of each macro and the name of the net
                        samenet[counter][1] = j;
                        samenet[counter][2] = number[index][i];
                        counter++; 
                    }
                }
            }
        }
        index ++;
    }
    //cout << mcount << endl;

    for(int t = 0; t < 1000; t++) //count the relevance of each pair of macro
    {
        // cout << samenet[t][0] << " " << samenet[t][1] << " " << samenet[t][2] << endl;
        relevance[samenet[t][0]][samenet[t][1]]+=1;
        relevance[samenet[t][1]][samenet[t][0]]+=1;
        for(int o = 0; o <76; o++)
        {
            output_name[t][o] = store_name[samenet[t][0]][o]; //store the names of paired macros
        }
        for(int o = 76; o <150; o++)
        {
            output_name[t][o] = store_name[samenet[t][1]][o-76];
        } 

    }
    myFile.close();
};

bool isLegal(Macro i){
    // N
    bool overlap = true;
    if(strcmp(i.dir,"N")==0){
        // cout << i.dir << " ";
        if(i.x < 0 || i.x+int(i.size_x) > diearea_x 
        || i.y < 0 || i.y+int(i.size_y) > diearea_y) return false;
        for(auto check:MacroList){
            if(strcmp(i.name, check.name)!=0){
                if(strcmp(check.dir,"N")==0){
                    overlap = !((check.x>i.x+int(i.size_x))||(i.x>check.x+int(check.size_x))
                              ||(check.y>i.y+int(i.size_y))||(i.y>check.y+int(check.size_y)));
                }
                else if(strcmp(check.dir,"FN")==0){
                    overlap = !((check.x-int(check.size_x)>i.x+int(i.size_x))||(i.x>check.x)
                              ||(check.y>i.y+int(i.size_y))||(i.y>check.y+int(check.size_y)));
                }
                else if(strcmp(check.dir,"S")==0){
                    overlap = !((check.x-int(check.size_x)>i.x+int(i.size_x))||(i.x>check.x)
                              ||(check.y-int(check.size_y)>i.y+int(i.size_y))||(i.y>check.y));
                }
                else{
                    overlap = !((check.x>i.x+int(i.size_x))||(i.x>check.x+int(check.size_x))
                              ||(check.y-int(check.size_y)>i.y+int(i.size_y))||(i.y>check.y));
                }
                if(overlap) return false;
            }
        }
    }
    // FN
    else if(strcmp(i.dir,"FN")==0){
        // cout << i.dir << " ";
        if(i.x-int(i.size_x) < 0 || i.x > diearea_x 
        || i.y < 0 || i.y+int(i.size_y) > diearea_y) return false;
        for(auto check:MacroList){
            if(strcmp(i.name, check.name)!=0){
                if(strcmp(check.dir,"N")==0){
                    overlap = !((check.x>i.x)||(i.x-int(i.size_x)>check.x+int(check.size_x))
                              ||(check.y>i.y+int(i.size_y))||(i.y>check.y+int(check.size_y)));
                }
                else if(strcmp(check.dir,"FN")==0){
                    overlap = !((check.x-int(check.size_x)>i.x)||(i.x-int(i.size_x)>check.x)
                              ||(check.y>i.y+int(i.size_y))||(i.y>check.y+int(check.size_y)));                    
                }
                else if(strcmp(check.dir,"S")==0){
                    overlap = !((check.x-int(check.size_x)>i.x)||(i.x-int(i.size_x)>check.x)
                              ||(check.y-int(check.size_y)>i.y+int(i.size_y))||(i.y>check.y));
                }
                else{
                    overlap = !((check.x>i.x)||(i.x-int(i.size_x)>check.x+int(check.size_x))
                              ||(check.y-int(check.size_y)>i.y+int(i.size_y))||(i.y>check.y));                    
                }
                if(overlap) return false;
            }
        }
    }
    // S
    else if(strcmp(i.dir,"S")==0){
        // cout << i.dir << " ";
        if(i.x-int(i.size_x) < 0 || i.x > diearea_x 
        || i.y-int(i.size_y) < 0 || i.y > diearea_y) return false;
        for(auto check:MacroList){
            if(strcmp(i.name, check.name)!=0){
                if(strcmp(check.dir,"N")==0){
                    overlap = !((check.x>i.x)||(i.x-int(i.size_x)>check.x+int(check.size_x))
                              ||(check.y>i.y)||(i.y-int(i.size_y)>check.y+int(check.size_y)));
                }
                else if(strcmp(check.dir,"FN")==0){
                    overlap = !((check.x-int(check.size_x)>i.x)||(i.x-int(i.size_x)>check.x)
                              ||(check.y>i.y)||(i.y-int(i.size_y)>check.y+int(check.size_y)));                    
                }
                else if(strcmp(check.dir,"S")==0){
                    overlap = !((check.x-int(check.size_x)>i.x)||(i.x-int(i.size_x)>check.x)
                              ||(check.y-int(check.size_y)>i.y)||(i.y-int(i.size_y)>check.y));
                }
                else{
                    overlap = !((check.x>i.x)||(i.x-int(i.size_x)>check.x+int(check.size_x))
                              ||(check.y-int(check.size_y)>i.y)||(i.y+int(i.size_y)>check.y));                    
                }
                if(overlap) return false;
            }
        }
    }
    // FS
    else{
        // cout << i.dir << " ";
        if(i.x < 0 || i.x+int(i.size_x) > diearea_x 
        || i.y-int(i.size_y) < 0 || i.y > diearea_y) return false;
        for(auto check:MacroList){
            if(strcmp(i.name, check.name)!=0){
                if(strcmp(check.dir,"N")==0){
                    overlap = !((check.x>i.x+int(i.size_x))||(i.x>check.x+int(check.size_x))
                              ||(check.y>i.y)||(i.y-int(i.size_y)>check.y+int(check.size_y)));
                }
                else if(strcmp(check.dir,"FN")==0){
                    overlap = !((check.x-int(check.size_x)>i.x+int(i.size_x))||(i.x>check.x)
                              ||(check.y>i.y)||(i.y-int(i.size_y)>check.y+int(check.size_y)));
                }
                else if(strcmp(check.dir,"S")==0){
                    overlap = !((check.x-int(check.size_x)>i.x+int(i.size_x))||(i.x>check.x)
                              ||(check.y-int(check.size_y)>i.y)||(i.y-int(i.size_y)>check.y));
                }
                else{
                    overlap = !((check.x>i.x+int(i.size_x))||(i.x>check.x+int(check.size_x))
                              ||(check.y-int(check.size_y)>i.y)||(i.y-int(i.size_y)>check.y));
                }
                if(overlap) return false;
            }
        }
    }

    return true;
};

int count_length(Macro M_1, Macro M_2){

};

void mySlidingWindow(int constraint, int size){
    int windowSizeX = diearea_x/size; 
    int windowSizeY = diearea_y/size;
    for(int windowX=1e7; windowX+windowSizeX<=diearea_x; windowX+=windowSizeX/2){
        for(int windowY=0; windowY+windowSizeY<=diearea_y; windowY+=windowSizeY/2){
            vector<pair<Macro, int>> insideMacros;
            // 先找出哪先macro在現在這個window裡 -> 兩矩形有相交
            for(auto i:MacroList){
                bool isOverlap = !((windowX>i.x+int(i.size_x))||(i.x>windowX+windowSizeX)
                                 ||(windowY>i.y+int(i.size_y))||(i.y>windowY+windowSizeY));
                if(isOverlap) insideMacros.push_back(make_pair(i, 0));
            }
            // 為了能滿足relevance的操作
            for(auto i:insideMacros){
                char temp1[128], temp2[128];
                // This is all for Yutang's format
                strcpy(temp1, i.first.type);
                int len1 = strlen(temp1);
                temp1[len1] = ' ';
                temp1[len1 + 1] = '\0';
                strcpy(temp2, i.first.name);
                int len2 = strlen(temp2);
                temp2[len2] = ' ';
                temp2[len2 + 1] = '\0';
                strcat(temp1, temp2);
                //////////////////////////////////
                for(int j=0;j<mcount;j++){
                    if(strcmp(temp1, store_name[j])==0){
                        i.second = j+1;
                        //cout << i.first.name << " " << i.second << endl;
                        break;
                    }
                }
            }
            cout << endl;
            // 現在我們可以正式對每個window去找他們的最佳解

            // for(auto i:insideMacros){
            //     cout << i.first.name << endl;
            //     int origin_x = i.first.x;
            //     int origin_y = i.first.y;
            //     char origin_dir[128];
            //     strcpy(origin_dir, i.first.dir);
            //     for(int j=0;j<=constraint;j++){
            //         for(int k=0;k<=constraint-j;k++){

            //             i.first.x += j*UNITS_DISTANCE_MICRONS;
            //             i.first.y += k*UNITS_DISTANCE_MICRONS;
            //             if(isLegal(i.first)) cout << "N ";
            //             i.first.x = origin_x;
            //             i.first.y = origin_y;
            //             strcpy(i.first.dir, origin_dir);

            //             i.first.x += j*UNITS_DISTANCE_MICRONS;
            //             i.first.y += k*UNITS_DISTANCE_MICRONS;
            //             strcpy(i.first.dir, "FN");
            //             if(isLegal(i.first)) cout << "FN ";
            //             i.first.x = origin_x;
            //             i.first.y = origin_y;
            //             strcpy(i.first.dir, origin_dir);

            //             i.first.x += j*UNITS_DISTANCE_MICRONS;
            //             i.first.y += k*UNITS_DISTANCE_MICRONS;
            //             strcpy(i.first.dir, "S");
            //             if(isLegal(i.first)) cout << "S ";
            //             i.first.x = origin_x;
            //             i.first.y = origin_y;
            //             strcpy(i.first.dir, origin_dir);

            //             i.first.x += j*UNITS_DISTANCE_MICRONS;
            //             i.first.y += k*UNITS_DISTANCE_MICRONS;
            //             strcpy(i.first.dir, "FS");
            //             if(isLegal(i.first)) cout << "FS ";
            //             i.first.x = origin_x;
            //             i.first.y = origin_y;
            //             strcpy(i.first.dir, origin_dir);

            //         }
            //     }
            //     cout << endl;
            // }

            
        }
    }
    
    return;
};

int main(int argc, char *argv[]) {

    // Parse Verilog file
    myVParse(argv[1]);
    
    // Parse DEF file
    myDEFParse(argv[4]);
    
    // Pasre LEF file
    myLEFParse(argv[2]);
    // Algorithm
    FILE *TXT_file = fopen(argv[5], "r");
    char s[128];
    fscanf(TXT_file, "%s", s);
    fscanf(TXT_file, "%s", s);
    int constraint = atoi(s);
    fclose(TXT_file);
    mySlidingWindow(constraint, 4);
    // cout << store_name[1] << "hi";
    return 0;
}

