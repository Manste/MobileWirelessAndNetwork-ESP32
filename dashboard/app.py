import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import json
import time

# MQTT Broker settings
MQTT_BROKER = "192.168.118.56"  # IP address of your MQTT broker
MQTT_PORT = 8008  # Port of your MQTT broker
MQTT_TOPIC = "esp1/sensorData"  # The MQTT topic to subscribe to

# Global variables for plotting
temperature_data = []
humidity_data = []
time_data = []

# Create a figure and axis for plotting
fig, (ax_temp, ax_hum) = plt.subplots(2, 1, figsize=(10, 8))

# Plot configuration
ax_temp.set_title("Temperature Over Time")
ax_temp.set_xlabel("Time")
ax_temp.set_ylabel("Temperature (°C)")
ax_hum.set_title("Humidity Over Time")
ax_hum.set_xlabel("Time")
ax_hum.set_ylabel("Humidity (%)")

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker with result code " + str(rc))
    # Subscribe to the topic
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    # Callback for when a message is received from the broker
    try:
        # Parse the JSON payload
        payload = json.loads(msg.payload)
        temperature = payload.get("temperature", None)
        humidity = payload.get("humidity", None)
        
        # Check if the data is valid
        if temperature is not None and humidity is not None:
            print(f"Received Data - Temperature: {temperature} °C, Humidity: {humidity}%")

            # Append the new data to the lists
            current_time = time.time()  # Use current time as x-axis
            time_data.append(current_time)
            temperature_data.append(temperature)
            humidity_data.append(humidity)

    except Exception as e:
        print(f"Error processing message: {e}")

def on_disconnect(client, userdata, rc):
    print("Disconnected from MQTT Broker with result code " + str(rc))

def animate(i):
    # This function will be called periodically by FuncAnimation to update the plot

    # Update the plots
    ax_temp.clear()
    ax_hum.clear()

    ax_temp.plot(time_data, temperature_data, label="Temperature (°C)", color='red')
    ax_hum.plot(time_data, humidity_data, label="Humidity (%)", color='blue')

    # Set titles and labels again after clearing
    ax_temp.set_title("Temperature Over Time")
    ax_temp.set_xlabel("Time")
    ax_temp.set_ylabel("Temperature (°C)")

    ax_hum.set_title("Humidity Over Time")
    ax_hum.set_xlabel("Time")
    ax_hum.set_ylabel("Humidity (%)")

    # Auto format the x-axis
    plt.xticks(rotation=45)

def main():
    # Create MQTT client with the latest version of callbacks
    client = mqtt.Client()

    # Assign the event callbacks using the latest API version
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect

    # Connect to the MQTT broker
    client.connect(MQTT_BROKER, MQTT_PORT, 60)

    # Start the MQTT loop in the background
    client.loop_start()

    # Set up real-time plot updates with FuncAnimation
    ani = FuncAnimation(fig, animate, interval=1000)  # Update every 1000 ms (1 second)

    # Display the plot
    plt.show()

    # Keep the script running to listen for MQTT messages
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")
        client.loop_stop()
        client.disconnect()
        plt.close()

if __name__ == "__main__":
    main()
