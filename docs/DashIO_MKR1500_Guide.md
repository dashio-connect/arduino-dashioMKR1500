<h1 id="toc_0">Dash IoT Guide (Arduino MKR NB 1500)</h1>

**12 February 2025**

This guide demonstrates how to an Arduino MKR NB 1500 to the **Dash** app using the Arduino or PlatformIO IDE and the **DashIO** Adruino library. It also shows how to load user controls (widgets) into your mobile device to monitor and control an IoT device.

<h2 id="toc_1">Getting Started</h2>

For the big picture on <strong>Dash</strong>, take a look at our website: <a href="https://dashio.io">dashio.io</a>

For the Dash arduino library: <a href="https://github.com/dashio-connect/arduino-dashioMKR1500">github.com/dashio-connect/arduino-dashioMKR1500</a>

<h2 id="toc_2">Requirements</h2>

The **Dash** server is used to enable communication between the MKR NB 1500 and you mobile phone or tablet, so you will need **Dash** <a href="https://dashio.io/account-subscriptions">subscription.</a>

Grab an Arduino MKR NB 1500 board, Arduino IDE and follow this guide.

You will need to install the <strong>Dash IoT</strong> app on your mobile phone or tablet.

You will all need a mobile SIM card that is suitable for communicating through LTE Cat M1/NB1.

<h2 id="toc_3">Install</h2>

<h3 id="toc_4">Arduino IDE</h3>

You will need to add the **DashioMKR1500** library into your project.  It is included in the Arduino IDE Library manager. Search the library manager for the library titled "DashioMKR1500" and install. The Arduino IDE may also ask you to install the following libraries (dependencies). Please make sure they are all installed from the Arduino IDE Library Manager.

- **Dashio** is the core messageing library used to manage messages.
- **MKRNB** is required for LTE connectivity.
- **MQTT** by Joël Gähwiler is required for MQTT messaging.

<h3 id="toc_5">PlatformIO</h3>

Using **Dash** with the **PlatformIO** IDE is easy.

1. Install the **Dash** library into your project. The easiest way to do this is to download the <a href="https://github.com/dashio-connect/arduino-dashio">**arduino-dashio**</a> library  into the ***lib*** directory of your project.
2. Install the libraries listed above using the PlatformIO **Libraries** manager.
3. Edit the ***platformio.ini*** file for you project by adding ```lib_ldf_mode = deep``` to enable all the required files to be found and also add ```monitor_speed = 115200``` to set the required serial monitor speed.

Your finished ***platformio.ini*** file should look something like this:

```
[env:mkrnb1500]
platform = atmelsam
board = mkrnb1500
framework = arduino
lib_deps = 
	arduino-libraries/MKRNB@^1.5.1
	256dpi/MQTT@^2.5.1
lib_ldf_mode = deep
monitor_speed = 115200
```

<h2 id="toc_6">Guide</h2>

<h3 id="toc_7">MQTT Over LTE Basics</h3>

<p>Lets start with a simple MQTT connection.</p>

```
#include <DashioMKR1500.h>

#define DEVICE_TYPE "SAMD LTE"
#define DEVICE_NAME "ArdyWardy"

// MQTT
#define MQTT_USER      "yourMQTTuserName"
#define MQTT_PASSWORD  "yourMQTTpassword"

DashDevice dashDevice(DEVICE_TYPE);
DashMQTT mqtt_con(&dashDevice, true, true);
DashLTE lte(true);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(115200);
    delay(2000);

    dashDevice.name = DEVICE_NAME;

    mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);

    lte.attachConnection(&mqtt_con);
    lte.begin();
}

void loop() {
    digitalWrite(LED_BUILTIN, lte.cellConnected);

    lte.run();
}
```

This is about the fewest number of lines of code necessary to get talking to the <strong>Dash</strong> app. There is a lot happening under the hood to make this work. After the <code>#include &quot; DashioMKR1500.h&quot;</code> we create a device with the <em>device_type</em> as its only attribute. We also create a MQTT connection, with the newly created device being an attribute to the connection. The second attribute of the MQTT connection enables a push notification to be sent to your mobile phone when the IoT device reboots (Dash MQTT broker only). The final attribute enables the received and transmitted messages to be printed to the serial monitor.

An LTE (Cat M1/NB1) connection object is created with ```DashLTE lte(true);```, where the attribute enables printing messages to the serial monitor. 

In the <code>setup()</code> function we set the built in LED to be used to show the connection status of the LTE connection, start the serial monitor and assign the device name to the ```dashDevice``` object. 

At this point in the code you can also assign your own deviceID with ```dashDevice.setup("yourDeviceID")```. However, if you don't set the deivceID, the IMEI from the LTE modem will be assigned to the deviceID.

