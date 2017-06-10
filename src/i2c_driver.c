#include "i2c_driver.h"

#define PROP_TIME 200
#define DELAY_TIME 400
#define WEIRD_WAIT_TIME 2000

#define I2C_MASK _AC(0x00003000,UL)
#define I2C_MASK_SDA _AC(0x00001000,UL)
#define I2C_MASK_SCL _AC(0x00002000,UL)

extern void print_msg(char *);

uint8_t started = 0;
uint8_t stopped = 0;

// Wait for time microseconds.
// Due to RTC_FREQ the minumun delay is greater than 30us.
void spin_rtc_delay(uint32_t time){
  uint32_t now = CLINT_REG(CLINT_MTIME);  
  uint32_t end_time = now + (time/1000000) * RTC_FREQ;

  // Make sure we won't get rollover.
  while (end_time < now){
    now = CLINT_REG(CLINT_MTIME);
    end_time = now + (time/1000) * RTC_FREQ;
    print_msg("Rolled over!!");
  }

  while(CLINT_REG(CLINT_MTIME) <= end_time){
    print_msg("waiting...");
  };
}

#define CPU_FREQ 279000000
// Wait for time microseconds.
// Due to CPU_FREQ the minumun delay is greater than 4ns. So, no problem in this front.
void spin_delay(uint32_t time){
  // TODO: Check this is calculating the time correctly.
  // After checking instead of dividing by 4, we divide by 58. No idea why. Memory?
  // Will check again.
  uint32_t now = 0;
  uint32_t nticks = (time * 1000) / 58;

  // Once loaded on a register now should increment at the frequency of the CPU.
  while(now <= nticks){
    now++;
  };
}

void i2c_init(void){
  GPIO_REG(GPIO_OUTPUT_EN)  &= ~((0x1<< PIN_19_OFFSET) | (0x1<< PIN_18_OFFSET));
  //GPIO_REG(GPIO_PULLUP_EN)  |=  ((0x1<< PIN_19_OFFSET) | (0x1<< PIN_18_OFFSET));
  spin_delay(DELAY_TIME);
  GPIO_REG(GPIO_INPUT_EN)   |=  ((0x1<< PIN_19_OFFSET) | (0x1<< PIN_18_OFFSET));
}

void set_sda(void){
  GPIO_REG(GPIO_OUTPUT_EN)  &= ~(0x1<< PIN_18_OFFSET);
  //GPIO_REG(GPIO_PULLUP_EN)  |=  (0x1<< PIN_18_OFFSET);
  spin_delay(DELAY_TIME);
  GPIO_REG(GPIO_INPUT_EN)   |=  (0x1<< PIN_18_OFFSET);
}

void set_scl(void){
  GPIO_REG(GPIO_OUTPUT_EN)  &= ~(0x1<< PIN_19_OFFSET);
  spin_delay(DELAY_TIME);
  //GPIO_REG(GPIO_PULLUP_EN)  |=  (0x1<< PIN_19_OFFSET);
  GPIO_REG(GPIO_INPUT_EN)   |=  (0x1<< PIN_19_OFFSET);
}

void clear_sda(void){
  //GPIO_REG(GPIO_PULLUP_EN)  &= ~(0x1<< PIN_18_OFFSET);
  spin_delay(DELAY_TIME);
  GPIO_REG(GPIO_OUTPUT_VAL) &= ~(0x1<< PIN_18_OFFSET);
  GPIO_REG(GPIO_OUTPUT_EN)  |=  (0x1<< PIN_18_OFFSET);
}

void clear_scl(void){
  //GPIO_REG(GPIO_PULLUP_EN)  &= ~(0x1<< PIN_19_OFFSET);
  spin_delay(DELAY_TIME);
  GPIO_REG(GPIO_OUTPUT_VAL) &= ~(0x1<< PIN_19_OFFSET);
  GPIO_REG(GPIO_OUTPUT_EN)  |=  (0x1<< PIN_19_OFFSET);
}

void i2c_atomic_init(void){
  GPIO_REG(GPIO_OUTPUT_EN)  =  (0x0);
  //GPIO_REG(GPIO_PULLUP_EN)  =  (I2C_MASK);
  GPIO_REG(GPIO_INPUT_EN)   =  (I2C_MASK);
}

void atomic_set_sda_scl(void){
  GPIO_REG(GPIO_OUTPUT_EN)  =  (0x0);
  //GPIO_REG(GPIO_PULLUP_EN)  =  (I2C_MASK);
  GPIO_REG(GPIO_INPUT_EN)   =  (I2C_MASK);
}

