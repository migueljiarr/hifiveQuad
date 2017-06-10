#include "bno055_driver.h"

uint8_t read_PAGE_ID(void){
  return i2c_read_at_address(0x07);
}

uint8_t set_PAGE_ID(uint8_t value){
  return i2c_write_data_at_address(value, 0x07);
}

uint8_t read_OPR_MODE(void){
  return i2c_read_at_address(0x3D);
}

uint8_t set_OPR_MODE(uint8_t value){
  uint8_t out = i2c_write_data_at_address(value, 0x3D);
  spin_delay(20000);
  return out;
}

uint8_t check_calibration(void){
  uint8_t out = i2c_read_at_address(0x35);
  return out;
}

int8_t calibrate(void){
  while(check_calibration() != 0xff){
    spin_delay(20000000);
  }
  return 1;
}

uint8_t read_ACC_OFFSET(uint8_t acc_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x55,acc_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

uint8_t read_MAG_OFFSET(uint8_t mag_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x5B,mag_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

uint8_t read_GYR_OFFSET(uint8_t gyr_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x61,gyr_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

uint8_t read_ACC_RADIUS(uint8_t acc_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(2,0x67,acc_data);
  if(out==2){
    return 1;
  }
  else
    return 0;
}

uint8_t read_MAG_RADIUS(uint8_t mag_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(2,0x69,mag_data);
  if(out==2){
    return 1;
  }
  else
    return 0;
}

uint8_t read_EUL_DATA(uint8_t eul_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x1A,eul_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

void get_orientation_euler(int16_t *x, int16_t *y, int16_t *z, uint8_t eul_data[]){
  int8_t lx, ly, lz;
  int8_t hx, hy, hz;

  lx=eul_data[0];
  hx=eul_data[1];
  ly=eul_data[2];
  hy=eul_data[3];
  lz=eul_data[4];
  hz=eul_data[5];

  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;

  (*x) /= 16;
  (*y) /= 16;
  (*z) /= 16;
}

// NOT TESTED.
uint8_t read_QUA_DATA(uint8_t qua_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(8,0x20,qua_data);
  if(out==8){
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
void get_orientation_quaternion(int16_t *w, int16_t *x, int16_t *y, int16_t *z, uint8_t qua_data[]){
  int8_t lw, lx, ly, lz;
  int8_t hw, hx, hy, hz;

  lw=qua_data[0];
  hw=qua_data[1];
  lx=qua_data[2];
  hx=qua_data[3];
  ly=qua_data[4];
  hy=qua_data[5];
  lz=qua_data[6];
  hz=qua_data[7];

  *w = hw<<8;
  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lw > 0)
    *w += lw;
  else
    *w -= lw;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;

  // PROBABLY HAVE TO USE FLOATS....  ==> problems
  (*w) /= (1<<14);
  (*x) /= (1<<14);
  (*y) /= (1<<14);
  (*z) /= (1<<14);
}

// NOT TESTED.
uint8_t read_LIA_DATA(uint8_t lia_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x28,lia_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
void get_accelerometer_linear_acc(int16_t *x, int16_t *y, int16_t *z, uint8_t lia_data[]){
  int8_t lx, ly, lz;
  int8_t hx, hy, hz;

  lx=lia_data[0];
  hx=lia_data[1];
  ly=lia_data[2];
  hy=lia_data[3];
  lz=lia_data[4];
  hz=lia_data[5];

  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;

  // ASUME m/s2. Have to check for all functions...
  (*x) /= 100;
  (*y) /= 100;
  (*z) /= 100;
}

// NOT TESTED.
uint8_t read_GRV_DATA(uint8_t grv_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x2E,grv_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
void get_accelerometer_gravity_vect(int16_t *x, int16_t *y, int16_t *z, uint8_t grv_data[]){
  int8_t lx, ly, lz;
  int8_t hx, hy, hz;

  lx=grv_data[0];
  hx=grv_data[1];
  ly=grv_data[2];
  hy=grv_data[3];
  lz=grv_data[4];
  hz=grv_data[5];

  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;

  // ASUME m/s2. Have to check for all functions...
  (*x) /= 100;
  (*y) /= 100;
  (*z) /= 100;
}

// NOT TESTED.
uint8_t read_TEMP_DATA(int8_t *temp){
  uint8_t out;
  out = i2c_read_at_address(0x34);
  if(out==6){
    *temp = out;
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
uint8_t read_ACC_DATA(uint8_t acc_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x08,acc_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
void get_accelerometer_data_fusion(int16_t *x, int16_t *y, int16_t *z, uint8_t acc_data[]){
  int8_t lx, ly, lz;
  int8_t hx, hy, hz;

  lx=acc_data[0];
  hx=acc_data[1];
  ly=acc_data[2];
  hy=acc_data[3];
  lz=acc_data[4];
  hz=acc_data[5];

  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;
}

// NOT TESTED.
uint8_t read_MAG_DATA(uint8_t mag_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x0E,mag_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
void get_magnetometer_data_fusion(int16_t *x, int16_t *y, int16_t *z, uint8_t mag_data[]){
  int8_t lx, ly, lz;
  int8_t hx, hy, hz;

  lx=mag_data[0];
  hx=mag_data[1];
  ly=mag_data[2];
  hy=mag_data[3];
  lz=mag_data[4];
  hz=mag_data[5];

  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;
}

// NOT TESTED.
uint8_t read_GYR_DATA(uint8_t gyr_data[]){
  uint8_t out;
  out = i2c_read_n_at_address(6,0x14,gyr_data);
  if(out==6){
    return 1;
  }
  else
    return 0;
}

// NOT TESTED.
void get_gyrometer_data_fusion(int16_t *x, int16_t *y, int16_t *z, uint8_t gyr_data[]){
  int8_t lx, ly, lz;
  int8_t hx, hy, hz;

  lx=gyr_data[0];
  hx=gyr_data[1];
  ly=gyr_data[2];
  hy=gyr_data[3];
  lz=gyr_data[4];
  hz=gyr_data[5];

  *x = hx<<8;
  *y = hy<<8;
  *z = hz<<8;

  if(lx > 0)
    *x += lx;
  else
    *x -= lx;

  if(ly > 0)
    *y += ly;
  else
    *y -= ly;

  if(lz > 0)
    *z += lz;
  else
    *z -= lz;
}

// NOT TESTED.
uint8_t get_UNIT_SEL(int8_t *aroid_win, int8_t *c_f, int8_t *dg_rd, int8_t *dps_rps, int8_t *ms2_mg){
  uint8_t out;
  out = i2c_read_at_address(0x3B);

  if(out & 0x80){
    *aroid_win=1;
  }
  else{
    *aroid_win=0;
  }

  if(out & 0x10){
    *c_f=1;
  }
  else{
    *c_f=0;
  }

  if(out & 0x04){
    *dg_rd=1;
  }
  else{
    *dg_rd=0;
  }

  if(out & 0x02){
    *dps_rps=1;
  }
  else{
    *dps_rps=0;
  }

  if(out & 0x01){
    *ms2_mg=1;
  }
  else{
    *ms2_mg=0;
  }

  if(out<-3){
    return 1;
  }
  else
    return 0;
}

