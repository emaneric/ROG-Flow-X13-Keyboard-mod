/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_hid.h"
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CONSUMER_VOLDOWN  0xF0U
#define CONSUMER_VOLUP    0xF1U
#define CONSUMER_MUTE     0xF2U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

#define NUM_ROWS 9
#define NUM_COLS 20

typedef struct { GPIO_TypeDef *port; uint16_t pin; } GPIOPin_t;

static const GPIOPin_t row_pins[NUM_ROWS] = {
    {GPIOA, GPIO_PIN_3},   /* R0 */
    {GPIOA, GPIO_PIN_4},   /* R1 */
    {GPIOA, GPIO_PIN_6},   /* R2 */
    {GPIOA, GPIO_PIN_7},   /* R3 */
    {GPIOB, GPIO_PIN_1},   /* R4 */
    {GPIOB, GPIO_PIN_2},   /* R5 */
    {GPIOB, GPIO_PIN_8},   /* R6 */
    {GPIOB, GPIO_PIN_9},   /* R7 */
    {GPIOB, GPIO_PIN_12},  /* R8 */
};

static const GPIOPin_t col_pins[NUM_COLS] = {
    {GPIOA, GPIO_PIN_2},   /* C0  */
    {GPIOA, GPIO_PIN_5},   /* C1  */
    {GPIOA, GPIO_PIN_10},  /* C2  */
    {GPIOA, GPIO_PIN_15},  /* C3  */
    {GPIOB, GPIO_PIN_0},   /* C4  */
    {GPIOB, GPIO_PIN_3},   /* C5  */
    {GPIOB, GPIO_PIN_4},   /* C6  */
    {GPIOB, GPIO_PIN_5},   /* C7  */
    {GPIOB, GPIO_PIN_6},   /* C8  */
    {GPIOB, GPIO_PIN_7},   /* C9  */
    {GPIOB, GPIO_PIN_10},  /* C10 */
    {GPIOB, GPIO_PIN_11},  /* C11 */
    {GPIOD, GPIO_PIN_0},   /* C12 */
    {GPIOD, GPIO_PIN_1},   /* C13 */
    {GPIOD, GPIO_PIN_2},   /* C14 */
    {GPIOD, GPIO_PIN_3},   /* C15 */
    {GPIOA, GPIO_PIN_8},   /* C16 */
    {GPIOB, GPIO_PIN_13},  /* C17 */
    {GPIOB, GPIO_PIN_14},  /* C18 */
    {GPIOB, GPIO_PIN_15},  /* C19 */
};

/*
 * HID keycodes indexed by [row][col].
 * 0x00 = no key assigned
 * 0xE0-0xE7 = modifier keys (LCtrl, LShift, LAlt, LGui, RCtrl, RShift, RAlt, RGui)
 * All other values = standard HID keyboard usage codes
 */
static const uint8_t keycode_map[NUM_ROWS][NUM_COLS] = {
    /* C0    C1    C2    C3    C4    C5    C6    C7    C8    C9    C10   C11   C12   C13   C14   C15   C16   C17   C18   C19 */
    /* R0 PA3  */ { 0x00, 0x00, 0x4F, 0x00, 0x00, 0x31, 0x08, 0x00, 0x3B, 0x2D, 0x00, 0x0B, 0x00, 0x40, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00 },
    /* R1 PA4  */ { 0x00, 0x00, 0x4C, 0x2E, 0x00, 0x43, 0x39, 0x35, 0x16, 0x27, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00 },
    /* R2 PA6  */ { 0x00, 0x00, 0x00, 0x2A, 0x00, 0x26, 0x20, 0x1E, 0x1F, 0x13, 0x00, 0x24, 0x00, 0x25, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00 },
    /* R3 PA7  */ { 0x00, 0x00, 0x00, 0x30, 0x00, 0x44, 0x3C, 0x00, 0x1A, 0x2F, 0x00, 0x18, 0x00, 0x0C, 0x50, 0x15, 0x00, 0x00, 0x00, 0x00 },
    /* R4 PB1  */ { 0x00, 0x00, 0x00, 0x07, 0x00, 0x0F, 0x00, 0x04, 0x29, 0x33, 0x00, 0x0D, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00 },
    /* R5 PB2  */ { 0x00, 0xE2, 0x41, 0x28, 0xE1, 0x51, 0x2C, 0x00, 0x1D, 0x52, 0x00, 0x11, 0xE0, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00 },
    /* R6 PB8  */ { 0x00, 0xE6, 0x00, 0x34, 0xE5, 0x37, 0x00, 0x14, 0x1B, 0x38, 0x06, 0x10, 0xE4, 0x36, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00 },
    /* R7 PB9  */ { 0xE3, 0x00, 0x00, 0x00, 0x00, 0x12, 0x3D, 0x2B, 0x3A, 0x45, 0x00, 0x1C, 0x00, 0x3F, 0x42, 0x17, 0x00, 0x00, 0x00, 0x00 },
    /* R8 PB12 */{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, CONSUMER_VOLDOWN, 0x00, CONSUMER_MUTE, CONSUMER_VOLUP },
};

static uint8_t curr_state[NUM_ROWS][NUM_COLS];
static uint8_t prev_state[NUM_ROWS][NUM_COLS];

/* 0xF0-0xF2 are internal markers for consumer keys, looked up in consumer_usage[]. */
static const uint16_t consumer_usage[] = {
    0x00EA,  /* 0xF0: Volume Decrement */
    0x00E9,  /* 0xF1: Volume Increment */
    0x00E2,  /* 0xF2: Mute            */
};

