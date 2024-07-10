/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "argtable3/argtable3.h"
#include "cmd_catdos.h"
#include "cmd_wifi.h"
// #include "driver/uart.h"
// #include "driver/uart_vfs.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "linenoise/linenoise.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "oled_screen.h"
#include "preferences.h"
#include "soc/soc_caps.h"

static const char* TAG = "cat_console";
#define PROMPT_STR "bsides"

static void initialize_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

static void initialize_console(void) {
  /* Drain stdout before reconfiguring it */
  fflush(stdout);
  fsync(fileno(stdout));

  /* Disable buffering on stdin */
  setvbuf(stdin, NULL, _IONBF, 0);

  /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
  // uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
  //                                       ESP_LINE_ENDINGS_CR);
  /* Move the caret to the beginning of the next line on '\n' */
  // uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM,
  //                                       ESP_LINE_ENDINGS_CRLF);

  /* Configure UART. Note that REF_TICK is used so that the baud rate remains
   * correct while APB frequency is changing in light sleep mode.
   */
  //   const uart_config_t uart_config = {
  //       .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
  //       .data_bits = UART_DATA_8_BITS,
  //       .parity = UART_PARITY_DISABLE,
  //       .stop_bits = UART_STOP_BITS_1,
  // #if SOC_UART_SUPPORT_REF_TICK
  //       .source_clk = UART_SCLK_REF_TICK,
  // #elif SOC_UART_SUPPORT_XTAL_CLK
  //       .source_clk = UART_SCLK_XTAL,
  // #endif
  //   };
  //   /* Install UART driver for interrupt-driven reads and writes */
  //   ESP_ERROR_CHECK(
  //       uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL,
  //       0));
  //   ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM,
  //   &uart_config));

  /*--- Initialize Console ---*/
  esp_console_repl_t* repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  // initialize_filesystem();
  // repl_config.history_save_path = HISTORY_FILE_PATH;
  repl_config.prompt = "bsides>";

  esp_console_dev_usb_serial_jtag_config_t usbjtag_config =
      ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config,
                                                       &repl_config, &repl));

  /* Tell VFS to use UART driver */
  // uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

  /* Initialize the console */
  //   esp_console_config_t console_config = {.max_cmdline_args = 8,
  //                                          .max_cmdline_length = 256,
  // #if CONFIG_LOG_COLORS
  //                                          .hint_color = atoi(LOG_COLOR_CYAN)
  // #endif
  //   };
  // ESP_ERROR_CHECK(esp_console_init(&console_config));

  /* Configure linenoise line completion library */
  /* Enable multiline editing. If not set, long commands will scroll within
   * single line.
   */
  // linenoiseSetMultiLine(1);

  // /* Tell linenoise where to get command completions and hints */
  // linenoiseSetCompletionCallback(&esp_console_get_completion);
  // linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

  // /* Set command history size */
  // linenoiseHistorySetMaxLen(100);

  // /* Set command maximum length */
  // linenoiseSetMaxLineLen(console_config.max_cmdline_length);

  // /* Don't return empty lines */
  // linenoiseAllowEmpty(false);
}

void cat_console_begin() {
  initialize_nvs();

  initialize_console();

  /* Register commands */
  esp_console_register_help_command();
  register_wifi();
  register_catdos_commands();

  /* Prompt to be printed before each line.
   * This can be customized, made dynamic, etc.
   */
  const char* prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;

  printf(
      "\n"
      "Welcome to the BSides Console\n"
      "Type 'help' to get the list of commands.\n"
      "Use UP/DOWN arrows to navigate through command history.\n"
      "Press TAB when typing command name to auto-complete.\n"
      "Press Enter or Ctrl+C will terminate the console environment.\n");

  /* Figure out if the terminal supports escape sequences */
  int probe_status = linenoiseProbe();
  if (probe_status) { /* zero indicates success */
    printf(
        "\n"
        "Your terminal application does not support escape sequences.\n"
        "Line editing and history features are disabled.\n"
        "On Windows, try using Putty instead.\n");
    linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
    /* Since the terminal doesn't support escape sequences,
     * don't use color codes in the prompt.
     */
    prompt = PROMPT_STR "> ";
#endif  // CONFIG_LOG_COLORS
  }

  /* Main loop */
  while (true) {
    /* Get a line using linenoise.
     * The line is returned when ENTER is pressed.
     */
    char* line = linenoise(prompt);
    if (line == NULL) { /* Break on EOF or error */
      break;
    }
    /* Add the command to the history if not empty*/
    if (strlen(line) > 0) {
      linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
      /* Save command history to filesystem */
      linenoiseHistorySave(HISTORY_PATH);
#endif
    }

    /* Try to run the command */
    int ret;
    esp_err_t err = esp_console_run(line, &ret);
    if (err == ESP_ERR_NOT_FOUND) {
      printf("Unrecognized command\n");
    } else if (err == ESP_ERR_INVALID_ARG) {
      // command was empty
    } else if (err == ESP_OK && ret != ESP_OK) {
      printf("Command returned non-zero error code: 0x%x (%s)\n", ret,
             esp_err_to_name(ret));
    } else if (err != ESP_OK) {
      printf("Internal error: %s\n", esp_err_to_name(err));
    }
    /* linenoise allocates line buffer on the heap, so need to free it */
    linenoiseFree(line);
  }

  ESP_LOGE(TAG, "Error or end-of-input, terminating console");
  esp_console_deinit();
}
