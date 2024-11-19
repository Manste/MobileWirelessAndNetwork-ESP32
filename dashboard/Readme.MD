To run this application, first make sure that the MQTT server and the ESP32 are running properly. Then, install `paho-mqtt` using this command:
```bash
pip install paho.mqtt
```
Make sure the MQTT broker is in the same network as the ESP32 and you update the IP of MQTT server address within the ESP32's code and the server's code.

Then run the server with following command:
```bash
python app.py
```
This will display the data in realtime.
