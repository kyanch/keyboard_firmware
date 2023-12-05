#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "stdint.h"

typedef enum {
    LCtrl=1,
    LShift=(1<<1),
    LAlt=(1<<2),
    LGUI=(1<<3),
    RCtrl=(1<<4),
    RShift=(1<<5),
    RAlt=(1<<6),
    RGUI=(1<<7),

} KB_MODIFIER;

// kb_report[0] 保存按键数量，后8字节为真正的report内容
// extern uint8_t kb_report[9];

void kb_report_clean();
void kb_add_modifiers(uint8_t mods);
void kb_add_key(uint8_t key);
void kb_send();

#endif