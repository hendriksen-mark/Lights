#if defined(ESP_PLATFORM)
#include "driver.h"
#else
#include "include/driver.h"
#endif

static TMC2209 driver1;
static TMC2209 driver2;
static HardwareSerial &serial_stream = Serial1;

// Convert DRIVER_ADDRESS defines to TMC2209::SerialAddress enum
TMC2209::SerialAddress getSerialAddress(uint8_t addr) {
  switch(addr) {
    case 0b00: return TMC2209::SERIAL_ADDRESS_0;
    case 0b01: return TMC2209::SERIAL_ADDRESS_1;
    case 0b10: return TMC2209::SERIAL_ADDRESS_2;
    case 0b11: return TMC2209::SERIAL_ADDRESS_3;
    default: return TMC2209::SERIAL_ADDRESS_0;
  }
}

void configureDualDrivers()
{
  // Initialize shared UART serial
  serial_stream.begin(115200, SERIAL_8N1, SW_RX, SW_TX);
  delay(100);
  
  LOG_DEBUG("=== Configuring Dual TMC2209 Drivers ===");
  
  // Configure Driver 1 (Left Motor)
  LOG_DEBUG("[DRIVER1]: Initializing...");
  driver1.setup(serial_stream, 115200, getSerialAddress(DRIVER_ADDRESS_1), SW_RX, SW_TX);
  delay(100);
  
  if (!driver1.isCommunicating()) {
    LOG_DEBUG("[DRIVER1]: ERROR - No UART communication!");
  } else {
    LOG_DEBUG("[DRIVER1]: Communication OK");
    LOG_DEBUG("[DRIVER1]: Hardware version: 0x", DebugLogBase::HEX, driver1.getVersion());
    
    // Configure driver settings for MAXIMUM TORQUE
    driver1.disableAutomaticCurrentScaling();
    driver1.disableAutomaticGradientAdaptation();
    
    // Set high current for strong torque
    driver1.setRunCurrent(R_CURRENT_PERCENT);
    driver1.setHoldCurrent(R_CURRENT_PERCENT); // Use full current in hold too
    driver1.setHoldDelay(10); // Minimal delay before reducing to hold current
    
    // Microstepping
    driver1.setMicrostepsPerStep(GUIDE_MICROSTEPPING);
    
    // SpreadCycle gives MUCH more torque than StealthChop
    // StealthChop is quiet but weak, SpreadCycle is louder but strong
    if (UART_STEALTH_MODE) {
      driver1.enableStealthChop();
      // If using StealthChop, set PWM thresholds for better performance
      driver1.setPwmOffset(36);
      driver1.setPwmGradient(14);
    } else {
      driver1.disableStealthChop(); // Use SpreadCycle for maximum torque
    }
    
    // Enable driver
    driver1.enable();
    delay(100);
    
    // Read back settings
    TMC2209::Settings settings1 = driver1.getSettings();
    LOG_DEBUG("[DRIVER1]: Microsteps per step:", settings1.microsteps_per_step);
    LOG_DEBUG("[DRIVER1]: Run current:", settings1.irun_percent, "%");
    LOG_DEBUG("[DRIVER1]: Hold current:", settings1.ihold_percent, "%");
    LOG_DEBUG("[DRIVER1]: StealthChop:", settings1.stealth_chop_enabled ? "ON (quiet/weak)" : "OFF (loud/strong)");
    LOG_DEBUG("[DRIVER1]: Enabled:", driver1.hardwareDisabled() ? "NO" : "YES");
  }
  
  delay(100);
  
  // Configure Driver 2 (Right Motor)  
  LOG_DEBUG("[DRIVER2]: Initializing...");
  driver2.setup(serial_stream, 115200, getSerialAddress(DRIVER_ADDRESS_2), SW_RX, SW_TX);
  delay(100);
  
  if (!driver2.isCommunicating()) {
    LOG_DEBUG("[DRIVER2]: ERROR - No UART communication!");
  } else {
    LOG_DEBUG("[DRIVER2]: Communication OK");
    LOG_DEBUG("[DRIVER2]: Hardware version: 0x", DebugLogBase::HEX, driver2.getVersion());
    
    // Configure driver settings for MAXIMUM TORQUE
    driver2.disableAutomaticCurrentScaling();
    driver2.disableAutomaticGradientAdaptation();
    
    // Set high current for strong torque
    driver2.setRunCurrent(R_CURRENT_PERCENT);
    driver2.setHoldCurrent(R_CURRENT_PERCENT); // Use full current in hold too
    driver2.setHoldDelay(10); // Minimal delay before reducing to hold current
    
    // Microstepping
    driver2.setMicrostepsPerStep(GUIDE_MICROSTEPPING);
    
    // SpreadCycle gives MUCH more torque than StealthChop
    // StealthChop is quiet but weak, SpreadCycle is louder but strong
    if (UART_STEALTH_MODE) {
      driver2.enableStealthChop();
      // If using StealthChop, set PWM thresholds for better performance
      driver2.setPwmOffset(36);
      driver2.setPwmGradient(14);
    } else {
      driver2.disableStealthChop(); // Use SpreadCycle for maximum torque
    }
    
    // Enable driver
    driver2.enable();
    delay(100);
    
    // Read back settings
    TMC2209::Settings settings2 = driver2.getSettings();
    LOG_DEBUG("[DRIVER2]: Microsteps per step:", settings2.microsteps_per_step);
    LOG_DEBUG("[DRIVER2]: Run current:", settings2.irun_percent, "%");
    LOG_DEBUG("[DRIVER2]: Hold current:", settings2.ihold_percent, "%");
    LOG_DEBUG("[DRIVER2]: StealthChop:", settings2.stealth_chop_enabled ? "ON (quiet/weak)" : "OFF (loud/strong)");
    LOG_DEBUG("[DRIVER2]: Enabled:", driver2.hardwareDisabled() ? "NO" : "YES");
  }
  
  LOG_DEBUG("=== Driver Configuration Complete ===");
}
