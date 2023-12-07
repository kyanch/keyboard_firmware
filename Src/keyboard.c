#include "keyboard.h"

#include "main.h"

Keyboard_CB keyboard = {0};

inline void clean_report(uint8_t *report, uint8_t len);

inline void kb_state_change();
inline void scan_state_change();

inline void kb_select_row(uint8_t row);
inline uint16_t kb_fetch_col();

void clean_report(uint8_t *report, uint8_t len) {
  for (int i = 0; i < len; i++)
    report[i] = 0;
}

void kb_init() {
  keyboard.scan_count = 0;
  // 从flash读取键盘布局
}

void kb_scan_tick() {
  kb_state_change();
  scan_state_change();
}

void scan_state_change() {
  uint8_t row = keyboard.scan_row;
  switch (keyboard.scan_state) {
  case KBSS_Paused:
    // 扫描暂停
    break;
  case KBSS_Start:
    // 初始化扫描
    // keyboard.scan_count++;
    keyboard.scan_state = KBSS_Progress;
    // No break, 立即执行Progress 减少一次tick
  case KBSS_Progress:
    // 选择一行，并获取该行的键盘状态;
    for (int i = 0; i < KB_MAX_ROW_COUNT; i++) {
      kb_select_row(row);
      keyboard.scan_result[row] = kb_fetch_col();
    }
    keyboard.scan_state = KBSS_Finish;
    break;
  case KBSS_Finish:
    // 扫描结束
    keyboard.scan_state = KBSS_Paused;
    break;
  }
}

void kb_state_chagne() {
  uint16_t *keys = keyboard.keys;
  uint16_t *new_keys = keyboard.scan_result;
  switch (keyboard.state) {
  case KBS_Paused:
    break;
  case KBS_Start:
    keyboard.time = 0;
    keyboard.scan_state = KBSS_Start;
    break;
  case KBS_Bouncing:
    if (KB_BOUNCING_DELAY == keyboard.time++) {
      // 保存第一次扫描的数据
      for (int i = 0; i < KB_MAX_ROW_COUNT; i++) {
        keys[i] = new_keys[i];
      }
      // 等待延时后，再次启动扫描
      keyboard.state = KBS_Confirm;
      keyboard.scan_state = KBSS_Start;
    }
    break;
  case KBS_Confirm:
    // 等待扫描完成
    if (keyboard.scan_state != KBSS_Finish)
      break;
    for (int i = 0; i < 8; i++)
      // keys[i]=new_keys *(keys ^ new_keys);
      keys[i] = new_keys[i] & keys[i];
    break;
  case KBS_Pressed:
    keyboard.state = KBS_Paused;
    break;
  }
}

void kb_select_row(uint8_t row) {
  static GPIO_TypeDef *rowPinGroup[] = {GPIOA, GPIOA, GPIOA, GPIOA,
                                        GPIOA, GPIOA, GPIOA, GPIOA};
  static uint8_t rowPinBit[] = {1, 2, 1, 1, 1, 1, 1, 1};
  uint8_t prev_row = (7 + row) & 0x07;
  rowPinGroup[prev_row]->ODR &= ~(((uint16_t)1) << rowPinBit[prev_row]);
  rowPinGroup[row]->ODR |= ((uint16_t)1) << rowPinBit[row];
}

uint16_t kb_fetch_col() {
  static GPIO_TypeDef *colPinGroup[] = {
      GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA,
      GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA};
  static uint8_t colPinBit[] = {1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1};
  uint16_t res = 0;
  for (int i = 0; i < 16; i++) {
    uint16_t tmp = 1;
    tmp = (colPinGroup[i]->IDR & (tmp << colPinBit[i])) != 0;
    res |= tmp << i;
  }
  return res;
}