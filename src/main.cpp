#include <Arduino.h>
#include "globals.h"
#include "canHelper.h"

#define CAN_SEND_MESSAGE_SHUNT01_IDENTIFIER 0x23;
#define CAN_SEND_MESSAGE_SHUNT02_IDENTIFIER 0x24;

volatile int batteryVoltageWholeNumber;
volatile int batteryVotlageDecimal;
volatile int isShuntCurrentNegative;
volatile int shuntCurrentWholeNumber;
volatile int shuntCurrentDecimal;
volatile int batterySocWholeNumber;
volatile int batterySocDecimal;
volatile int isWattageInUseNegative;
volatile int wattageInUse;
volatile byte wattageMsb;
volatile byte wattageLsb;
volatile byte wattageNegativeIndicator;
volatile byte ttgMsb;
volatile byte ttgLsb;

unsigned long canStartMillis;
unsigned long canCurrentMillis;
const unsigned long canStatusPeriod = 33;

static void send_batt_message()
{
  // Configure message to transmit
  twai_message_t message;
  message.identifier = CAN_SEND_MESSAGE_SHUNT01_IDENTIFIER;
  message.extd = false;
  message.rtr = false;
  message.data_length_code = 7;
  message.data[0] = batteryVoltageWholeNumber;
  message.data[1] = batteryVotlageDecimal;
  message.data[2] = isShuntCurrentNegative;
  message.data[3] = shuntCurrentWholeNumber;
  message.data[4] = shuntCurrentDecimal;
  message.data[5] = batterySocWholeNumber;
  message.data[6] = batterySocDecimal;
  // Queue message for transmission
  if (twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK)
  {
    // debugln("Message queued for transmission");
  }
  else
  {
    // debugln("Failed to queue message for transmission");
  }
  return;
}

static void send_batt_message_01()
{
  // Configure message to transmit
  twai_message_t message;
  message.identifier = CAN_SEND_MESSAGE_SHUNT02_IDENTIFIER;
  message.extd = false;
  message.rtr = false;
  message.data_length_code = 5;
  message.data[0] = wattageNegativeIndicator;
  message.data[1] = wattageMsb;
  message.data[2] = wattageLsb;
  message.data[3] = ttgMsb;
  message.data[4] = ttgLsb;
  // Queue message for transmission
  if (twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK)
  {
    // debugln("Message queued for transmission");
  }
  else
  {
    // debugln("Failed to queue message for transmission");
  }
  return;
}

void setup()
{
  Serial.begin(115200); // Used for debugging and logging
  while (!Serial)
  {
    ;
    ;
  }
  Serial2.begin(19200);
  canHelper::canSetup();
}

