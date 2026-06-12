/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "app_display.h"
#include "bme280.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
BME280_Data_t sensor;

osMutexId_t I2CMutex;

static StaticTask_t SensorTaskControlBlock;
static StackType_t SensorTaskStack[512];
static StaticTask_t DisplayTaskControlBlock;
static StackType_t DisplayTaskStack[512];
static StaticQueue_t SensorQueueControlBlock;
static uint8_t SensorQueueStorage[sizeof(BME280_Data_t)];
/* USER CODE END Variables */
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .cb_mem = &SensorTaskControlBlock,
  .cb_size = sizeof(SensorTaskControlBlock),
  .stack_mem = SensorTaskStack,
  .stack_size = sizeof(SensorTaskStack),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .cb_mem = &DisplayTaskControlBlock,
  .cb_size = sizeof(DisplayTaskControlBlock),
  .stack_mem = DisplayTaskStack,
  .stack_size = sizeof(DisplayTaskStack),
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for sensorQueue */
osMessageQueueId_t sensorQueueHandle;
const osMessageQueueAttr_t sensorQueue_attributes = {
  .name = "sensorQueue",
  .cb_mem = &SensorQueueControlBlock,
  .cb_size = sizeof(SensorQueueControlBlock),
  .mq_mem = SensorQueueStorage,
  .mq_size = sizeof(SensorQueueStorage),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void CheckThreadCreated(osThreadId_t thread, const char *name);

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void *argument);
void StartDisplayTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  const osMutexAttr_t I2CMutex_attributes =
  {
    .name = "I2CMutex"
};

  I2CMutex = osMutexNew(&I2CMutex_attributes);
  if (I2CMutex == NULL)
  {
    Error_Handler();
  }
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensorQueue */
  sensorQueueHandle = osMessageQueueNew (1, sizeof(BME280_Data_t), &sensorQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  if(sensorQueueHandle == NULL)
  {
    printf("Queue Create Failed\r\n");
    Error_Handler();
  }
  else
  {
    printf("Queue Create OK\r\n");
  }
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  CheckThreadCreated(SensorTaskHandle, "SensorTask");
  CheckThreadCreated(DisplayTaskHandle, "DisplayTask");
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartSensorTask */
/**
  * @brief  Function implementing the SensorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  /* Infinite loop */
  for(;;)
  {
    BME280_ReadData(&sensor);

    osMessageQueuePut(
      sensorQueueHandle,
      &sensor,
      0,
      0
      );
    printf("Queue Put\r\n");

    osDelay(1000);
  }
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  printf("DisplayTask Start\r\n");
  BME280_Data_t rxData;
  /* Infinite loop */
  for (;;)
  {

    if (

        osMessageQueueGet(
            sensorQueueHandle,
            &rxData,
            NULL,
            osWaitForever
        ) == osOK
    )
    {
      printf(
          "T=%ld H=%ld P=%ld\r\n",
          rxData.temperature,
          rxData.humidity,
          rxData.pressure
      );
    }
  }
  /* USER CODE END StartDisplayTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void CheckThreadCreated(osThreadId_t thread, const char *name)
{
  if (thread == NULL)
  {
    printf("Create %s failed\r\n", name);
    Error_Handler();
  }
}

/* USER CODE END Application */

