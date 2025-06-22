#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

TimerHandle_t to_nor;
TimerHandle_t to_mid;
TimerHandle_t to_prior;

TaskHandle_t xNorHandle = NULL;
TaskHandle_t xMidHandle = NULL;
TaskHandle_t xPriorHandle = NULL;

void nor(void *pvParameters)
{
    TickType_t startTime = xTaskGetTickCount();
    TickType_t duration = pdMS_TO_TICKS(100);
    uint32_t x = 0;

    while (1)
    {
        if ((xTaskGetTickCount() - startTime) >= duration) {
            printf("nor stop...\n");
            break;
        }

        x++;

        printf("nor running: %lu\n", x);
    }

    vTaskDelete(NULL);
}

void mid(void *pvParameters)
{
    if (xNorHandle != NULL && eTaskGetState(xNorHandle) == eRunning) {
        printf("mid suspend nor\n");
        vTaskSuspend(xNorHandle);
    }

    TickType_t startTime = xTaskGetTickCount();
    TickType_t duration = pdMS_TO_TICKS(100);
    uint32_t x = 0;

    while (1)
    {
        if ((xTaskGetTickCount() - startTime) >= duration) {
            if (xNorHandle != NULL && eTaskGetState(xNorHandle) == eSuspended) {
                printf("mid resume nor\n");
                vTaskResume(xNorHandle);
            }
            printf("mid stop...\n");
            break;
        }

        x++;

        printf("mid running: %lu\n", x);
    }

    vTaskDelete(NULL);
}

void prior(void *pvParameters)
{
    if (xMidHandle != NULL && eTaskGetState(xMidHandle) == eRunning) {
        printf("prior suspend mid\n");
        vTaskSuspend(xMidHandle);
    }

    TickType_t startTime = xTaskGetTickCount();
    TickType_t duration = pdMS_TO_TICKS(100);
    uint32_t x = 0;

    while (1)
    {
        if ((xTaskGetTickCount() - startTime) >= duration) {
            if (xMidHandle != NULL && eTaskGetState(xMidHandle) == eSuspended) {
                printf("prior resume mid\n");
                vTaskResume(xMidHandle);
            }

            printf("prior stop...\n");
            break;
        }

        x++;

        printf("prior running: %lu\n", x);
    }

    vTaskDelete(NULL);
}

void timer_callback_nor(TimerHandle_t xTimer)
{
    printf("Timer callback: Creating nor task\n");
    xTaskCreate(nor, "nor", 2048, NULL, 1, NULL);
}

void timer_callback_mid(TimerHandle_t xTimer)
{
    printf("Timer callback: Creating mid task\n");
    xTaskCreate(mid, "mid", 2048, NULL, 3, NULL);
}

void timer_callback_prior(TimerHandle_t xTimer)
{
    printf("Timer callback: Creating prior task\n");
    xTaskCreate(prior, "prior", 2048, NULL, 5, NULL);
}

//case: nor -> mid -> preempt -> nor -> prior -> preempt -> mid

void preemptive_scheduling(void)
{
    printf("Creating nor task\n");
    xTaskCreate(nor, "nor", 2048, NULL, 1, &xNorHandle);

    to_mid = xTimerCreate(
        "to_mid",
        pdMS_TO_TICKS(10),
        pdFALSE,
        NULL,
        timer_callback_mid
    );
    if (to_mid != NULL)
        xTimerStart(to_mid, 0);

    to_prior = xTimerCreate(
        "to_prior",
        pdMS_TO_TICKS(60),
        pdFALSE,
        NULL,
        timer_callback_prior
    );
    if (to_prior != NULL)
        xTimerStart(to_prior, 0);
}
//---------------------------------------

void app_main(void)
{
    preemptive_scheduling();
}