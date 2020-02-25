#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"

#include "driver/gpio.h"
#include "m5stickc.h"

// For mbedtls_md_hmac
#include "mbedtls/md.h"

// My secrets
#include "locky_secrets.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define PORT 4141

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static int s_connect_desired = 0;
static TaskHandle_t lock_task = NULL;
static esp_netif_t *s_wifi_netif;

/* Logging tag */
static const char *TAG = "smart-locky";

/* Prototypes */
static void screen_print(char *buf);
static void unlock_remote_lock();
static int decrypt_secret(int id, char *dest, int dest_len);

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_connect_desired && s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        char buf[32];
        sprintf(buf, "ip: " IPSTR, IP2STR(&event->ip_info.ip));
        screen_print(buf);
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static int decrypt_secret(int id, char *dest, int dest_len) {
  //ESP_LOGI(TAG, "Secret pointers: %p %p %p %p %p",
  //    SECRETS[0], SECRETS[1], SECRETS[2], SECRETS[3], SECRETS[4]);
  int secret_len = *(int *)(SECRETS[id+1]);
  //ESP_LOGI(TAG, "Secret is %d bytes.", secret_len);
  if (secret_len >= dest_len)
    return -1;
  int key = *(int *)(SECRETS[0]);
  //ESP_LOGI(TAG, "Key is %d.", key);
  memcpy(dest, SECRETS[id], secret_len);
  dest[(int)secret_len] = '\0';
  for (int i=0;i<secret_len;i++) {
    dest[i] ^= ((key+i) & 0xFF);
  }
  //ESP_LOGI(TAG, "secret is: %s", dest);
  return secret_len;
}

static int wifi_init_sta(void) {
    // Just in case
    s_connect_desired = 0;
    esp_wifi_disconnect();
    esp_wifi_stop();
    s_retry_num = 0;

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
        },
    };
    decrypt_secret(
        SECRET_PSK,
        (char *)wifi_config.sta.password,
        sizeof(wifi_config.sta.password));

    s_connect_desired = 1;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "password is %s", wifi_config.sta.password);
    ESP_LOGI(TAG, "wifi_init_sta kicked off.");

    /* Clear bits first */
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT|WIFI_FAIL_BIT);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s",
                 CONFIG_ESP_WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s",
                 CONFIG_ESP_WIFI_SSID);
        esp_wifi_stop();
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    int rv = bits & WIFI_FAIL_BIT;

    return rv;
}

void remote_unlock_task() {
  while (1) {
    // Block on mutex
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // Actually unlock
    unlock_remote_lock();
  }
  // Shouldn't exit
  vTaskDelete(NULL);
}

static void unlock_remote_lock() {
  screen_print("Starting unlock");
  ESP_LOGI(TAG, "Unlock starting.");
  ESP_LOGI(TAG, "STA mode starting.");
  // TODO: check if we are already connected?
  if(wifi_init_sta()) {
    ESP_LOGE(TAG, "Connection failed, unable to proceed.");
    goto fail;
  }

  // Make TCP connection to server
  esp_netif_ip_info_t my_ip;
  if (esp_netif_get_ip_info(s_wifi_netif, &my_ip) != ERR_OK) {
    ESP_LOGE(TAG, "Error getting my ip.");
    goto fail;
  }
  uint32_t srv = (ip4_addr_get_u32(&(my_ip.ip)) & 0xFFFFFF) | 0x1000000;
  esp_ip4_addr_t srv_a = {
    .addr = srv
  };
  ESP_LOGI(TAG, "Connecting to " IPSTR " from " IPSTR,
      IP2STR(&srv_a), IP2STR(&(my_ip.ip)));
  struct sockaddr_in srv_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(PORT)
  };
  srv_addr.sin_addr.s_addr = srv;
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (sock < 0) {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    goto fail;
  }
  int err = connect(sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
  if (err) {
    ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
    goto fail;
  }
  char buf[64];
  int len = decrypt_secret(SECRET_PASSWD, buf, sizeof(buf));
  if (len < 0) {
    ESP_LOGE(TAG, "Failed decrypting.");
    close(sock);
    goto fail;
  }
  int sent = send(sock, buf, len, 0);
  if (sent != len) {
    ESP_LOGE(TAG, "Error sending!");
    close(sock);
    goto fail;
  }
  len = recv(sock, buf, sizeof(buf)-1, 0);
  if (len < 0) {
    ESP_LOGE(TAG, "Error unlocking!");
    close(sock);
    goto fail;
  }
  buf[len] = (char)0;
  screen_print(buf);
  close(sock);
  screen_print("Unlocked");
  return;

fail:
    screen_print("Connection failed.");
    return;
}

