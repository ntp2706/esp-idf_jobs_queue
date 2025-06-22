#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

//case: jobs queue
typedef enum {
    JOB_PRINT_TEXT,
    JOB_PRINT_NUMBER
} job_type_t;

typedef struct {
    job_type_t type;
    union {
        const char *text;
        int number;
    };
} job_t;

QueueHandle_t xQueue;

TimerHandle_t xTimer_1;
TimerHandle_t xTimer_2;

void job_handler(void *pvParameters)
{
    job_t job;
    while (1) {
        if (xQueueReceive(xQueue, &job, portMAX_DELAY)) {
            switch (job.type) {
                case JOB_PRINT_TEXT:
                    printf("Job: %s\n", job.text);
                    break;
                case JOB_PRINT_NUMBER:
                    printf("Job: %d\n", job.number);
                    break;
                default:
                    printf("Unknown job type\n");
                    break;
            }
        }
    }
}

void client_task_1(void *pvParameters)
{
    job_t job;
    job.type = JOB_PRINT_TEXT;
    job.text = "Hello from client 1";
    
    if (xQueueSend(xQueue, &job, portMAX_DELAY) != pdPASS) {
        printf("Failed to send job from client 1\n");
    }
    
    vTaskDelete(NULL);
}

void client_task_2(void *pvParameters)
{
    job_t job;
    job.type = JOB_PRINT_NUMBER;
    job.number = 6;

    if (xQueueSend(xQueue, &job, portMAX_DELAY) != pdPASS) {
        printf("Failed to send job from client 2\n");
    }

    vTaskDelete(NULL);
}

void timer_callback_1(TimerHandle_t xTimer)
{
    printf("Timer 1 callback: Creating client_task_1\n");
    xTaskCreate(client_task_1, "client_task_1", 2048, NULL, 1, NULL);
}

void timer_callback_2(TimerHandle_t xTimer)
{
    printf("Timer 2 callback: Creating client_task_2\n");
    xTaskCreate(client_task_2, "client_task_2", 2048, NULL, 1, NULL);
}

void jobs_queue(void)
{
    xQueue = xQueueCreate(10, sizeof(job_t));
    if (xQueue == NULL) {
        printf("Failed to create queue\n");
        return;
    }

    xTaskCreate(job_handler, "job_handler", 2048, NULL, 2, NULL);
    
    xTimer_1 = xTimerCreate(
        "to_client_task_1",
        pdMS_TO_TICKS(3000),
        pdTRUE,
        NULL,
        timer_callback_1
    );

    if (xTimer_1 != NULL)
        xTimerStart(xTimer_1, 0);

    xTimer_2 = xTimerCreate(
        "to_client_task_2",
        pdMS_TO_TICKS(5000),
        pdTRUE,
        NULL,
        timer_callback_2
    );

    if (xTimer_2 != NULL)
        xTimerStart(xTimer_2, 0);

}
//---------------------------------------

void app_main(void)
{
    jobs_queue();
}
