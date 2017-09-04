/*************************************** Copyright (c)******************************************************
** File name            :   main.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   主文件，包含应用代码
**
**--------------------------------------------------------------------------------------------------------
** Created by           :   01课堂 lishutong
** Created date         :   2016-06-01
** Version              :   1.0
** Descriptions         :   The original version
**
**--------------------------------------------------------------------------------------------------------
** Copyright            :   版权所有，禁止用于商业用途
** Author Blog          :   http://ilishutong.com
**********************************************************************************************************/
void delay (int count) 
{
    while (--count > 0);
}

int flag;

int main () 
{
    for (;;) {
        flag = 0;
        delay(100);
        flag = 1;
        delay(100);
    }
    
    return 0;
}