void atomic_clear_sda_scl(void){
  GPIO_REG(GPIO_OUTPUT_VAL) =  (0x0);
  GPIO_REG(GPIO_OUTPUT_EN)  =  (I2C_MASK);
  GPIO_REG(GPIO_INPUT_EN)   =  (I2C_MASK);
}

void atomic_set_sda_clear_scl(void){
  GPIO_REG(GPIO_OUTPUT_VAL) =  (0x0);
  GPIO_REG(GPIO_OUTPUT_EN)  =  (I2C_MASK_SCL);
  GPIO_REG(GPIO_INPUT_EN)   =  (I2C_MASK);
}

void atomic_clear_sda_set_scl(void){
  GPIO_REG(GPIO_OUTPUT_VAL) =  (0x0);
  GPIO_REG(GPIO_OUTPUT_EN)  =  (I2C_MASK_SDA);
  GPIO_REG(GPIO_INPUT_EN)   =  (I2C_MASK);
}

uint32_t read_sda(void){
  if((GPIO_REG(GPIO_INPUT_VAL) & (0x1<< PIN_18_OFFSET)) > 0)
    return 1;
  else
    return 0;
}

uint32_t read_scl(void){
  if((GPIO_REG(GPIO_INPUT_VAL) & (0x1<< PIN_19_OFFSET)) > 0)
    return 1;
  else
    return 0;
}

void i2c_start(void) {
  // This is the default (idle) state of the buses due to the pull up resistors.
  set_sda();
  set_scl();

  // While SCL is remains high, we deliberably pull SDA to 0.
  spin_delay(DELAY_TIME);
  clear_sda();
  spin_delay(DELAY_TIME);

  // Make SCL Low for data transmission.
  clear_scl();
  started = 1;
}

void i2c_atomic_start(void){
  // This is the default (idle) state of the buses due to the pull up resistors.
  atomic_set_sda_scl();

  // While SCL is remains high, we deliberably pull SDA to 0.
  spin_delay(DELAY_TIME);
  atomic_clear_sda_set_scl();
  spin_delay(DELAY_TIME);

  // Make SCL Low for data transmission.
  atomic_clear_sda_scl();
  started = 1;
}

void i2c_stop(void) {
  // Bring SDA down so that we can stop and let go of SCL.
  clear_sda();
  set_scl();
  spin_delay(DELAY_TIME);

  // SCL is remains high and we let go of SDA to 1.
  set_sda();
  spin_delay(DELAY_TIME);

  // The buses are idle now.
  stopped = 1;
}

void i2c_atomic_stop(void){
  // Bring SDA down so that we can stop and let go of SCL.
  atomic_clear_sda_set_scl();
  spin_delay(DELAY_TIME);

  // SCL is remains high and we let go of SDA to 1.
  atomic_set_sda_scl();
  spin_delay(DELAY_TIME);

  // The buses are idle now.
  stopped = 1;
}

void write1(uint8_t bit){
  // Make sure SCL is low for transmission.
  clear_scl();

  if(bit){
    set_sda();
  }
  else{
    clear_sda();
  }

  spin_delay(PROP_TIME);

  set_scl();
  spin_delay(PROP_TIME);

  spin_delay(DELAY_TIME);

  // End transmission.
  clear_scl();
  
  // Release control of SDA.
  set_sda();
}

void atomic_write1(uint8_t bit){
  // Make sure SCL is low for transmission.
  atomic_set_sda_clear_scl();

  if(bit){
    atomic_set_sda_scl();
    spin_delay(PROP_TIME);
    spin_delay(DELAY_TIME);

    // End transmission.
    atomic_set_sda_clear_scl();
    // Release control of SDA.
  }
  else{
    atomic_clear_sda_scl();
    spin_delay(DELAY_TIME);
    atomic_clear_sda_set_scl();
    spin_delay(PROP_TIME);
    spin_delay(DELAY_TIME);
    // End transmission.
    atomic_clear_sda_scl();
    spin_delay(DELAY_TIME);
    // Release control of SDA.
    atomic_set_sda_clear_scl();
  }
}

uint8_t read1(){
  uint8_t bit, ck;

  clear_scl();
  spin_delay(DELAY_TIME);

  set_scl();
  // Wait until high clock. Maybe there's clock streching.
  while(!(ck = read_scl()));

  spin_delay(DELAY_TIME);
  bit = read_sda();

  // Wait until end of transmission (0).
/*
  // Needed? I have data, so stop?
  while((ck = read_scl()));
*/
  clear_scl();
  set_sda();
  return bit;
}

