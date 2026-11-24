#include "Particle.h"
#include <stdlib.h>

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

const int moisturePin = A0;
const int dryReading = 500;
const int wetReading = 2600;
const int superwetReading = 3500;
int targetMoisturePercent = 60;
const unsigned long readIntervalMs = 1000;
unsigned long lastReadTime = 0;

const char* moistureLevel(int reading) {
  if (reading <= dryReading) return "DRY";
  if (reading <= wetReading) return "WET";
  return "SUPER WET";
}

int moisturePercent(int reading) {
  long percent = static_cast<long>(reading - dryReading) * 100L /
      (superwetReading - dryReading);
  if (percent < 0) return 0;
  if (percent > 100) return 100;
  return static_cast<int>(percent);
}

bool setTargetMoisturePercent(int percent) {
  if (percent < 0 || percent > 100) {
    return false;
  }

  targetMoisturePercent = percent;
  Log.info("Target moisture set to %d%%", targetMoisturePercent);
  return true;
}

int setMoistureTargetFunction(String command) {
  const char* input = command.c_str();
  char* end = nullptr;
  long requestedPercent = strtol(input, &end, 10);
  if (end == input || *end != '\0' || requestedPercent < 0 || requestedPercent > 100) {
    Log.warn("Rejected moisture target; enter an integer from 0 to 100");
    return -1;
  }

  if (!setTargetMoisturePercent(static_cast<int>(requestedPercent))) {
    return -1;
  }
  return static_cast<int>(requestedPercent);
}

// setup() runs once, when the device is first turned on
void setup() {
  pinMode(moisturePin, INPUT);
  if (!Particle.function("moistTarget", setMoistureTargetFunction)) {
    Log.error("Failed to register moistTarget Particle function");
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  if (millis() - lastReadTime >= readIntervalMs) {
    lastReadTime = millis();
    int reading = analogRead(moisturePin);
    int currentMoisturePercent = moisturePercent(reading);
    const char* targetStatus = currentMoisturePercent >= targetMoisturePercent
        ? "TARGET MET"
        : "BELOW TARGET";
    Log.info("Moisture: %d%% (raw %d, %s), target: %d%% (%s)",
        currentMoisturePercent, reading, moistureLevel(reading),
        targetMoisturePercent, targetStatus);
  }
}