#include "kb_hid.h"
#include "usbd_ctlreq.h"

static uint8_t KB_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t KB_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t KB_HID_Setup(USBD_HandleTypeDef *pdev,
                            USBD_SetupReqTypedef *req);

static uint8_t *KB_HID_GetFSCfgDesc(uint16_t *length);

static uint8_t KB_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
// clang-format off
USBD_ClassTypeDef  KB_HID=
{
  KB_HID_Init,
  KB_HID_DeInit,
  KB_HID_Setup,
  NULL, /*EP0_TxSent*/
  NULL, /*EP0_RxReady*/
  KB_HID_DataIn, /*DataIn*/
  NULL, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,
  NULL,
  KB_HID_GetFSCfgDesc,
  NULL,
  NULL,
};
// clang-format on

/* USB HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t KB_CfgFSDesc[KB_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09,                        /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    KB_CONFIG_DESC_SIZ,
    /* wTotalLength: Bytes returned */
    0x00, 0x01, /*bNumInterfaces: 1 interface*/
    0x01,       /*bConfigurationValue: Configuration value*/
    0x00,       /*iConfiguration: Index of string descriptor describing
  the configuration*/
    0xA0,       /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32,       /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Interface Descriptor of KB ****************/
    /* 09 */
    0x09,                    /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
    0x00,                    /*bInterfaceNumber: Number of Interface*/
    0x00,                    /*bAlternateSetting: Alternate setting*/
    0x02,                    /*bNumEndpoints*/
    0x03,                    /*bInterfaceClass: HID*/
    0x01,                    /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x01, /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,    /*iInterface: Index of string descriptor*/
    /******************** HID Descriptor of KB ********************/
    /* 18 */
    0x09,                /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11,                /*bcdHID: HID Class Spec release number*/
    0x01, 0x00,          /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    KB_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** IN Endpoint Descriptor of KB ********************/
    /* 27 */
    0x07,                   /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

    KB_EPIN_ADDR,           /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,                   /*bmAttributes: Interrupt endpoint*/
    KB_EPIN_SIZE,           /*wMaxPacketSize: 4 Byte max */
    0x00, HID_FS_BINTERVAL, /*bInterval: Polling Interval */
    /* 34 */
    /******************** OUT Endpoint Descriptor of KB ********************/
    /* 27 */
    0x07,                   /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

    KB_EPOUT_ADDR,          /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,                   /*bmAttributes: Interrupt endpoint*/
    KB_EPOUT_SIZE,          /*wMaxPacketSize: 4 Byte max */
    0x00, HID_FS_BINTERVAL, /*bInterval: Polling Interval */
                            /* 34 */
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t KB_HID_Desc[KB_HID_DESC_SIZ] __ALIGN_END = {
    /* 18 */
    0x09,                /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11,                /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00, /*bCountryCode: Hardware target country*/
    0x01, /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22, /*bDescriptorType*/
    KB_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
    0x00,
};

__ALIGN_BEGIN static uint8_t KB_ReportDesc[KB_REPORT_DESC_SIZE] __ALIGN_END = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)
    0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x01, //   LOGICAL_MAXIMUM (1)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x95, 0x08, //   REPORT_COUNT (8)
    0x81, 0x02, //   INPUT (Data,Var,Abs)
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x01, //   INPUT (Cnst,Ary,Abs)
    0x95, 0x05, //   REPORT_COUNT (5)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x05, 0x08, //   USAGE_PAGE (LEDs)
    0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05, //   USAGE_MAXIMUM (Kana)
    0x91, 0x02, //   OUTPUT (Data,Var,Abs)
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x03, //   REPORT_SIZE (3)
    0x91, 0x01, //   OUTPUT (Cnst,Ary,Abs)
    0x95, 0x06, //   REPORT_COUNT (6)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x65, //   LOGICAL_MAXIMUM (101)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)
    0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00, //   INPUT (Data,Ary,Abs)
    0xc0        // END_COLLECTION
};

