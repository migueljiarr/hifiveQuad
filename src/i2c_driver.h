#include <platform.h>
#include <hifive1.h>

void spin_rtc_delay(uint32_t time);
void spin_delay(uint32_t time);
void i2c_init(void);
void set_sda(void);
void set_scl(void);
void clear_scl(void);
void clear_sda(void);
void i2c_atomic_init(void);
void atomic_set_sda_scl(void);
void atomic_clear_sda_scl(void);
void atomic_set_sda_clear_scl(void);
void atomic_clear_sda_set_scl(void);
uint32_t read_sda(void);
uint32_t read_scl(void);
void i2c_start(void);
void i2c_atomic_start(void);
void i2c_stop(void);
void i2c_atomic_stop(void);
void write1(uint8_t);
void atomic_write1(uint8_t);
uint8_t read1(void);
uint8_t atomic_read1(void);
void write7(uint8_t);
void atomic_write7(uint8_t);
void write8(uint8_t);
void atomic_write8(uint8_t);
uint8_t i2c_write_data_at_address(uint8_t, uint8_t);
uint8_t read8(void);
uint8_t atomic_read8(void);
uint8_t i2c_std_read_at_address(uint8_t);
uint8_t i2c_read_at_address(uint8_t);
uint8_t i2c_read_n_at_address(uint8_t, uint8_t, uint8_t[]);
