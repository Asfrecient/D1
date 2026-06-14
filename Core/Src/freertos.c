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
#include "app_sensor.h"
#include "bme280.h"
#include "task.h"
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
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for MonitorTask */
osThreadId_t MonitorTaskHandle;
const osThreadAttr_t MonitorTask_attributes = {
  .name = "MonitorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for sensorQueue */
osMessageQueueId_t sensorQueueHandle;
const osMessageQueueAttr_t sensorQueue_attributes = {
  .name = "sensorQueue"
};
/* Definitions for SensorTimer */
osTimerId_t SensorTimerHandle;
const osTimerAttr_t SensorTimer_attributes = {
  .name = "SensorTimer"
};
/* Definitions for SensorSem */
osSemaphoreId_t SensorSemHandle;
const osSemaphoreAttr_t SensorSem_attributes = {
  .name = "SensorSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void CheckThreadCreated(osThreadId_t thread, const char *name);

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void *argument);
void StartDisplayTask(void *argument);
void StartMonitorTask(void *argument);
void SensorTimerCallback(void *argument);

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

  /* Create the semaphores(s) */
  /* creation of SensorSem */
  SensorSemHandle = osSemaphoreNew(1, 0, &SensorSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of SensorTimer */
  SensorTimerHandle = osTimerNew(SensorTimerCallback, osTimerPeriodic, NULL, &SensorTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensorQueue */
  sensorQueueHandle = osMessageQueueNew (1, 12, &sensorQueue_attributes);

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

  /* creation of MonitorTask */
  MonitorTaskHandle = osThreadNew(StartMonitorTask, NULL, &MonitorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osTimerStart(
    SensorTimerHandle,
    1000);

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
  BME280_Data_t txData;
  /* Infinite loop */
  for(;;)
  {

    osSemaphoreAcquire(
        SensorSemHandle,
        osWaitForever);


    APP_SensorRead(&txData);

    osMessageQueuePut(
      sensorQueueHandle,
      &txData,
      0,
      0
      );

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


      APP_DisplayUpdate(&rxData);
      }
    }
  }

  /* USER CODE END StartDisplayTask */


/* USER CODE BEGIN Header_StartMonitorTask */
/**
* @brief Function implementing the MonitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMonitorTask */
void StartMonitorTask(void *argument)
{
  /* USER CODE BEGIN StartMonitorTask */
  /* Infinite loop */
  for(;;)
  {
    printf("\r\n");
    printf("===== Task Monitor =====\r\n");

    printf(
        "SensorTask  Stack : %lu\r\n",
        osThreadGetStackSpace(
            SensorTaskHandle));

    printf(
        "DisplayTask Stack : %lu\r\n",
        osThreadGetStackSpace(
            DisplayTaskHandle));

    printf("========================\r\n");

    osDelay(5000);
  }
  /* USER CODE END StartMonitorTask */
}

/* SensorTimerCallback function */
void SensorTimerCallback(void *argument)
{
  /* USER CODE BEGIN SensorTimerCallback */


  osSemaphoreRelease(SensorSemHandle);
  /* USER CODE END SensorTimerCallback */
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