The dash MQTT connection is then setup with a username and password. Make sure you enter your username and password correctly where they are defined at the top of the code example.

We then attach the MQTT connection to the ```lte``` object with <code>lte.attachConnection(&amp;mqtt_con);</code>. This enables the <code>lte</code> object to manage the MQTT connection from here on in.

Finally, we start the ```lte``` connection <code>lte.begin()</code>.

This device is discoverable by the <strong>Dash</strong> app. 

Setup the Arduino IDE serial monitor to 115200 baud and run the above code. Then run the <strong>Dash</strong> app on your mobile device and you will see connection &quot;WHO&quot; messages on the Arduino serial monitor as the Dash app detects and communicates with your IoT device.

Lets add Dial control messages that are sent to the <strong>Dash</strong> app every second. To do this we create a new task to provide a 1 second time tick and then send a Dial value message from the loop every second.

```
// This example uses an Arduino MKR NB 1500 board

#include <DashioMKR1500.h>

#define DEVICE_TYPE "SAMD LTE"
#define DEVICE_NAME "ArdyWardy"

// MQTT
#define MQTT_USER      "yourMQTTuserName"
#define MQTT_PASSWORD  "yourMQTTpassword"

DashDevice dashDevice(DEVICE_TYPE);
DashMQTT mqtt_con(&dashDevice, true, true);
DashLTE lte(true);

auto timer = timer_create_default();
bool oneSecond = false; // Set by hardware timer every second.

// Timer Interrupt
static bool onTimerCallback(void *argument) {
    oneSecond = true;
    return true; // to repeat the timer action - false to stop
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(115200);
    delay(2000);

    dashDevice.name = DEVICE_NAME;

    mqtt_con.setup(MQTT_USER, MQTT_PASSWORD);

    lte.attachConnection(&mqtt_con);
    lte.begin();

    timer.every(1000, onTimerCallback); // 1000ms
}

void loop() {
    digitalWrite(LED_BUILTIN, lte.cellConnected);

    timer.tick();
    lte.run();

    if (oneSecond) { // Tasks to occur every second
        oneSecond = false;
        mqtt_con.sendMessage(dashDevice.getDialMessage("D01", int(random(0, 100))));
    }
}
```

The line <code>dashDevice.getDialMessage(&quot;D01&quot;, int(random(0, 100)))</code> creates the message with two parameters. The first parameter is the <em>control_ID</em> which identifies the specific Dial control in the <strong>Dash</strong> app and the second parameter is simply the Dial value.

Once again, run the above code and the <strong>Dash</strong> app. This time, a new &quot;DIAL&quot; messages will be seen on the serial monitor every second.

The next step is to show the Dial values from the messages on a control on the <strong>Dash</strong> app.

In the <strong>Dash</strong> app, tap the <img src="https://dashio.io/wp-content/uploads/2021/07/iot_blue_44.png" width="20"> <strong>All Devices</strong> button, followed by the <img src="https://dashio.io/wp-content/uploads/2021/07/magnifying_glass_44_blue.png" width="20"> <strong>Find New Device</strong> button. Then select the <strong>My Devices on Dash</strong> option to show a list of new IoT devices on the **Dash** server. Your IoT device shpuld be shown in the list. Select your device and from the next menu select <strong>Create Device View</strong>. This will create an empty Device View for your new IoT deivce. You can now add controls to the Device View:

<h3 id="toc_8">Adding Controls to Dash App</h3>

Once you have discovered your IoT device in the **Dash** app and have a **Device View** available for editing, you can add controls to the **Device View**:

<ul>
<li><strong>Start Editing</strong>: Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/pencil_44_blue.png" width="20"> <strong>Edit</strong> button (it not already in editing mode).</li>
<li><strong>Add Dial Control</strong>: Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/add_44_blue.png" width="20"> <strong>Add Control</strong> button and select the Dial control from the list.</li>
<li><strong>Edit Controls</strong>: Tap the Dial control to select it. The <img src="https://dashio.io/wp-content/uploads/2021/07/spanner_44_button.png" width="20"> <strong>Control Settings Menu</strong> button will appear in the middle of the Control. The Control can then be dragged and resized (pinch). Tapping the <img src="https://dashio.io/wp-content/uploads/2021/07/spanner_44_button.png" width="20"> button allows you to edit the Control settings where you can setup the style, colors and other characteristics of your Control. Make sure the <em>Control_ID</em> is set to the same value that is used in the Dial messages (in this case it should be set to &quot;D01&quot;).</li>
<li><strong>Quit editing</strong>: Tap the <img src="https://dashio.io/wp-content/uploads/2021/07/pencil_quit_44.png" width="20"> <strong>Edit</strong> button again.</li>
</ul>

