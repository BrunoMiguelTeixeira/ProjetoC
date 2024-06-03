#include "menu.h"
#include "sys_config.h"
#include "uart.h"
#include <sys/attribs.h>
#include <xc.h>
#include <stdio.h>

#define OVERFLOW 10000

void DefaultMenu(float weight, uint8_t mode, float tareVal, uint8_t maxFlag, uint8_t holdFlag){
    PutString("\e[1;1H\e[2J");  //Clear screen  
    PutString("\e[33m");        // Make the letters yellow
    
    // Print the current weight
    PutString("Current Weight: ");
    if(weight == OVERFLOW){
        PutStringn("OVERFLOW");
    }
    else{
        PutInt(weight);
        PutStringn("  g");
    }
    
    // Print the current configuration
    PutChar('\n');
    PutString("Scale Mode: ");
    switch(mode){
        case 0:
            PutString("Auto   (0 .. 800) g");
            break;
        case 1:
            PutString("Scale1 (0 .. 400) g");
            break;
        case 2:
            PutString("Scale2 (0 .. 800) g");
            break;
        default:
            break;
    }

    PutString("\tTare Value: ");
    PutInt(tareVal);

    PutString("\tMax Mode: ");
    if(maxFlag){
        PutString("ON");
    }else{
        PutString("OFF");
    }

    PutString("\tHold Mode: ");
    if(holdFlag){
        PutString("ON");
    }else{
        PutString("OFF");
    }
    
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
            PutStringn("When ready press Enter to set the Tare value.");
            break;
        case 3:
            PutStringn("When ready press Enter to change the Max Mode.");
            break;
        case 4:
            PutStringn("When ready press Enter to change the Hold Mode.");
            break;
        default:
            PutStringn("Invalid Option; Press Enter.");
            break;
    };
}