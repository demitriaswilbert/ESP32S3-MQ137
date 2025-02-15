#pragma once

#include <Arduino.h>
#include <vector>
#include "main.h"
#include "rgb_led.h"

extern String rx_str;

/**
 * @brief command structure type
 *
 * consists of prefix string, parameter length, and function pointer
 *
 */
typedef struct
{
    String prefix;
    size_t param_len;
    void (*func)(String in_str);
} command_t;

/**
 * @brief check if a character is an end character
 *
 * @param enable
 * @param c
 * @return true
 * @return false
 */
inline bool isEnd(bool enable, char c)
{
    return (enable && (c == '\n' || c == '\r')) || c == '\033';
}

/**
 * @brief check input string and execute suitable command
 *
 * @param in input string to check
 * @param is_esp_now send log messages to esp-now
 * @param ctl pointer to command struct
 * @return int
 */
int process_command(String &in, command_t *ctl);

/**
 * @brief Process received character
 *
 * @param c char to process
 * @param is_esp_now send log messages to esp-now
 */
void process_char(Stream &in, char c);

/**
 * @brief wrapper function for process_char
 *
 * @param buf string buffer to process
 * @param is_esp_now send log messages to esp-now
 */
void process_string(Stream &in, const char *buf);