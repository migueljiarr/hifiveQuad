// See LICENSE for license details.

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include <string.h>
#include "plic/plic_driver.h"
#include "fe300prci/fe300prci_driver.h"
#include "encoding.h"
#include <unistd.h>
#include <inttypes.h>
#include "hifive1.h"  //RTC_FREQ, pin offsets, etc.
#include "i2c_driver.h"
#include "bno055_driver.h"

void reset_demo (void);

// Structures for registering different interrupt handlers
// for different parts of the application.
typedef void (*function_ptr_t) (void);

void no_interrupt_handler (void) {}

function_ptr_t g_ext_interrupt_handlers[PLIC_NUM_INTERRUPTS];


// Instance data for the PLIC.

plic_instance_t g_plic;

void print_msg(char *msg) {
  write (STDOUT_FILENO, msg, strlen(msg));
}

void exInt(){
    print_msg("external interrupt!\n");

    // Reset interrupt.
    GPIO_REG(GPIO_RISE_IP) = (0x1 << PIN_11_OFFSET);
}

/*Entry Point for PLIC Interrupt Handler*/
void handle_m_ext_interrupt(){
  plic_source int_num  = PLIC_claim_interrupt(&g_plic);
  if ((int_num >=1 ) && (int_num < PLIC_NUM_INTERRUPTS)) {
    g_ext_interrupt_handlers[int_num]();
  }
  else {
    exit(1 + (uintptr_t) int_num);
  }
  PLIC_complete_interrupt(&g_plic, int_num);
}


/*Entry Point for Machine Timer Interrupt Handler*/
void handle_m_time_interrupt(){

  clear_csr(mie, MIP_MTIP);

  // Reset the timer for 3s in the future.
  // This also clears the existing timer interrupt.

  volatile uint64_t * mtime       = (uint64_t*) (CLINT_BASE_ADDR + CLINT_MTIME);
  volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_BASE_ADDR + CLINT_MTIMECMP);
  uint64_t now = *mtime;
  uint64_t then = now + 3 * RTC_FREQ;
  *mtimecmp = then;

  GPIO_REG(GPIO_OUTPUT_VAL) ^= ((0x1 << RED_LED_OFFSET)   |
		     (0x1 << GREEN_LED_OFFSET) |
		     (0x1 << BLUE_LED_OFFSET));

  // Re-enable the timer interrupt.
  set_csr(mie, MIP_MTIP);
  print_msg("time interrupt!\n");
}

void reset_demo (){

  // Disable the machine & timer interrupts until setup is done.

  clear_csr(mie, MIP_MEIP);
  clear_csr(mie, MIP_MTIP);

  uint32_t cpu_freq = PRCI_measure_mcycle_freq(10000, RTC_FREQ);
  char s_cpu_freq[11];
  sprintf(s_cpu_freq,"%"PRIu32,cpu_freq);
  print_msg("cpu_freq: ");
  print_msg(s_cpu_freq);
  print_msg("\n");

  for (int ii = 0; ii < PLIC_NUM_INTERRUPTS; ii ++){
    g_ext_interrupt_handlers[ii] = no_interrupt_handler;
  }
  
  //g_ext_interrupt_handlers[INT_GPIO_BASE + PIN_11_OFFSET] = exInt;

  //PLIC_enable_interrupt(&g_plic,INT_GPIO_BASE + PIN_11_OFFSET);
  //PLIC_set_priority(&g_plic, INT_GPIO_BASE + PIN_11_OFFSET, 1);
  //GPIO_REG(GPIO_RISE_IE) |= (1 << PIN_11_OFFSET);

  // Set the machine timer to go off in 3 seconds.
  volatile uint64_t * mtime       = (uint64_t*) (CLINT_BASE_ADDR + CLINT_MTIME);
  volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_BASE_ADDR + CLINT_MTIMECMP);
  uint64_t now = *mtime;
  uint64_t then = now + 3*RTC_FREQ;
  *mtimecmp = then;

  // Enable the Machine-External bit in MIE
  set_csr(mie, MIP_MEIP);

  // Enable the Machine-Timer bit in MIE
  set_csr(mie, MIP_MTIP);

  // Enable interrupts in general.
  set_csr(mstatus, MSTATUS_MIE);
}

