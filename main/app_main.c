// #include "esp_freertos_hooks.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* LVGL specific */
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "ui/ui.h"
#include "lvgl/demos/widgets/lv_demo_widgets.h"
// #include "lvgl/demos/benchmark/lv_demo_benchmark.h"

/*********************
 *      DEFINES
 *********************/
#define LV_HOR_RES_MAX 320
#define LV_VER_RES_MAX 240
#define LV_TICK_PERIOD_MS 10
#define TAG "app_main"

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg) {
  (void)arg;

  lv_tick_inc(LV_TICK_PERIOD_MS);
}
static void guiTask(void *pvParameter);

SemaphoreHandle_t xGuiSemaphore;

/**********************
 *   APPLICATION MAIN
 **********************/
void app_main() {

  /* If you want to use a task to create the graphic, you NEED to create a
   * Pinned task Otherwise there can be problem such as memory corruption and so
   * on. NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to
   * core 0 */
  xTaskCreatePinnedToCore(guiTask, "gui", 4096 * 2, NULL, 10, NULL, 0);
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */

static void guiTask(void *pvParameter) {

  (void)pvParameter;
  xGuiSemaphore = xSemaphoreCreateMutex();

  lv_init();
  /* Initialize SPI or I2C bus used by the drivers */
  lvgl_driver_init();

  /* Initialize the working buffers. */
  // static lv_color_t buf1[DISP_BUF_SIZE];
  // static lv_color_t buf2[DISP_BUF_SIZE];

  lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA); assert(buf1 != NULL);
  lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA); assert(buf2 != NULL);
  /*register DISP driver*/
  static lv_disp_draw_buf_t disp_buf;
  uint32_t size_in_px = DISP_BUF_SIZE;

  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &disp_buf;
  disp_drv.flush_cb = disp_driver_flush;
  disp_drv.hor_res = LV_HOR_RES_MAX;
  disp_drv.ver_res = LV_VER_RES_MAX;
  // disp_drv.rotated = 1;
  lv_disp_drv_register(&disp_drv);

  /*register TOUCH driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read_cb = touch_driver_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t periodic_timer_args = {
      .callback = &lv_tick_task, .name = "periodic_gui"};
  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

  // ui_init();
  lv_demo_widgets();
  // lv_demo_benchmark();

  while (1) {

    vTaskDelay(pdMS_TO_TICKS(10));
    /* Try to take the semaphore, call lvgl related function on success */
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_task_handler();
      xSemaphoreGive(xGuiSemaphore);
    }
  }
  free(buf1);
  free(buf2);
  vTaskDelete(NULL);
}
/*
// initialize buffer on heap
  lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf1 != NULL);

  lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf2 != NULL);
  // free(buf1);
  // free(buf2);
*/