typedef struct {
    uint8_t report_id;  /* Always 1 */
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keycodes[6];
} KeyboardReport_t;

typedef struct {
    uint8_t  report_id;  /* Always 2 */
    uint16_t usage;      /* Consumer usage code; 0 = no key */
} __attribute__((packed)) ConsumerReport_t;

static const uint32_t brightness_levels[4] = {0, 333, 666, 1000};
static int brightness_idx = 2; /* start at medium brightness */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
 * Return true if any three pressed keys form three corners of a row/column
 * rectangle, which means a ghost key may appear at the fourth corner.
 */
static bool has_ghost(void)
{
    for (int r1 = 0; r1 < NUM_ROWS; r1++) {
        for (int c1 = 0; c1 < NUM_COLS; c1++) {
            if (!curr_state[r1][c1]) continue;
            for (int c2 = c1 + 1; c2 < NUM_COLS; c2++) {
                if (!curr_state[r1][c2]) continue;
                /* Two keys share row r1 at c1 and c2.  If any other row has
                 * a key in c1 or c2, we have three corners of a rectangle. */
                for (int r2 = 0; r2 < NUM_ROWS; r2++) {
                    if (r2 == r1) continue;
                    if (curr_state[r2][c1] || curr_state[r2][c2])
                        return true;
                }
            }
        }
    }
    return false;
}

/* Drive each row low in turn and read all column pins. */
static void scan_matrix(void)
{
    GPIO_InitTypeDef init = {0};
    for (int r = 0; r < NUM_ROWS; r++) {
        init.Pin   = row_pins[r].pin;
        init.Mode  = GPIO_MODE_OUTPUT_PP;
        init.Pull  = GPIO_NOPULL;
        init.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(row_pins[r].port, &init);
        HAL_GPIO_WritePin(row_pins[r].port, row_pins[r].pin, GPIO_PIN_RESET);

        HAL_Delay(1); /* settle */

        for (int c = 0; c < NUM_COLS; c++) {
            curr_state[r][c] =
                (HAL_GPIO_ReadPin(col_pins[c].port, col_pins[c].pin) == GPIO_PIN_RESET) ? 1U : 0U;
        }

        init.Mode = GPIO_MODE_INPUT;
        init.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(row_pins[r].port, &init);
    }
}

/* Build and send keyboard + consumer HID reports from the current key state. */
static void send_hid_report(void)
{
    KeyboardReport_t kb = {0};
    kb.report_id = 1U;

    ConsumerReport_t consumer = {0};
    consumer.report_id = 2U;

    if (!has_ghost()) {
        int key_count = 0;
        for (int r = 0; r < NUM_ROWS; r++) {
            for (int c = 0; c < NUM_COLS; c++) {
                if (!curr_state[r][c]) continue;
                uint8_t kc = keycode_map[r][c];
                if (kc == 0x00) continue;
                if (kc >= 0xF0U) {
                    consumer.usage = consumer_usage[kc - 0xF0U];
                } else if (kc >= 0xE0U) {
                    kb.modifiers |= (uint8_t)(1u << (kc - 0xE0U));
                } else if (key_count < 6) {
                    kb.keycodes[key_count++] = kc;
                }
            }
        }
    }

    USBD_HID_HandleTypeDef *hhid =
        (USBD_HID_HandleTypeDef *)hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId];

    uint32_t t = HAL_GetTick();
    while (hhid && hhid->state == USBD_HID_BUSY && (HAL_GetTick() - t) < 10U) {}
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&kb, sizeof(kb));

    t = HAL_GetTick();
    while (hhid && hhid->state == USBD_HID_BUSY && (HAL_GetTick() - t) < 10U) {}
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&consumer, sizeof(consumer));
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_Device_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, brightness_levels[brightness_idx]);

  while (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) { HAL_Delay(10); }
  HAL_Delay(500);

  scan_matrix();
  memcpy(prev_state, curr_state, sizeof(curr_state));

  /* Send empty report so host starts in a clean state */
  KeyboardReport_t empty = {0};
  USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&empty, sizeof(empty));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    scan_matrix();

    /* Drive PC7 (caps lock LED via MOSFET) from bit 1 of the host LED report */
    {
        USBD_HID_HandleTypeDef *hhid =
            (USBD_HID_HandleTypeDef *)hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId];
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7,
            (hhid != NULL && (hhid->led_report[1] & 0x02U)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    /* ROG key (R8 C17): cycle backlight through off -> low -> medium -> high -> off */
    if (curr_state[8][17] && !prev_state[8][17]) {
        brightness_idx = (brightness_idx + 1) % 4;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, brightness_levels[brightness_idx]);
    }

    if (memcmp(curr_state, prev_state, sizeof(curr_state)) != 0) {
        send_hid_report();
        memcpy(prev_state, curr_state, sizeof(curr_state));
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_CRSInitTypeDef pInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the SYSCFG APB clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  pInit.Prescaler = RCC_CRS_SYNC_DIV1;
  pInit.Source = RCC_CRS_SYNC_SOURCE_USB;
  pInit.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  pInit.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
  pInit.ErrorLimitValue = 34;
  pInit.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&pInit);
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA2 PA3 PA4 PA5
                           PA6 PA7 PA8 PA10
                           PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_10
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PB13 PB14
                           PB15 PB3 PB4 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD0 PD1 PD2 PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
