#include <new>
#include "driver.h"

static uint8_t driver_storage[sizeof(TMC2209Stepper)];
TMC2209Stepper *driver = nullptr;

void configureRAdriver(uint16_t RA_SW_RX, uint16_t RA_SW_TX, float rsense, byte driveraddress, int rmscurrent, int stallvalue)
{
  driver = reinterpret_cast<TMC2209Stepper*>(driver_storage);
  new (driver) TMC2209Stepper(RA_SW_RX, RA_SW_TX, rsense, driveraddress);
  driver->beginSerial(19200);
  driver->mstep_reg_select(true);
  driver->pdn_disable(true);
  driver->toff(0);
#if USE_VREF == 0  //By default, Vref is ignored when using UART to specify rms current.
  driver->I_scale_analog(false);
#endif
  if (DEBUG == true)
  {
    Serial.print("[MOUNT]: Requested RA motor rms_current: "); Serial.print(rmscurrent); Serial.println(" mA");
  }
  driver->rms_current(rmscurrent, 1.0f);  //holdMultiplier = 1 to set ihold = irun
  driver->toff(1);
  driver->en_spreadCycle(UART_STEALTH_MODE == 0);
  driver->blank_time(24);
  driver->semin(0);                                                                    //disable CoolStep so that current is consistent
  driver->microsteps(GUIDE_MICROSTEPPING == 1 ? 0 : GUIDE_MICROSTEPPING);  // System starts in tracking mode
  driver->fclktrim(4);
  driver->TCOOLTHRS(0xFFFFF);  //xFFFFF);
  driver->SGTHRS(stallvalue);
  if (DEBUG == true)
  {
    Serial.print("[MOUNT]: Actual RA motor rms_current: "); Serial.print(driver->rms_current()); Serial.println(" mA");
    Serial.print("[MOUNT]: Actual RA CS value: "); Serial.println(driver->cs_actual());
    Serial.print("[MOUNT]: Actual RA vsense: "); Serial.println(driver->vsense());
  }
}
