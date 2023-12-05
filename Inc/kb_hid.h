#ifndef __KB_HID_H
#define __KB_HID_H

#include "stdint.h"
#include "usbd_hid.h"

#define KB_EPIN_ADDR 0x81U
#define KB_EPIN_SIZE 8U
#define KB_EPOUT_ADDR 0x01U
#define KB_EPOUT_SIZE 1U

#define KB_CONFIG_DESC_SIZ 41U
#define KB_HID_DESC_SIZ 9U
#define KB_REPORT_DESC_SIZE 63U

extern USBD_ClassTypeDef KB_HID;
#define KB_HID_CLASS &KB_HID;

uint8_t KB_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);

uint32_t KB_HID_GetPollingInterval(USBD_HandleTypeDef *pdev);

#endif