// BEGIN TEST I2C_DRIVER.
void test_delay(uint32_t millis){
  print_msg("TEST: spin_delay: ");
  print_msg("starting ");
  millis *= 1000; // input of spin_delay in microseconds.
  uint32_t start_mtime = CLINT_REG(CLINT_MTIME);

  // Don't start measuring until mtime edge.
  uint32_t tmp = start_mtime;
  do {
    start_mtime = CLINT_REG(CLINT_MTIME);
  } while (start_mtime == tmp);

  spin_delay(millis);

  uint32_t end_mtime = CLINT_REG(CLINT_MTIME);

  uint32_t dif = end_mtime - start_mtime;
  dif *= 1000;  // Output in milliseconds.
  dif /= RTC_FREQ;
  char s_dif[22];
  sprintf(s_dif,"%"PRIu32,dif);
  print_msg(s_dif);
  print_msg("\n");
}

void test_setup_pins(void){
  print_msg("TEST: setup_pins: ");
  if((GPIO_REG(GPIO_INPUT_EN) & (1<<PIN_19_OFFSET | 1<<PIN_18_OFFSET)) > 0){
    if((GPIO_REG(GPIO_OUTPUT_EN) & (1<<PIN_19_OFFSET | 1<<PIN_18_OFFSET)) > 0){
      print_msg("ok");
    }
    else
      print_msg("no ok");
  }
  else
    print_msg("no ok");
  print_msg("\n");
}

void test_sda(uint32_t expected){
  print_msg("TEST: sda: ");
  if(((GPIO_REG(GPIO_INPUT_VAL) & (1<<PIN_18_OFFSET))>>(PIN_18_OFFSET)) == expected){
      print_msg("ok");
  }
  else
    print_msg("no ok");
  print_msg("\n");
}

void test_read_sda(){
  print_msg("TEST: read_sda: ");
  if(((GPIO_REG(GPIO_OUTPUT_VAL) & (1<<PIN_18_OFFSET))>>(PIN_18_OFFSET)) == read_sda()){
      print_msg("ok");
  }
  else
    print_msg("no ok");
  print_msg("\n");
}

void test_scl(uint32_t expected){
  print_msg("TEST: scl: ");
  if(((GPIO_REG(GPIO_INPUT_VAL) & (1<<PIN_19_OFFSET))>>(PIN_19_OFFSET)) == expected){
      print_msg("ok");
  }
  else
    print_msg("no ok");
  print_msg("\n");
}

void test_read_scl(){
  print_msg("TEST: read_scl: ");
  if(((GPIO_REG(GPIO_OUTPUT_VAL) & (1<<PIN_19_OFFSET))>>(PIN_19_OFFSET)) == read_scl()){
      print_msg("ok");
  }
  else
    print_msg("no ok");
  print_msg("\n");
}

