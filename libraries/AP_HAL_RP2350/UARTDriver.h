/*
 * non-blocking UART driver for RP2350 using IRQ-driven ring buffers
 *
 * date: 2026-02-28
 */
#pragma once

#include <AP_HAL/UARTDriver.h>
#include <AP_HAL/utility/RingBuffer.h>
#include "HAL_RP2350_Namespace.h"
#include "Semaphores.h"

class RP2350::UARTDriver : public AP_HAL::UARTDriver
{
public:
    // serial_num: 0 = USB CDC, 1 = uart0, 2 = uart1
    UARTDriver(uint8_t serial_num);
    virtual ~UARTDriver() = default;

    void vprintf(const char *fmt, va_list ap) override;
    bool is_initialized() override;
    bool tx_pending() override;
    uint32_t txspace() override;
    void _timer_tick(void) override;
    uint32_t bw_in_bytes_per_second() const override { return 10 * 1024; }
    uint64_t receive_time_constraint_us(uint16_t nbytes) override;
    uint32_t get_baud_rate() const override { return _baudrate; }

private:
    bool _initialized;
    uint8_t _serial_num;
    uint32_t _baudrate;

    static const size_t TX_BUF_SIZE = 512;
    static const size_t RX_BUF_SIZE = 512;

    ByteBuffer _readbuf{0};
    ByteBuffer _writebuf{0};
    Semaphore _write_mutex;

    // usb cdc support
    bool _is_usb;
    void _usb_read_data();
    void _usb_write_data();

    // hardware uart support
    void _hw_read_data();
    void _hw_write_data();

    // timestamp for receive timing
    uint64_t _receive_timestamp[2];
    uint8_t _receive_timestamp_idx;
    void _receive_timestamp_update(void);

protected:
    void _begin(uint32_t b, uint16_t rxS, uint16_t txS) override;
    void _end() override;
    void _flush() override;
    uint32_t _available() override;
    ssize_t _read(uint8_t *buffer, uint16_t count) override;
    size_t _write(const uint8_t *buffer, size_t size) override;
    bool _discard_input() override;
};
