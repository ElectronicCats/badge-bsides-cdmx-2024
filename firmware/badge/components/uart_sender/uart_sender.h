#pragma once
#include <stdint.h>
#ifndef UART_SENDER
  #define UART_SENDER

/**
 * @brief Send a UART packet
 *
 * @param packet packet to send
 * @param len length of the packet
 * @return void
 */
void uart_sender_send_packet(uint8_t* packet, uint8_t len);

/**
 * @brief DeInitialize the UART sender
 *
 * @return void
 */
void uart_sender_deinit();
#endif  // UART_SENDER