/**
 * @brief  USBD_HID_Init
 *         Initialize the HID interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t KB_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
  /* Open EP IN */
  USBD_LL_OpenEP(pdev, KB_EPIN_ADDR, USBD_EP_TYPE_INTR, KB_EPIN_SIZE);
  pdev->ep_in[KB_EPIN_ADDR & 0xFU].is_used = 1U;
  USBD_LL_OpenEP(pdev, KB_EPOUT_ADDR, USBD_EP_TYPE_INTR, KB_EPOUT_SIZE);
  pdev->ep_out[KB_EPOUT_ADDR & 0xFU].is_used = 1U;

  pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

  if (pdev->pClassData == NULL) {
    return USBD_FAIL;
  }

  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;

  return USBD_OK;
}

/**
 * @brief  USBD_HID_Init
 *         DeInitialize the HID layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t KB_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
  /* Close HID EPs */
  USBD_LL_CloseEP(pdev, KB_EPIN_ADDR);
  pdev->ep_in[KB_EPIN_ADDR & 0xFU].is_used = 0U;
  USBD_LL_CloseEP(pdev, KB_EPOUT_ADDR);
  pdev->ep_out[KB_EPOUT_ADDR & 0xFU].is_used = 0U;

  /* FRee allocated memory */
  if (pdev->pClassData != NULL) {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
 * @brief  USBD_HID_Setup
 *         Handle the HID specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t KB_HID_Setup(USBD_HandleTypeDef *pdev,
                            USBD_SetupReqTypedef *req) {
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK) {
  case USB_REQ_TYPE_CLASS:
    switch (req->bRequest) {
    case HID_REQ_SET_PROTOCOL:
      hhid->Protocol = (uint8_t)(req->wValue);
      break;

    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
      break;

    case HID_REQ_SET_IDLE:
      hhid->IdleState = (uint8_t)(req->wValue >> 8);
      break;

    case HID_REQ_GET_IDLE:
      USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    }
    break;
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest) {
    case USB_REQ_GET_STATUS:
      if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
      } else {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_GET_DESCRIPTOR:
      if (req->wValue >> 8 == HID_REPORT_DESC) {
        len = MIN(KB_REPORT_DESC_SIZE, req->wLength);
        pbuf = KB_ReportDesc;
      } else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE) {
        pbuf = KB_HID_Desc;
        len = MIN(KB_HID_DESC_SIZ, req->wLength);
      } else {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
        break;
      }
      USBD_CtlSendData(pdev, pbuf, len);
      break;

    case USB_REQ_GET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
      } else {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_SET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED) {
        hhid->AltSetting = (uint8_t)(req->wValue);
      } else {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    }
    break;

  default:
    USBD_CtlError(pdev, req);
    ret = USBD_FAIL;
    break;
  }

  return ret;
}

/**
 * @brief  USBD_HID_SendReport
 *         Send HID Report
 * @param  pdev: device instance
 * @param  buff: pointer to report
 * @retval status
 */
uint8_t KB_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len) {
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;

  if (pdev->dev_state == USBD_STATE_CONFIGURED) {
    if (hhid->state == HID_IDLE) {
      hhid->state = HID_BUSY;
      USBD_LL_Transmit(pdev, KB_EPIN_ADDR, report, len);
    }
  }
  return USBD_OK;
}

/**
 * @brief  USBD_HID_GetPollingInterval
 *         return polling interval from endpoint descriptor
 * @param  pdev: device instance
 * @retval polling interval
 */
uint32_t KB_HID_GetPollingInterval(USBD_HandleTypeDef *pdev) {
  uint32_t polling_interval = 0U;

  /* HIGH-speed endpoints */
  if (pdev->dev_speed == USBD_SPEED_HIGH) {
    /* Sets the data transfer polling interval for high speed transfers.
     Values between 1..16 are allowed. Values correspond to interval
     of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1U << (HID_HS_BINTERVAL - 1U))) / 8U);
  } else /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full
    speed transfers */
    polling_interval = HID_FS_BINTERVAL;
  }

  return ((uint32_t)(polling_interval));
}

/**
 * @brief  USBD_HID_GetCfgFSDesc
 *         return FS configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *KB_HID_GetFSCfgDesc(uint16_t *length) {
  *length = KB_CONFIG_DESC_SIZ;
  return KB_CfgFSDesc;
}

/**
 * @brief  USBD_HID_DataIn
 *         handle data IN Stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t KB_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}