void loop()
{
  canCurrentMillis = millis();
  if (canCurrentMillis - canStartMillis >= canStatusPeriod)
  {
    while (Serial2.available())
    {
      String input = Serial2.readStringUntil('\n');
      // Split the input string based on the delimiter "]t"
      int startIndex = 0;
      int delimiterIndex = 0;

      while ((delimiterIndex = input.indexOf("\t", startIndex)) != -1)
      {
        // Extract the key (substring from startIndex to delimiterIndex)
        String key = input.substring(startIndex, delimiterIndex);
        startIndex = delimiterIndex + 1; // Move past the "]t" delimiter

        // Extract the value (substring from startIndex to the next delimiter or end of string)
        int nextDelimiterIndex = input.indexOf("\t", startIndex);
        String value;
        if (nextDelimiterIndex != -1)
        {
          value = input.substring(startIndex, nextDelimiterIndex);
          startIndex = nextDelimiterIndex + 2; // Move past the "\t" delimiter
        }
        else
        {
          value = input.substring(startIndex); // Handle last key-value pair (no delimiter after value)
        }

        if (key == "V")
        {
          float voltageValue = (value.toInt() * 0.001);
          batteryVoltageWholeNumber = (int)voltageValue;                                   // First we get the whole number voltage.
          batteryVotlageDecimal = (int)((voltageValue - batteryVoltageWholeNumber) * 100); // For 2 decimal places
        }
        else if (key == "VPV")
        {
          // debug("Panel Voltage: ");
          // debugln(value);
        }
        else if (key == "PPV")
        {
          // debug("Panel Power: ");
          // debugln(value);
        }
        else if (key == "I")
        {
          float currentValue = (value.toInt() * 0.001);
          shuntCurrentWholeNumber = (int)currentValue;
          shuntCurrentDecimal = (int)((currentValue - shuntCurrentWholeNumber) * 100);
          if (currentValue < 0)
          {
            isShuntCurrentNegative = 1;
          }
          else
          {
            isShuntCurrentNegative = 0;
          }
          // debug("Current: ");
          // debug(currentValue);
          // debugln(" A");
        }
        else if (key == "P")
        {
          wattageInUse = value.toInt();
          byte signByte = (wattageInUse < 0) ? 0xFF : 0x00; // MSB (Most Significant Bit) is set for negative numbers
          // Get the absolute value of the number
          unsigned int absValue = abs(wattageInUse);
          // Break the absolute value into 2 bytes
          byte lowByte = absValue & 0xFF;         // Least significant byte
          byte highByte = (absValue >> 8) & 0xFF; // Most significant byte          
          // Store the sign byte and the two data bytes
          wattageNegativeIndicator = signByte;
          wattageMsb = highByte;
          wattageLsb = lowByte;
        }
        else if (key == "CE")
        {
          float consumedPowerValue = (value.toInt() * 0.001);
          // debug("Consumed Ah: ");
          // debug(consumedPowerValue);
          // debugln(" A");
        }
        else if (key == "SOC")
        {
          float battSoc = (value.toInt() * 0.1);
          batterySocWholeNumber = (int)battSoc;
          batterySocDecimal = (int)((battSoc - batterySocWholeNumber) * 100);
        }
        else if (key == "TTG")
        {
          // debug("Time Remaining: ");
          // float ttgValue = (value.toInt() / 60);
          // debug(ttgValue);
          // debugln("hrs");
          int number = value.toInt();
          ttgMsb = (number >> 8) & 0xFF;  // Most significant byte
          ttgLsb = number & 0xFF;         // Least significant byte         
        }
        else if (key == "Alarm")
        {
          // debug("Alarm Condition: ");
          // debugln(value);
        }
        else if (key == "AR")
        {
          // debug("Alarm Reason: ");
          // debugln(value);
        }
        else if (key == "Relay")
        {
          // debug("Relay State: ");
          // debugln(value);
        }
        else if (key == "H1")
        {
          // debug("Depth of Deepest Discharge: ");
          // debugln(value);
        }
        else if (key == "H2")
        {
          // debug("Depth of Last Discharge: ");
          // debugln(value);
        }
        else if (key == "H3")
        {
          // debug("Depth of Average Discharge: ");
          // debugln(value);
        }
        else if (key == "H4")
        {
          // debug("Number of Charge Cycles: ");
          // debugln(value);
        }
        else if (key == "H5")
        {
          // debug("Number of Full Discharges: ");
          // debugln(value);
        }
        else if (key == "H6")
        {
          // debug("Cumulative Amp Hours Drawn: ");
          // debugln(value);
        }
        else if (key == "H7")
        {
          float minVoltageValue = (value.toInt() * 0.001);
          // debug("Minimum Main Battery Voltage: ");
          // debug(minVoltageValue);
          // debugln(" V");
        }
        else if (key == "H8")
        {
          float maxVoltageValue = (value.toInt() * 0.001);
          // debug("Maximum Main Battery Voltage: ");
          // debug(maxVoltageValue);
          // debugln(" V");
        }
        else if (key == "H9")
        {
          // debug("Seconds since last full charge: ");
          // debugln(value);
        }
        else if (key == "H10")
        {
          // debug("Number of automatic syncs: ");
          // debugln(value);
        }
        else if (key == "H11")
        {
          // debug("Number of low main voltage alarms: ");
          // debugln(value);
        }
        else if (key == "H12")
        {
          // debug("Number of high main voltage alarms: ");
          // debugln(value);
        }
        else if (key == "H17")
        {
          // debug("Amount of discharded energy: ");
          // debugln(value);
        }
        else if (key == "H18")
        {
          // debug("Amount of charged energy: ");
          // debugln(value);
        }
        else if (key == "BMV")
        {
          // debug("Model: ");
          // debugln(value);
        }
        else if (key == "FW")
        {
          // debug("Firmware Version: ");
          // debugln(value);
        }
        else if (key == "PID")
        {
          // debug("Product ID: ");
          // debugln(value);
        }
      }
    }
    canHelper::canLoop();
    send_batt_message();
    send_batt_message_01();
    canStartMillis = canCurrentMillis;
  }
}
