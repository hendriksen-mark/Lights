#include <new>
#if defined(ESP_PLATFORM)
#include "driver.h"
#else
#include "include/driver.h"
#endif

static TMC2209Stepper *driver1 = nullptr;
static TMC2209Stepper *driver2 = nullptr;

void configureDualDrivers()
{
  // Initialize shared UART serial
  static HardwareSerial SerialDriver(1);
  SerialDriver.begin(19200, SERIAL_8N1, SW_RX, SW_TX);

  // Configure Driver 1 (Left Motor)
  if (driver1 == nullptr) {
    driver1 = new TMC2209Stepper(&SerialDriver, R_SENSE, DRIVER_ADDRESS_1);
  }
  driver1->begin();
  driver1->mstep_reg_select(true);
  driver1->pdn_disable(true);
  driver1->toff(0);
#if USE_VREF == 0
  driver1->I_scale_analog(false);
#endif
  LOG_DEBUG("[DRIVER1]: Requested motor rms_current:", R_CURRENT, " mA");
  driver1->rms_current(R_CURRENT, 1.0f);
  driver1->toff(1);
  driver1->en_spreadCycle(UART_STEALTH_MODE == 0);
  driver1->blank_time(24);
  driver1->semin(0);
  driver1->microsteps(GUIDE_MICROSTEPPING == 1 ? 0 : GUIDE_MICROSTEPPING);
  driver1->fclktrim(4);
  driver1->TCOOLTHRS(0xFFFFF);
  driver1->SGTHRS(STALL_VALUE);
  LOG_DEBUG("[DRIVER1]: Actual motor rms_current:", driver1->rms_current(), " mA");
  LOG_DEBUG("[DRIVER1]: Actual CS value:", driver1->cs_actual());
  LOG_DEBUG("[DRIVER1]: Actual vsense:", driver1->vsense());

  // Configure Driver 2 (Right Motor)
  if (driver2 == nullptr) {
    driver2 = new TMC2209Stepper(&SerialDriver, R_SENSE, DRIVER_ADDRESS_2);
  }
  driver2->begin();
  driver2->mstep_reg_select(true);
  driver2->pdn_disable(true);
  driver2->toff(0);
#if USE_VREF == 0
  driver2->I_scale_analog(false);
#endif
  LOG_DEBUG("[DRIVER2]: Requested motor rms_current:", R_CURRENT, " mA");
  driver2->rms_current(R_CURRENT, 1.0f);
  driver2->toff(1);
  driver2->en_spreadCycle(UART_STEALTH_MODE == 0);
  driver2->blank_time(24);
  driver2->semin(0);
  driver2->microsteps(GUIDE_MICROSTEPPING == 1 ? 0 : GUIDE_MICROSTEPPING);
  driver2->fclktrim(4);
  driver2->TCOOLTHRS(0xFFFFF);
  driver2->SGTHRS(STALL_VALUE);
  LOG_DEBUG("[DRIVER2]: Actual motor rms_current:", driver2->rms_current(), " mA");
  LOG_DEBUG("[DRIVER2]: Actual CS value:", driver2->cs_actual());
  LOG_DEBUG("[DRIVER2]: Actual vsense:", driver2->vsense());
}
