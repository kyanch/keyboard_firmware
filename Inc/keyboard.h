#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "stdint.h"

// 按键扫描次数, 超过这个次数没扫描就等待中断唤醒
#define KB_MAX_SCAN_COUNT 127

#define KB_MAX_ROW_COUNT 8
#define KB_MAX_COL_COUNT 16
// 按键消抖时间
#define KB_BOUNCING_DELAY 20

typedef enum { KBS_Paused, KBS_Start, KBS_Bouncing,KBS_Confirm, KBS_Pressed } KB_STATE;
typedef enum {
  KBSS_Paused,
  KBSS_Start,
  KBSS_Progress,
  KBSS_Finish
} KB_SCAN_STATE;

typedef struct {
  uint8_t map[128];
  uint8_t row;
  uint8_t col;
  // 键盘状态机
  KB_STATE state;
  uint8_t time;
  uint16_t keys[KB_MAX_ROW_COUNT];
  // 键盘扫描状态机
  uint8_t scan_count;
  KB_SCAN_STATE scan_state;
  uint8_t scan_row;
  uint16_t scan_result[KB_MAX_ROW_COUNT];
} Keyboard_CB;

extern Keyboard_CB keyboard;

void kb_init();

KB_STATE kb_scan(uint8_t *report, KB_STATE lastState);

// 依赖sysTick的键盘扫描
inline void kb_scan_tick();

#endif