#include "keyboard.h"
#include "hid_kb.h"

uint8_t kb_report[9]={0};

extern USBD_HandleTypeDef hUsbDeviceFS;

void kb_report_clean(){
    for (uint8_t i = 0; i < 9; i++)
        kb_report[i]=0;
}

void kb_add_modifiers(uint8_t mods){
    kb_report[1]|=mods;
}

void kb_add_key(uint8_t key){
    if(kb_report[0]>5)
        return;
    kb_report[3+kb_report[0]]=key;
    kb_report[0]++;
}


void kb_send(){
    KB_SendReport(&hUsbDeviceFS,kb_report+1,8);
}