#define SCREEN_LINES 5
#define MAX_LINE_LEN 32
#define MARGIN_LEFT 4

static char s_screen_buf[SCREEN_LINES][MAX_LINE_LEN+1];

static void screen_print(char *buf) {
    static int line_idx = 0;
    int line_height = TFT_getfontheight();
    // Copy the data in
    strncpy(s_screen_buf[line_idx], buf, MAX_LINE_LEN);
    s_screen_buf[line_idx][MAX_LINE_LEN] = '\0';
    // Print screen
    TFT_fillScreen(TFT_BLACK);
    for (int i=SCREEN_LINES; i>0; i--) {
      char *line = s_screen_buf[(i+line_idx) % SCREEN_LINES];
      TFT_print(
          line,
          MARGIN_LEFT,
          (i-1)*line_height+1);
    }
    // Increment idx
    line_idx++;
    line_idx %= SCREEN_LINES;
}

void btn_a_handler(void *handler_arg, esp_event_base_t base, int32_t id,
    void *event_data) {
  ESP_LOGI(TAG, "btn_a_handler");
  switch(id) {
    case M5BUTTON_BUTTON_CLICK_EVENT:
      m5display_on();
      // do unlock on another task
      xTaskNotifyGive(lock_task);
      break;
  }
}

void btn_b_handler(void *handler_arg, esp_event_base_t base, int32_t id,
    void *event_data) {
  static int screen_toggle = 0;
  ESP_LOGI(TAG, "btn_b_handler");
  switch(id) {
    case M5BUTTON_BUTTON_HOLD_EVENT:
      if (screen_toggle) {
        ESP_LOGI(TAG, "display on");
        if (m5display_on() == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed turning display on.");
        }
        m5display_set_backlight_level(3);
      } else {
        ESP_LOGI(TAG, "display off");
        if (m5display_off() == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed turning display off.");
        }
        m5display_set_backlight_level(0);
      }
      screen_toggle ^= 1;
      break;
  }
}

// One time setup for networking
void net_setup(void) {
  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  s_wifi_netif = esp_netif_create_default_wifi_sta();
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
}


void app_main(void) {
    m5stickc_config_t m5config = M5STICKC_CONFIG_DEFAULT();
    m5config.power.lcd_backlight_level = 3;
    m5_init(&m5config);

    // Check TFT
    screen_print("Loading...");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    net_setup();

    // Start lock task
    if (xTaskCreate(
            remote_unlock_task, "remote_unlock_task",
            4096, NULL, 1, &lock_task) != pdPASS) {
      ESP_LOGE(TAG, "Error starting unlock task!");
    }

    // Setup event handlers
    esp_event_handler_register_with(
        m5_event_loop,
        M5BUTTON_A_EVENT_BASE,
        M5BUTTON_BUTTON_CLICK_EVENT,
        btn_a_handler,
        NULL);
    esp_event_handler_register_with(
        m5_event_loop,
        M5BUTTON_B_EVENT_BASE,
        M5BUTTON_BUTTON_CLICK_EVENT,
        btn_b_handler,
        NULL);
    esp_event_handler_register_with(
        m5_event_loop,
        M5BUTTON_B_EVENT_BASE,
        M5BUTTON_BUTTON_HOLD_EVENT,
        btn_b_handler,
        NULL);

    // timeout display after 15 seconds
    if (ESP_OK != m5display_timeout(15000)) {
      ESP_LOGE(TAG, "Error starting display timeout.");
    }

    screen_print("Ready.");
}
