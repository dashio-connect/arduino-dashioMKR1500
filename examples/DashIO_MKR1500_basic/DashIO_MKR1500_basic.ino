// This example uses an Arduino MKR NB 1500 board

#include <DashioMKR1500.h>

#define DEVICE_TYPE "SAMD LTE"
#define DEVICE_NAME "ArdyWardy"

// MQTT
#define MQTT_USER      "yourMQTTuserName"
#define MQTT_PASSWORD  "yourMQTTpassword"

const char configC64Str[] PROGMEM =
"vVNNc9owEP0rGZ09HWOapMPNxoFQbAxGIe10elCwgC1CYmQ5QDL896z8kbhJLr10fGDZfXor7dv3TKIgIr1fvx0yniQBRs+kKCAj"
"PaK+//wKMzmcTeHb+q5j9vEf1pkRh+TmJDgCJkka+xEmlkoarcQoxOQ4cDsWw2WWSHFKZMoFZznijS64Q3bsSHod13XInmkuTXko"
"XNgzmmcLJgqEXmPZgCm70PLXIRsO641JmQFFeu6XrotfjZqqHDAtER0klCaxvcBGHWKQsW23YiLH1qcG90ZyZT+HbKV66CuhNDLE"
"TCukckgGTAyUEOqQj7HePMCmG+xQc26hB8jM5pX10iHHD60879q7xgvvAK/pnnHe8yhMq8nTmx+0isJRnYr9aR3cTO6qKEqGVeAv"
"wiqYR31a4/tRHYSLxX2pYqNS4M9HfSuS0QKnMUCt5vCElY6HU15ryPA1xU7mpOd5dm4oS5VpXiyL3Suk87falXCWOlA64/ptMOzU"
"FLZrmTX5+w0YXheaXMj09uL9AaqZzMsNWZ5wXrZrzcxA5g9KK/JxRezpMg7UsWEPBFtu39U+kNsppKgz6XW7LWhrVrgkgA+fsF25"
"+Upyci718qO2Z9bjweVlMB2ledGd319lT3v/sGp7JvBT/LfXfAl5ubHeu4GW7vlfJilXHMSrFDFk0hJcBLbDZ45xa8q9AmnaeldG"
"wFV54LrVKroZ0M/d+A+eKSQY1IaQtn36t2ltGjpMp7dVGFA6aaJh7aD+AH3zTDL+CEs+56bYI5VEIZ0DrMDJWL5x4hml1fXDErYA"
"fqi3fbVO+SOG5/ML";

DashDevice dashDevice(DEVICE_TYPE, configC64Str, 1);
DashMQTT mqtt_con(&dashDevice, true, true);
DashLTE lte(true);

int dialValue = 0;

void processStatus(ConnectionType connectionType) {
    String message((char *)0);
    message.reserve(1024);

    message = dashDevice.getKnobMessage("KB01", dialValue);
    message += dashDevice.getDialMessage("D01", dialValue);

    mqtt_con.sendMessage(message);
}

void processIncomingMessage(MessageData *messageData) {
    switch (messageData->control) {
    case status:
        processStatus(messageData->connectionType);
        break;
    case knob:
        if (messageData->idStr == "KB01") {
            dialValue = messageData->payloadStr.toFloat();
            String message = dashDevice.getDialMessage("D01", dialValue);
            mqtt_con.sendMessage(message);
        }
        break;
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(115200);
    delay(2000);

    dashDevice.name = DEVICE_NAME;

    mqtt_con.setCallback(&processIncomingMessage);
    mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);

    lte.attachConnection(&mqtt_con);
    lte.begin();
}

void loop() {
    digitalWrite(LED_BUILTIN, lte.cellConnected);

    lte.run();
}