void test_read_at_address(){
  uint32_t aux;
  char s_aux[22];
  print_msg("TEST: read_at_address 0x0. Expected 0xA0: ");
  if((aux = i2c_read_at_address(0x0)) == 0xA0){
    print_msg("ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  else{
    print_msg("no ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_n_at_address(){
  uint32_t aux;
  char s_aux[22];
  uint8_t buf[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
  print_msg("TEST: read_n_at_address: ");
  aux = i2c_read_n_at_address(8,0x00,buf);
  if(!aux){
    print_msg("ok. ");
  }
  else{
    print_msg("no ok. ");
    sprintf(s_aux,"%"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
  print_msg("\tExpected: 0xA0, 0xFB, 0x32, 0x0F, 0x11, 0x03, 0xX, 0x00.");
  print_msg("\n");
  print_msg("\tReceived: ");
  for(int i=0;i<8;i++){
    sprintf(s_aux,"%x"PRIu32,buf[i]);
    print_msg(s_aux);
    print_msg("   ");
  }
  print_msg("\n");
}

void test_write_at_address(){
  uint32_t aux, aux2;
  char s_aux[22];
  print_msg("TEST: write_at_address 0x07: ");
  print_msg("\n");
  print_msg("\tPrevious value: ");
  aux = i2c_read_at_address(0x07);
  sprintf(s_aux,"%"PRIu32,aux);
  print_msg(s_aux);
  print_msg("\n");

  aux2 = i2c_write_data_at_address((!aux)&0x01, 0x07);
  if(!aux2){
    print_msg("\tWritten ok. ");
    sprintf(s_aux,"%"PRIu32,aux2);
    print_msg(s_aux);
  }
  else{
    print_msg("\tWritten no ok. ");
    sprintf(s_aux,"%"PRIu32,aux2);
    print_msg(s_aux);
  }
  print_msg("\n");

  print_msg("\tCurrent value: ");
  aux2 = i2c_read_at_address(0x07);
  sprintf(s_aux,"%"PRIu32,aux2);
  print_msg(s_aux);

  if(aux==(!(aux2)&0x01))
    print_msg(" OK.");
  else
    print_msg(" NO OK.");

  print_msg("\n");
}
// END TEST I2C_DRIVER.

// BEGIN TEST BNO055_DRIVER.
void test_read_PAGE_ID(){
  uint32_t aux;
  char s_aux[22];
  print_msg("TEST: test_read_PAGE_ID: ");
  if((aux = read_PAGE_ID()) < 0x02){
    print_msg("ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  else{
    print_msg("no ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_set_PAGE_ID(){
  uint32_t aux, aux2;
  char s_aux[22];
  print_msg("TEST: set_PAGE_ID: ");
  print_msg("\n");
  print_msg("\tPrevious value: ");
  aux = read_PAGE_ID();
  sprintf(s_aux,"%"PRIu32,aux);
  print_msg(s_aux);
  print_msg("\n");

  aux2 = set_PAGE_ID((!aux)&0x01);
  if(!aux2){
    print_msg("\tWritten ok. ");
    sprintf(s_aux,"%"PRIu32,aux2);
    print_msg(s_aux);
  }
  else{
    print_msg("\tWritten no ok. ");
    sprintf(s_aux,"%"PRIu32,aux2);
    print_msg(s_aux);
  }
  print_msg("\n");

  print_msg("\tCurrent value: ");
  aux2 = read_PAGE_ID();
  sprintf(s_aux,"%"PRIu32,aux2);
  print_msg(s_aux);

  if(aux==(!(aux2)&0x01))
    print_msg(" OK.");
  else
    print_msg(" NO OK.");

  print_msg("\n");
}

void test_read_OPR_MODE(){
  uint32_t aux;
  char s_aux[22];
  print_msg("TEST: test_read_OPR_MODE: ");
  if((aux = read_OPR_MODE()) < 0x0D){
    print_msg("ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  else{
    print_msg("no ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_set_OPR_MODE(){
  uint32_t aux;
  char s_aux[22];
  print_msg("TEST: test_write_OPR_MODE: ");
  print_msg("\n");

  print_msg("\tCONFIG: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_CONFIG);
  if(aux!=OPR_MODE_CONFIG){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();

  print_msg("\tAMG: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_AMG);
  if(aux!=OPR_MODE_AMG){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();

  print_msg("\tCONFIG: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_CONFIG);
  if(aux!=OPR_MODE_CONFIG){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();

  print_msg("\tIMU: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_IMU);
  if(aux!=OPR_MODE_IMU){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();

  print_msg("\tCONFIG: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_CONFIG);
  if(aux!=OPR_MODE_CONFIG){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();

  print_msg("\tNDOF: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_NDOF);
  if(aux!=OPR_MODE_NDOF){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();

  print_msg("\tCONFIG: ");
  print_msg("\n");
  aux = set_OPR_MODE(OPR_MODE_CONFIG);
  if(aux!=OPR_MODE_CONFIG){
    print_msg("\tFAIL\n");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  test_read_OPR_MODE();
  print_msg("TEST: test_write_OPR_MODE: PASS\n");
}

void test_calibration(){
  print_msg("TEST: test_calibration: ");
  if(calibrate()>0)
    print_msg("Calibrated!");
  else
    print_msg("An error ocurred while calibrating.");
  print_msg("\n");
}

void test_read_ACC_OFFSET(){
  uint32_t aux;
  char s_aux[22];
  uint8_t acc_data[6]={0xff,0xff,0xff,0xff,0xff,0xff};
  print_msg("TEST: test_read_ACC_OFFSET: ");
  if((aux = read_ACC_OFFSET(&acc_data[0])) > 0){
    print_msg("PASS: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
    print_msg("\n");
    print_msg("values: ");
    for(int i=0;i<6;i++){
      sprintf(s_aux,"%x"PRIu32,acc_data[i]);
      print_msg(s_aux);
      print_msg("  ");
    }
  }
  else{
    print_msg("FAIL: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_MAG_OFFSET(){
  uint32_t aux;
  char s_aux[22];
  uint8_t mag_data[6]={0xff,0xff,0xff,0xff,0xff,0xff};
  print_msg("TEST: test_read_MAG_OFFSET: ");
  if((aux = read_MAG_OFFSET(&mag_data[0])) > 0){
    print_msg("PASS: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
    print_msg("\n");
    print_msg("values: ");
    for(int i=0;i<6;i++){
      sprintf(s_aux,"%x"PRIu32,mag_data[i]);
      print_msg(s_aux);
      print_msg("  ");
    }
  }
  else{
    print_msg("FAIL: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_GYR_OFFSET(){
  uint32_t aux;
  char s_aux[22];
  uint8_t gyr_data[6]={0xff,0xff,0xff,0xff,0xff,0xff};
  print_msg("TEST: test_read_GYR_OFFSET: ");
  if((aux = read_GYR_OFFSET(&gyr_data[0])) > 0){
    print_msg("PASS: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
    print_msg("\n");
    print_msg("values: ");
    for(int i=0;i<6;i++){
      sprintf(s_aux,"%x"PRIu32,gyr_data[i]);
      print_msg(s_aux);
      print_msg("  ");
    }
  }
  else{
    print_msg("FAIL: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_ACC_RADIUS(){
  uint32_t aux;
  char s_aux[22];
  uint8_t acc_data[2]={0xff,0xff};
  print_msg("TEST: test_read_ACC_RADIUS: ");
  if((aux = read_ACC_RADIUS(&acc_data[0])) > 0){
    print_msg("PASS: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
    print_msg("\n");
    print_msg("values: ");
    for(int i=0;i<2;i++){
      sprintf(s_aux,"%x"PRIu32,acc_data[i]);
      print_msg(s_aux);
      print_msg("  ");
    }
  }
  else{
    print_msg("FAIL: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_MAG_RADIUS(){
  uint32_t aux;
  char s_aux[22];
  uint8_t mag_data[2]={0xff,0xff};
  print_msg("TEST: test_read_MAG_RADIUS: ");
  if((aux = read_MAG_RADIUS(&mag_data[0])) > 0){
    print_msg("PASS: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
    print_msg("\n");
    print_msg("values: ");
    for(int i=0;i<2;i++){
      sprintf(s_aux,"%x"PRIu32,mag_data[i]);
      print_msg(s_aux);
      print_msg("  ");
    }
  }
  else{
    print_msg("FAIL: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_EUL_DATA(){
  uint32_t aux;
  int16_t x,y,z;
  char s_aux[22];
  uint8_t eul_data[6]={0xff,0xff,0xff,0xff,0xff,0xff};
  print_msg("TEST: test_read_EUL_DATA: ");
  if((aux = read_EUL_DATA(&eul_data[0])) > 0){
    print_msg("PASS: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
    print_msg("\n");
    print_msg("values: ");
    for(int i=0;i<6;i++){
      sprintf(s_aux,"%x"PRIu32,eul_data[i]);
      print_msg(s_aux);
      print_msg("  ");
    }
    print_msg("\n");
    get_orientation_euler(&x,&y,&z,eul_data);
    print_msg("x, y, z: ");
    print_msg("\n");

    sprintf(s_aux,"%"PRId16,x);
    print_msg(s_aux);
    print_msg(",");
    sprintf(s_aux,"%"PRId16,y);
    print_msg(s_aux);
    print_msg(",");
    sprintf(s_aux,"%"PRId16,z);
    print_msg(s_aux);
  }
  else{
    print_msg("FAIL: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

void test_read_byte(uint8_t address, uint8_t expected){
  uint32_t aux;
  char s_aux[22];
  print_msg("TEST: test_read address: ");
  sprintf(s_aux,"%x"PRIu32,address);
  print_msg(s_aux);
  print_msg(". Expected: ");
  sprintf(s_aux,"%x"PRIu32,expected);
  print_msg(s_aux);
  print_msg(". Result: ");
  if((aux = i2c_read_at_address(address)) == expected){
    print_msg("ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  else{
    print_msg("no ok: ");
    sprintf(s_aux,"%x"PRIu32,aux);
    print_msg(s_aux);
  }
  print_msg("\n");
}

/* Unused at the moment.
void test_write_byte(uint8_t address, uint8_t expected){
  uint32_t aux, aux2;
  char s_aux[22];
  print_msg("TEST: set_PAGE_ID: ");
  print_msg("\n");
  print_msg("\tPrevious value: ");
  aux = read_PAGE_ID();
  sprintf(s_aux,"%"PRIu32,aux);
  print_msg(s_aux);
  print_msg("\n");

  aux2 = set_PAGE_ID((!aux)&0x01);
  if(!aux2){
    print_msg("\tWritten ok. ");
    sprintf(s_aux,"%"PRIu32,aux2);
    print_msg(s_aux);
  }
  else{
    print_msg("\tWritten no ok. ");
    sprintf(s_aux,"%"PRIu32,aux2);
    print_msg(s_aux);
  }
  print_msg("\n");

  print_msg("\tCurrent value: ");
  aux2 = read_PAGE_ID();
  sprintf(s_aux,"%"PRIu32,aux2);
  print_msg(s_aux);

  if(aux==(!(aux2)&0x01))
    print_msg(" OK.");
  else
    print_msg(" NO OK.");

  print_msg("\n");
}
*/

// END TEST BNO055_DRIVER.

int main(void)
{
  // Set up the GPIOs such that the LED GPIO
  // can be used as both Inputs and Outputs.

  //GPIO_REG(GPIO_INPUT_EN)   |=    (1<<PIN_11_OFFSET);
  //GPIO_REG(GPIO_OUTPUT_VAL) &=   ~(1<<PIN_11_OFFSET);
  //GPIO_REG(GPIO_PULLUP_EN)  &=   ~(1<<PIN_11_OFFSET);

  //GPIO_REG(GPIO_INPUT_EN)   &= ~((0x1<< RED_LED_OFFSET) | (0x1<< GREEN_LED_OFFSET) | (0x1 << BLUE_LED_OFFSET));

  i2c_atomic_init();

  spin_delay(1000000);

  /**************************************************************************
   * Set up the PLIC
   *
   *************************************************************************/
  /*
  PLIC_init(&g_plic,
	    PLIC_BASE_ADDR,
	    PLIC_NUM_INTERRUPTS,
	    PLIC_NUM_PRIORITIES);

  reset_demo();
  */

  test_read_OPR_MODE();
  set_OPR_MODE(OPR_MODE_NDOF);
  test_read_OPR_MODE();
  test_calibration();

  test_read_ACC_OFFSET();
  test_read_MAG_OFFSET();
  test_read_GYR_OFFSET();
  test_read_ACC_RADIUS();
  test_read_MAG_RADIUS();
  for(int i=0;i<100;i++){
    test_read_EUL_DATA();
    spin_delay(200000);
  }

/*
  test_set_OPR_MODE();
*/

/*
  test_set_PAGE_ID();
  test_set_PAGE_ID();
  test_set_PAGE_ID();
  test_set_PAGE_ID();
  test_set_PAGE_ID();
*/
/*
  test_read_PAGE_ID();
  test_read_PAGE_ID();
  test_read_PAGE_ID();
  test_read_PAGE_ID();
  test_read_PAGE_ID();
*/





/*
  test_write_at_address();
  test_write_at_address();
  test_write_at_address();
  test_write_at_address();
  test_write_at_address();
  test_write_at_address();
  test_write_at_address();
*/
/*
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
  test_read_n_at_address();
*/
/*
  test_read_at_address();
  test_read_at_address();
  test_read_at_address();
  test_read_at_address();
  test_read_at_address();
  test_read_at_address();
  test_read_at_address();
  test_read_at_address();
*/

  while (1);

  return 0;

}