uint8_t atomic_read1(void){
  uint8_t bit, ck;

  atomic_set_sda_clear_scl();
  spin_delay(DELAY_TIME);

  atomic_set_sda_scl();
  // Wait until high clock. Maybe there's clock streching.
  while(!(ck = read_scl()));

  spin_delay(DELAY_TIME);
  bit = read_sda();

  // Wait until end of transmission (0).
/*
  // Needed? I have data, so stop?
  while((ck = read_scl()));
*/
  atomic_set_sda_clear_scl();
  return bit;
}

void write7(uint8_t byte){
  uint8_t bit;
  
  byte <<= 1;
  for (bit = 0; bit < 7; bit++) {
    write1((byte & 0x80));
    byte <<= 1;
  }
}

void atomic_write7(uint8_t byte){
  uint8_t bit;
  
  byte <<= 1;
  for (bit = 0; bit < 7; bit++) {
    atomic_write1((byte & 0x80));
    byte <<= 1;
  }
}

void write8(uint8_t byte){
  uint8_t bit;

  for (bit = 0; bit < 8; bit++) {
    write1((byte & 0x80));
    byte <<= 1;
  }
}

void atomic_write8(uint8_t byte){
  uint8_t bit;

  for (bit = 0; bit < 8; bit++) {
    atomic_write1((byte & 0x80));
    byte <<= 1;
  }
}

uint8_t i2c_write_data_at_address(uint8_t data, uint8_t reg){
  uint8_t nack;

  i2c_atomic_start();

  atomic_write7(0x28);
  atomic_write1(0);
  nack = atomic_read1();
  if(nack)
    return -1;

  atomic_write8(reg);
  nack = atomic_read1();
  if(nack)
    return -2;

  atomic_write8(data);
  nack = atomic_read1();
  if(nack)
    return -3;
  
  spin_delay(WEIRD_WAIT_TIME);
  i2c_atomic_stop();

  return data;
}

uint8_t read8(){
  uint8_t bit, byte=0, aux;

  for (bit = 0; bit < 8; bit++) {
    aux   = read1();
    byte |= aux;
    byte <<= 1;
  }
  return byte;
}

uint8_t atomic_read8(){
  uint8_t bit, byte=0x0, aux;

  for (bit = 0; bit < 7; bit++) {
    aux   = atomic_read1();
    byte |= aux;
    byte <<= 1;
  }
  aux   = atomic_read1();
  byte |= aux;
  return byte;
}

uint8_t i2c_std_read_at_address(uint8_t reg){
  uint8_t nack, byte;
  i2c_start();

  write7(0x29);
  write1(1);
  nack = read1();
  if(nack)
    return -1;

  write8(reg);
  nack = read1();
  if(nack)
    return -1;

  byte=read8();
  write1(0);

  i2c_stop();
  return byte;
}

uint8_t i2c_read_at_address(uint8_t reg){
  uint8_t nack, byte;
  i2c_atomic_start();

  atomic_write7(0x28);
  atomic_write1(0);
  nack = atomic_read1();
  if(nack)
    return -1;

  atomic_write8(reg);
  nack = atomic_read1();
  if(nack)
    return -2;

  i2c_atomic_start();

  atomic_write7(0x28);
  atomic_write1(1);
  nack = atomic_read1();
  if(nack)
    return -3;

  byte=atomic_read8();
  atomic_write1(1); // NACK, so that the read stops.

  spin_delay(WEIRD_WAIT_TIME);
  i2c_atomic_stop();
  return byte;
}

uint8_t i2c_read_n_at_address(uint8_t n, uint8_t reg, uint8_t buf[]){
  uint8_t nack, byte, i=0;
  i2c_atomic_start();

  atomic_write7(0x28);
  atomic_write1(0);
  nack = atomic_read1();
  if(nack)
    return -1;

  atomic_write8(reg);
  nack = atomic_read1();
  if(nack)
    return -2;

  i2c_atomic_start();

  atomic_write7(0x28);
  atomic_write1(1);
  nack = atomic_read1();
  if(nack)
    return -3;

  for(i=0;i<n-1;i++){ // -1 because the last read musn't be ACKed.
    byte=atomic_read8();
    atomic_write1(0); // ACK, so that the read continues.
    buf[i]=byte;
  }
  byte=atomic_read8();
  atomic_write1(1); // NACK, so that the read stops.
  buf[i]=byte;

  spin_delay(WEIRD_WAIT_TIME);
  i2c_atomic_stop();
  //while(!read_sda());

  //spin_delay(20000);
  return i+1;
}
