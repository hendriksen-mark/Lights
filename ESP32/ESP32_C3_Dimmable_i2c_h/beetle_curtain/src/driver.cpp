#include <new>
#if defined(ARDUINO_ESP32C3_M1_I_KIT)
#include "driver.h"
#else
#include "include/driver.h"
#endif

static uint8_t driver_storage[sizeof(TMC2209Stepper)];
static TMC2209Stepper *driver = nullptr;

void configureRAdriver(uint16_t RA_SW_RX, uint16_t RA_SW_TX, float rsense, byte driveraddress, int rmscurrent, int stallvalue)
{
  driver = reinterpret_cast<TMC2209Stepper *>(driver_storage);
  // new (driver) TMC2209Stepper(RA_SW_RX, RA_SW_TX, rsense, driveraddress);
  HardwareSerial SerialDriver(Serial1);
  new (driver) TMC2209Stepper(&SerialDriver, rsense, driveraddress);
  SerialDriver.begin(19200, SERIAL_8N1, RA_SW_RX, RA_SW_TX);
  driver->begin();
  driver->mstep_reg_select(true);
  driver->pdn_disable(true);
  driver->toff(0);
#if USE_VREF == 0 // By default, Vref is ignored when using UART to specify rms current.
  driver->I_scale_analog(false);
#endif
  LOG_DEBUG("[MOUNT]: Requested RA motor rms_current:", rmscurrent, " mA");
  driver->rms_current(rmscurrent, 1.0f); // holdMultiplier = 1 to set ihold = irun
  driver->toff(1);
  driver->en_spreadCycle(UART_STEALTH_MODE == 0);
  driver->blank_time(24);
  driver->semin(0);                                                       // disable CoolStep so that current is consistent
  driver->microsteps(GUIDE_MICROSTEPPING == 1 ? 0 : GUIDE_MICROSTEPPING); // System starts in tracking mode
  driver->fclktrim(4);
  driver->TCOOLTHRS(0xFFFFF); // xFFFFF);
  driver->SGTHRS(stallvalue);
  LOG_DEBUG("[MOUNT]: Actual RA motor rms_current:", driver->rms_current(), " mA");
  LOG_DEBUG("[MOUNT]: Actual RA CS value:", driver->cs_actual());
  LOG_DEBUG("[MOUNT]: Actual RA vsense:", driver->vsense());
}
