import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from tkinter import Tk, Scale, Label, HORIZONTAL, Frame
from datetime import datetime
from matplotlib.animation import FuncAnimation
import threading
import time
from utils import *

# MQTT broker configuration
BROKER = "192.168.79.56"
PORT = 8008
USERNAME = "server"
PASSWORD = "nxPd3T25Jt2eYDTT82n9d78xQ" 

# Topics
ESP_TOPICS = {
    "esp1": {"temperature": "esp1/temperature", "humidity": "esp1/humidity", "threshold": "esp1/threshold"},
    "esp2": {"temperature": "esp2/temperature", "humidity": "esp2/humidity", "threshold": "esp2/threshold"},
}

# Data storage with separate time lists
data = {
    "esp1": {
        "temperature": [], 
        "humidity": [], 
        "threshold": 50,
        "time_temperature": [], 
        "time_humidity": []
    },
    "esp2": {
        "temperature": [], 
        "humidity": [], 
        "threshold": 50,
        "time_temperature": [], 
        "time_humidity": []
    },
}

# Create a lock to handle data synchronization
data_lock = threading.Lock()

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker")
        for esp, topics in ESP_TOPICS.items():
            client.subscribe(topics["temperature"])
            client.subscribe(topics["humidity"])
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    esp_name = None
    value_type = None
    for esp, topics in ESP_TOPICS.items():
        if msg.topic == topics["temperature"]:
            esp_name = esp
            value_type = "temperature"
        elif msg.topic == topics["humidity"]:
            esp_name = esp
            value_type = "humidity"

    if esp_name and value_type:
        try:
            tmp_data = msg.payload.decode()
            print("e received: ", tmp_data)
            decrypted_data = process_received_payload(tmp_data, esp_name)
            value = float(decrypted_data)
            timestamp = datetime.now().strftime("%H:%M:%S")
            with data_lock:
                # Choose the correct time list
                time_key = f"time_{value_type}"

                # Synchronize lengths
                if len(data[esp_name][time_key]) >= 50:  # Limit to 50 points
                    data[esp_name][time_key].pop(0)
                    data[esp_name][value_type].pop(0)

                # Add new data
                data[esp_name][time_key].append(timestamp)
                data[esp_name][value_type].append(value)

        except ValueError:
            print(f"Invalid data received on {msg.topic}: {msg.payload.decode()}")


# MQTT Client
mqtt_client = mqtt.Client()

# Set username and password for authentication
mqtt_client.username_pw_set(USERNAME, PASSWORD)

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(BROKER, PORT)

mqtt_thread = threading.Thread(target=mqtt_client.loop_forever)
mqtt_thread.daemon = True  # Ensure the thread exits when the main program exits
mqtt_thread.start()

# Function to update the threshold and publish to the appropriate topic
def update_threshold(esp, value):
    with data_lock:
        data[esp]["threshold"] = int(value)
    topic = ESP_TOPICS[esp]["threshold"]
    threshold_data = prepare_payload(str(data[esp]["threshold"]))
    mqtt_client.publish(topic, threshold_data)
    print(f"Updated {esp} threshold to {value} and sent to {topic}")

# Tkinter GUI with integrated sliders and plots
root = Tk()
root.title("ESP Sensor Monitoring")

# Create a figure for the plots
fig, axes = plt.subplots(2, 2, figsize=(12, 8))  # 2x2 grid for temperature/humidity of 2 ESPs
esp_axes = {
    "esp1": {"temperature": axes[0, 0], "humidity": axes[1, 0]},
    "esp2": {"temperature": axes[0, 1], "humidity": axes[1, 1]},
}

# Add the matplotlib figure to the Tkinter window
canvas = FigureCanvasTkAgg(fig, master=root)
canvas_widget = canvas.get_tk_widget()
canvas_widget.grid(row=0, column=0, columnspan=2)

# Add sliders for threshold control
slider_frame = Frame(root)
slider_frame.grid(row=1, column=0, columnspan=2, pady=10)

for idx, esp in enumerate(ESP_TOPICS.keys()):
    Label(slider_frame, text=f"{esp.upper()} Humidity Threshold").grid(row=idx, column=0, padx=10, pady=10)
    slider = Scale(
        slider_frame,
        from_=0,
        to=100,
        resolution=5,  # Step size
        orient=HORIZONTAL,
        tickinterval=10,  # Show tick marks every 10
        length=400,
        command=lambda value, esp=esp: update_threshold(esp, value),
    )
    slider.set(data[esp]["threshold"])
    slider.grid(row=idx, column=1, padx=10, pady=10)

# Function to animate plots
def animate(i):
    with data_lock:
        for esp, sensors in data.items():
            # Plot temperature
            if len(sensors["time_temperature"]) == len(sensors["temperature"]):
                temp_ax = esp_axes[esp]["temperature"]
                temp_ax.clear()
                temp_ax.plot(
                    sensors["time_temperature"], 
                    sensors["temperature"], 
                    label="Temperature (°C)", 
                    color="blue"
                )
                temp_ax.set_title(f"{esp.upper()} Temperature")
                temp_ax.set_xlabel("Time")
                temp_ax.set_ylabel("Temperature (°C)")
                temp_ax.legend(loc="upper right")
                temp_ax.tick_params(axis="x", rotation=45)
            else:
                print(f"Skipping temperature plot for {esp} due to mismatched lengths")

            # Plot humidity
            if len(sensors["time_humidity"]) == len(sensors["humidity"]):
                hum_ax = esp_axes[esp]["humidity"]
                hum_ax.clear()
                hum_ax.plot(
                    sensors["time_humidity"], 
                    sensors["humidity"], 
                    label="Humidity (%)", 
                    color="green"
                )
                hum_ax.axhline(
                    sensors["threshold"], 
                    color="red", 
                    linestyle="--", 
                    label="Threshold"
                )
                hum_ax.set_title(f"{esp.upper()} Humidity")
                hum_ax.set_xlabel("Time")
                hum_ax.set_ylabel("Humidity (%)")
                hum_ax.legend(loc="upper right")
                hum_ax.tick_params(axis="x", rotation=45)
            else:
                print(f"Skipping humidity plot for {esp} due to mismatched lengths")
    fig.tight_layout()
    canvas.draw()

# Function to continuously publish thresholds every second
def publish_thresholds():
    while True:
        with data_lock:
            for esp, sensors in data.items():
                topic = ESP_TOPICS[esp]["threshold"]
                threshold_value = prepare_payload(str(sensors["threshold"]))
                mqtt_client.publish(topic, threshold_value)
                print(f"Published {esp} threshold: {threshold_value} to {topic}")
        time.sleep(1)  # Wait for 1 second before the next publish

# Start the threshold publishing thread
threshold_thread = threading.Thread(target=publish_thresholds)
threshold_thread.daemon = True  # Ensure this thread exits when the main program exits
threshold_thread.start()

# Ensure animation updates are safely handled
ani = FuncAnimation(fig, animate, interval=1000)
plt.tight_layout()
canvas.draw()

# Start the Tkinter mainloop
root.mainloop()
