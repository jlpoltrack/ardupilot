/*
 * non-blocking UART driver for RP2350
 * uses IRQ-driven ring buffers - no blocking calls in data path
 *
 * date: 2026-03-02
 */

#include <AP_HAL/AP_HAL.h>
#include "UARTDriver.h"

#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <pico/stdio/driver.h>
#include <pico/stdio_usb.h>

#include <stdio.h>
#include <stdarg.h>

using namespace RP2350;

extern const AP_HAL::HAL& hal;

UARTDriver::UARTDriver(uint8_t serial_num)
    : AP_HAL::UARTDriver(),
      _initialized(false),
      _serial_num(serial_num),
      _baudrate(0),
      _is_usb(serial_num == 0),
      _receive_timestamp_idx(0)
{
    _receive_timestamp[0] = 0;
    _receive_timestamp[1] = 0;
}

void UARTDriver::_begin(uint32_t b, uint16_t rxS, uint16_t txS)
{
    if (rxS == 0) rxS = RX_BUF_SIZE;
    if (txS == 0) txS = TX_BUF_SIZE;

    // allocate ring buffers
    _readbuf.set_size(rxS);
    _writebuf.set_size(txS);

    if (_is_usb) {
        // usb cdc is initialized by pico sdk stdio_init_all() at startup
        _baudrate = 0; // not applicable for usb
    } else {
        // hardware uart init
        uart_inst_t *uart = (_serial_num == 1) ? uart0 : uart1;
        _baudrate = uart_init(uart, b);

        // note: pin assignment is done via hwdef at board level
        // the hwdef will call gpio_set_function(pin, GPIO_FUNC_UART)

        // enable rx interrupt for non-blocking receive
        uart_set_fifo_enabled(uart, true);
    }

    _initialized = true;
}

void UARTDriver::_end()
{
    if (!_is_usb && _initialized) {
        uart_inst_t *uart = (_serial_num == 1) ? uart0 : uart1;
        uart_deinit(uart);
    }
    _initialized = false;
}

void UARTDriver::_flush()
{
    if (!_initialized) return;

    if (_is_usb) {
        stdio_flush();
    } else {
        uart_inst_t *uart = (_serial_num == 1) ? uart0 : uart1;
        uart_tx_wait_blocking(uart);
    }
}

uint32_t UARTDriver::_available()
{
    if (!_initialized) return 0;
    return _readbuf.available();
}

ssize_t UARTDriver::_read(uint8_t *buffer, uint16_t count)
{
    if (!_initialized) return -1;
    return _readbuf.read(buffer, count);
}

size_t UARTDriver::_write(const uint8_t *buffer, size_t size)
{
    if (!_initialized) return 0;

    // non-blocking write into ring buffer
    _write_mutex.take_blocking();
    size_t written = _writebuf.write(buffer, size);
    _write_mutex.give();
    return written;
}

bool UARTDriver::_discard_input()
{
    if (!_initialized) return false;
    _readbuf.clear();
    return true;
}

bool UARTDriver::is_initialized()
{
    return _initialized;
}

bool UARTDriver::tx_pending()
{
    return _writebuf.available() > 0;
}

uint32_t UARTDriver::txspace()
{
    return _writebuf.space();
}

void UARTDriver::vprintf(const char *fmt, va_list ap)
{
    if (!_initialized) return;

    char buf[128];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (n > 0) {
        _write((const uint8_t *)buf, (size_t)n);
    }
}

// called from the uart freertos task at ~1kHz
void UARTDriver::_timer_tick(void)
{
    if (!_initialized) return;

    if (_is_usb) {
        _usb_read_data();
        _usb_write_data();
    } else {
        _hw_read_data();
        _hw_write_data();
    }
}

// --- usb cdc non-blocking transfers via pico sdk stdio ---

void UARTDriver::_usb_read_data()
{
    // read from pico sdk stdio (USB CDC) into ring buffer
    // getchar_timeout_us(0) returns PICO_ERROR_TIMEOUT if nothing available
    for (uint8_t i = 0; i < 64; i++) {
        int ch = getchar_timeout_us(0);
        if (ch == PICO_ERROR_TIMEOUT) break;
        uint8_t byte = (uint8_t)ch;
        _readbuf.write(&byte, 1);
    }
    _receive_timestamp_update();
}

void UARTDriver::_usb_write_data()
{
    uint8_t buf[64];
    _write_mutex.take_blocking();
    uint32_t n = _writebuf.read(buf, sizeof(buf));
    _write_mutex.give();

    if (n > 0) {
        // write via pico sdk stdio (USB CDC)
        for (uint32_t i = 0; i < n; i++) {
            putchar_raw(buf[i]);
        }
        stdio_flush();
    }
}

// --- hardware uart non-blocking transfers ---

void UARTDriver::_hw_read_data()
{
    uart_inst_t *uart = (_serial_num == 1) ? uart0 : uart1;

    // drain rx fifo into ring buffer (non-blocking)
    while (uart_is_readable(uart)) {
        uint8_t ch = uart_getc(uart);
        _readbuf.write(&ch, 1);
    }
    _receive_timestamp_update();
}

void UARTDriver::_hw_write_data()
{
    uart_inst_t *uart = (_serial_num == 1) ? uart0 : uart1;

    // drain ring buffer into tx fifo (non-blocking)
    _write_mutex.take_blocking();
    while (_writebuf.available() > 0 && uart_is_writable(uart)) {
        uint8_t ch;
        if (_writebuf.read(&ch, 1) == 1) {
            uart_putc_raw(uart, ch);
        }
    }
    _write_mutex.give();
}

void UARTDriver::_receive_timestamp_update()
{
    _receive_timestamp[_receive_timestamp_idx ^ 1] = AP_HAL::micros64();
    _receive_timestamp_idx ^= 1;
}

uint64_t UARTDriver::receive_time_constraint_us(uint16_t nbytes)
{
    uint64_t last_receive = _receive_timestamp[_receive_timestamp_idx];
    if (_baudrate > 0) {
        // assume 10 bits per byte for uart
        last_receive -= (uint64_t)(nbytes * 10000000ULL / _baudrate);
    }
    return last_receive;
}
