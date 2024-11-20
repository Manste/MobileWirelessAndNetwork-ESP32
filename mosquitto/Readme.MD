To launch the MQTT broker, first make sure you have stop the mosquitto.service using this command:
```
systemctl stop mosquitto.service
```
Now, we are going to run the command that will launch MQTT broker based on our configuration file:
```
mosquitto -c mosquitto.conf -v
```