The Dial on the <strong>Dash</strong> app will now show the random Dial values as they arrive.

The next piece of the puzzle to consider is how your IoT device can receive data from the <strong>Dash</strong> app. Lets add a Knob and connect it to the Dial.

In the <strong>Dash</strong> app you will need to add a <strong>Knob</strong> control onto your <strong>Device View</strong>, next to your <strong>Dial</strong> control. Edit the <strong>Knob</strong> to make sure the <strong>Control ID</strong> of the Knob matches what you have used in your Knob messages (in this case it should be &quot;KB01&quot;), then quit edit mode.

Continuing with the example, in the Arduino code we need to respond to messages coming in from a Knob control that we just added to the <strong>Dash</strong> app. To make the changes to your IoT device we add a callback, <code>processIncomingMessage</code>, into the MQTT connection with the <code>setCallback</code> function.

```
// This example uses an Arduino MKR NB 1500 board

#include <DashioMKR1500.h>

#define DEVICE_TYPE "SAMD LTE"
#define DEVICE_NAME "ArdyWardy"

// MQTT
#define MQTT_USER      "yourMQTTuserName"
#define MQTT_PASSWORD  "yourMQTTpassword"

DashDevice dashDevice(DEVICE_TYPE);
DashMQTT mqtt_con(&dashDevice, true, true);
DashLTE lte(true);

int dialValue = 0;

void processIncomingMessage(MessageData *messageData) {
    switch (messageData->control) {
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
```

We obtain the Knob value from the message data payload that we receive in the <code>processIncomingMessage</code> function. We then create a Dial message with the value from the Knob and send this back to the <strong>Dash</strong> app. Remember to remove the timer and the associated Dial <code>ble_con.sendMessage</code> from the <code>loop()</code> function.

<p>When you adjust the Knob on the <strong>Dash</strong> app, a message with the Knob value is sent your IoT device, which returns the Knob value into the Dial control, which you will see on the <strong>Dash</strong> app.</p>

<p>Finally, we should respond to the STATUS message from the <strong>Dash</strong> app. STATUS messages allows the IoT device to send initial conditions for each control to the <strong>Dash</strong> app as soon as a connection becomes active. Once again, we do this from the <code>processIncomingMessage</code> function and our complete code looks like this:</p>

```
// This example uses an Arduino MKR NB 1500 board

#include <DashioMKR1500.h>

#define DEVICE_TYPE "SAMD LTE"
#define DEVICE_NAME "ArdyWardy"

// MQTT
#define MQTT_USER      "yourMQTTuserName"
#define MQTT_PASSWORD  "yourMQTTpassword"

DashDevice dashDevice(DEVICE_TYPE);
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
```

<h2 id="toc_9">Layout Configuration</h2>

<p><strong>Layout configuration</strong> allows the IoT device to hold a copy of the complete layout of the device as it should appear on the <strong>Dash</strong> app. It includes all infomration for the device, controls (size, colour, style etc.), device views and connections.</p>

<p>When the <strong>Dash</strong> app discovers a new IoT device, it will download the Layout Configuration from the IoT device to display the device controls they way they have been designed by the developer. This is particularly useful developing commercial products and distributing your IoT devices to other users.</p>

<p>To include the Layout Configuration in your IoT device, simply follow these steps:</p>

<ul>
<li><strong>Design your layout</strong> in the <strong>Dash</strong> app and include all controls and connections that you need in your layout.</li>
<li><strong>Export the layout</strong>: Tap on the <strong>Device</strong> button, then tap the <strong>Export Layout</strong> button.</li>
<li>Tap the <strong>Export</strong> button. The Layout Configuration will be emailed to you.</li>
<li><strong>Copy and paste</strong> the C64 configuration text from the email into your Arduino code, assigning it to a pointer to store the text in program memory. Your C64 configuration text will be different to that shown below.</li>
<li><strong>Add the Layout Config Revision</strong> integer (CONFIG_REV) as a third attribute to the DashDevice object.</li>
</ul>

```
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

DashDevice    dashDevice(DEVICE_TYPE, configC64Str, CONFIG_REV);
```

Here is our Knob and Dial example, with MQTT connection through LTE and Layout Configuration added:

```
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
```

<h1 id="toc_10">Jump In and Build Your Own IoT Device</h1>

When you are ready to create your own IoT device, the Dash Arduino C++ Library will provide you with more details about what you need to know:

<a href="https://dashio.io/arduino-library/">https://dashio.io/arduino-library/</a>
