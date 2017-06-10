#include "i2c_driver.h"

#define OPR_MODE_CONFIG 0x00
#define OPR_MODE_ACC    0x01
#define OPR_MODE_MAG    0x02
#define OPR_MODE_GYR    0x03
#define OPR_MODE_AMG    0x07
#define OPR_MODE_IMU    0x08
#define OPR_MODE_NDOF   0x0C

uint8_t read_PAGE_ID(void);
uint8_t set_PAGE_ID(uint8_t);

uint8_t read_OPR_MODE(void);
uint8_t set_OPR_MODE(uint8_t);

uint8_t check_calibration(void);
int8_t calibrate(void);

uint8_t read_ACC_OFFSET(uint8_t []);
uint8_t read_MAG_OFFSET(uint8_t []);
uint8_t read_GYR_OFFSET(uint8_t []);
uint8_t read_ACC_RADIUS(uint8_t []);
uint8_t read_MAG_RADIUS(uint8_t []);

uint8_t read_EUL_DATA(uint8_t []);
void get_orientation_euler(int16_t*,int16_t*,int16_t*,uint8_t []);
uint8_t read_QUA_DATA(uint8_t []);
void get_orientation_quaternion(int16_t*,int16_t*,int16_t*,int16_t*,uint8_t []);
uint8_t read_LIA_DATA(uint8_t []);
void get_accelerometer_linear_acc(int16_t*,int16_t*,int16_t*,uint8_t []);
uint8_t read_GRV_DATA(uint8_t []);
void get_accelerometer_gravity_vect(int16_t*,int16_t*,int16_t*,uint8_t []);
uint8_t read_TEMP_DATA(int8_t*);

uint8_t read_ACC_DATA(uint8_t []);
void get_accelerometer_data_fusion(int16_t*,int16_t*,int16_t*,uint8_t []);
uint8_t read_MAG_DATA(uint8_t []);
void get_magnetometer_data_fusion(int16_t*,int16_t*,int16_t*,uint8_t []);
uint8_t read_GYR_DATA(uint8_t []);
void get_gyrometer_data_fusion(int16_t*,int16_t*,int16_t*,uint8_t []);

uint8_t get_UNIT_SEL(int8_t *aroid_win, int8_t *c_f, int8_t *dg_rd, int8_t *dps_rps, int8_t *ms2_mg);
