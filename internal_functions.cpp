#include <Arduino.h>

#include <cstdint>

#define CHECK_ARGS(n)                             \
    do                                            \
    {                                             \
        if (argc < n)                             \
        {                                         \
            sifault(sinter_fault_function_arity); \
        }                                         \
    } while (0)

static const sivmfnptr_t internals[] = {
    digital_read,
    digital_write,
    pin_mode,
    analog_read,
    analog_reference,
    analog_write,
    fn_delay,
    delay_us,
    fn_micros,
    fn_millis,
    attach_interrupt,
    detach_interrupt,
    enable_interrupts,
    disable_interrupts,
    serial_begin,
    serial_end,
    serial_settimeout,
    serial_print,
    serial_println,
    serial_read,
    serial_write,
    serial_flush};

void setupInternals()
{
    sivmfn_vminternals = internals;
    sivmfn_vminternal_count = sizeof(internals) / sizeof(*internals);
}