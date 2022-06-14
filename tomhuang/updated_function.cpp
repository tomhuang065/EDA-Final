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
            M_1.PinList[i].piny = 2*M_1.y - M_1.PinList[i].piny;
        }
    }
    else if(DIR1 == 2)
    {
        for(int i = 0; i < M_1.PinList.size(); i++)
        {
            M_1.PinList[i].piny = 2*M_1.y - M_1.PinList[i].piny;
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
            M_2.PinList[i].piny = 2*M_2.y - M_2.PinList[i].piny;
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
            M_2.PinList[i].piny = 2*M_2.y - M_2.PinList[i].piny;
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


    