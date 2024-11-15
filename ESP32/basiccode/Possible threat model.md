# Threat Model
## a. Communication Security
### Threats:
+ **Eavesdropping:** MQTT by default transmits data in plaintext, making it
vulnerable to interception.
+ **Man-in-the-Middle (MitM) Attacks:** An attacker could intercept and modify data
during transmission.
+ **Replay Attacks:** Old messages can be replayed to create confusion in data
analysis.
### Mitigations:
+ Use TLS/SSL encryption to secure MQTT communications between ESP32
devices and the server.
+ Authentication with strong client certificates or API tokens to verify device
legitimacy.
+ Enable unique session IDs or time-stamped tokens to prevent replay attacks.
  
## b. Device Security (ESP32)
### Threats:
+ **Physical Tampering:** Physical access to ESP32 devices can allow attackers to
inject malicious firmware or extract sensitive data.
+ **Firmware Vulnerabilities:** Insecure or outdated firmware may contain
vulnerabilities.
+ **Malicious Code Injection:** Attackers could inject malware into the device’s
memory.
### Mitigations:
+ Use secure boot on the ESP32 to prevent unauthorized firmware changes.
+ Encrypt sensitive data stored on the ESP32, such as Wi-Fi credentials.
+ Implement over-the-air (OTA) updates with secure signing to patch
vulnerabilities.

## c. MQTT Server Security
### Threats:
+ **Denial of Service (DoS) Attacks:** An attacker could flood the server with
requests, leading to downtime.
+ **Unauthorized Access:** If the MQTT server is not securely configured,
unauthorized devices or users could connect.
+ **Data Injection:** An attacker could publish incorrect temperature/humidity data.
### Mitigations:
+ Use access control lists (ACLs) to restrict topics that each client can access.
+ Ensure the MQTT server supports rate-limiting and IP blocking.
+ Enable multi-factor authentication (MFA) where possible to protect access to
the server.

## d. Data Integrity and Confidentiality in Database
### Threats:
+ **Data Leakage:** Sensitive temperature and humidity data could be exposed.
+ **Data Tampering:** Unauthorized users could modify or delete data, affecting
historical records.
+ **Insider Threats:** Privileged users might misuse their access.
### Mitigations:
+ Encrypt data at rest and in transit between the MQTT server and database.
+ Apply role-based access control (RBAC) for database access.
+ Audit logs to monitor access and changes, especially for sensitive data.
  
## e. Dashboard Security
### Threats:
+ **Cross-Site Scripting (XSS) and SQL Injection:** If the dashboard is accessible
via a web interface, these vulnerabilities could expose the system.
+ **Unauthorized Access:** If authentication isn’t enforced, attackers might access
or manipulate data on the dashboard.
### Mitigations:
+ Implement input validation and output encoding to prevent XSS and SQL
injection.
+ Use strong, multi-factor authentication for dashboard access.
+ Apply role-based permissions to restrict access to sensitive controls on the
dashboard.

## f. Network Security
### Threats:
+ **Network Sniffing:** Open networks could allow attackers to sniff MQTT traffic.
+ **Rogue Access Points:** Attackers could set up fake access points to intercept
ESP32 communications.
### Mitigations:
+ Use a secure network (WPA2/WPA3) for device connectivity.
+ Segment the network to isolate IoT devices from critical infrastructure.
+ Monitor network traffic for anomalies that could indicate rogue access points or
intrusions.

## g. Availability and Continuity Risks
### Threats:
+ **System Downtime:** Outages in any system component (MQTT server, database,
dashboard) can disrupt data flow.
+ **Data Loss:** Loss of historical data could impact monitoring and analysis.
### Mitigations:
+ Set up automated backups for historical data.
+ Use failover configurations and high-availability setups for critical servers.
+ Implement alert systems for rapid response to component failure.
