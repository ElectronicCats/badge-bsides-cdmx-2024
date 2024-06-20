/* BSD non-blocking socket example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <esp_pthread.h>
#include <pthread.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "protocol_examples_common.h"

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "sdkconfig.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "192.168.0.123"
#define WEB_PORT   "5000"
#define WEB_PATH   "/"

static const char* TAG = "example";

static const char* REQUEST = "GET " WEB_PATH
                             " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER ":" WEB_PORT
                             "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

static void http_get_task(void* pvParameters) {
  const struct addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
  };
  struct addrinfo* res;
  struct in_addr* addr;
  int s, r;
  char recv_buf[64];

  while (1) {
    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

    if (err != 0 || res == NULL) {
      ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }

    /* Code to print the resolved IP.

       Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code
     */
    addr = &((struct sockaddr_in*) res->ai_addr)->sin_addr;
    // ESP_LOGI(TAG, "[%d/] DNS lookup succeeded. IP=%s", task_number,
    // inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0) {
      ESP_LOGE(TAG, "... Failed to allocate socket.");
      freeaddrinfo(res);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    // ESP_LOGI(TAG, "... allocated socket");

    if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
      ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
      close(s);
      freeaddrinfo(res);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }

    freeaddrinfo(res);

    if (write(s, REQUEST, strlen(REQUEST)) < 0) {
      ESP_LOGE(TAG, "... socket send failed");
      close(s);
      vTaskDelay(400 / portTICK_PERIOD_MS);
      continue;
    }
    // ESP_LOGI(TAG, "[%d] Task", task_number);
    // ESP_LOGI(TAG, "... socket send success");
    close(s);
  }
}

void catdos_module_begin() {
  // ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  // ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* This helper function configures Wi-Fi or Ethernet, as selected in
   * menuconfig. Read "Establishing Wi-Fi or Ethernet Connection" section in
   * examples/protocols/README.md for more information about this function.
   */
  ESP_ERROR_CHECK(example_connect());

  xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);

  pthread_attr_t attr;
  pthread_t thread1, thread2, thread3, thread4, thread5, thread6, thread7,
      thread8;
  esp_pthread_cfg_t esp_pthread_cfg;
  int res;

  res = pthread_create(&thread1, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 1 created\n");
  res = pthread_attr_init(&attr);
  assert(res == 0);
  pthread_attr_setstacksize(&attr, 16384);
  res = pthread_create(&thread2, &attr, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 2 created\n");
  res = pthread_create(&thread3, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 4 created\n");
  res = pthread_create(&thread4, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 4 created\n");
  res = pthread_create(&thread5, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 5 created\n");
  res = pthread_create(&thread6, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 6 created\n");
  res = pthread_create(&thread7, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 7 created\n");
  res = pthread_create(&thread8, NULL, (void*) http_get_task, NULL);
  assert(res == 0);
  printf("Thread 8 created\n");

  res = pthread_join(thread1, NULL);
  assert(res == 0);
  printf("Thread 1 joined\n");
  res = pthread_join(thread2, NULL);
  assert(res == 0);
  printf("Thread 2 joined\n");
  res = pthread_join(thread3, NULL);
  assert(res == 0);
  printf("Thread 3 joined\n");
  res = pthread_join(thread4, NULL);
  assert(res == 0);
  printf("Thread 4 joined\n");
  res = pthread_join(thread5, NULL);
  assert(res == 0);
  printf("Thread 5 joined\n");
  res = pthread_join(thread6, NULL);
  assert(res == 0);
  printf("Thread 6 joined\n");
  res = pthread_join(thread7, NULL);
  assert(res == 0);
  printf("Thread 7 joined\n");
  res = pthread_join(thread8, NULL);
  assert(res == 0);
  printf("Thread 8 joined\n");
}
