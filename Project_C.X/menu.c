#include "menu.h"
#include "sys_config.h"
#include "uart.h"
#include <sys/attribs.h>
#include <xc.h>
#include <stdio.h>

void DefaultMenu(float weight){
    PutString("\e[1;1H\e[2J");  //Clear screen  
    PutString("\e[33m");        // Make the letters yellow
    
    PutString("Current Weight: ");
    PutFloat(weight,1);
    PutString("  g");
    PutStringn("\e[0m");         // Reset to default color (white)
}

void Menu(uint8_t option, int value){
    switch(option){
        case 0:
            PutStringn("---------MENU--------");
            PutStringn("1- Set Scale Mode");
            PutStringn("2- Set Tare");
            PutStringn("3- Set Max Mode");
            PutStringn("4- Set Hold Mode");
            PutString("Choice: ");
            PutInt(value);
            PutStringn(" ");
            break;
        case 1:
            PutStringn("\e[33m1\e[0m- Auto; \e[33m2\e[0m- Scale1 (0 .. 400) g; \e[33m3\e[0m- Scale2 (0 .. 800) g;");
            PutString("Desired Scale Mode: ");
            PutInt(value);
            PutStringn(" ");
            break;
        case 2:
            PutStringn("Tare redefined; Press Enter.");
            break;
        case 3:
            PutStringn("");
            break;
        case 4:
            PutStringn("");
            break;
        default:
            PutStringn("Invalid Option; Press Enter.");
            break;
    };
}