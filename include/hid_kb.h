#ifndef __KB_HID_H
#define __KB_HID_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "stdint.h"

#define KB_EPIN_ADDR 0x81U
#define KB_EPIN_SIZE 8U
#define KB_EPOUT_ADDR 0x01U
#define KB_EPOUT_SIZE 1U

#define KB_CONFIG_DESC_SIZ 41U
#define KB_HID_DESC_SIZ 9U
#define KB_REPORT_DESC_SIZE 63U

// #define HID_DESCRIPTOR_TYPE           0x21U
// #define HID_REPORT_DESC               0x22U

// #ifndef HID_HS_BINTERVAL
// #define HID_HS_BINTERVAL            0x07U
// #endif /* HID_HS_BINTERVAL */

// #ifndef HID_FS_BINTERVAL
// #define HID_FS_BINTERVAL            0x0AU
// #endif /* HID_FS_BINTERVAL */

// #define HID_REQ_SET_PROTOCOL          0x0BU
// #define HID_REQ_GET_PROTOCOL          0x03U

// #define HID_REQ_SET_IDLE              0x0AU
// #define HID_REQ_GET_IDLE              0x02U

// #define HID_REQ_SET_REPORT            0x09U
// #define HID_REQ_GET_REPORT            0x01U

extern USBD_ClassTypeDef KB_HID;
#define KB_HID_CLASS &KB_HID;

uint8_t KB_SendReport(USBD_HandleTypeDef *pdev,
                      uint8_t *report,
                      uint16_t len);

uint32_t KB_HID_GetPollingInterval(USBD_HandleTypeDef *pdev);

#endif