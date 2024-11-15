Threat Model
a. Communication Security
 Threats:
o Eavesdropping: MQTT by default transmits data in plaintext, making it
vulnerable to interception.
o Man-in-the-Middle (MitM) Attacks: An attacker could intercept and modify data
during transmission.
o Replay Attacks: Old messages can be replayed to create confusion in data
analysis.
 Mitigations:
o Use TLS/SSL encryption to secure MQTT communications between ESP32
devices and the server.
o Authentication with strong client certificates or API tokens to verify device
legitimacy.
o Enable unique session IDs or time-stamped tokens to prevent replay attacks.
b. Device Security (ESP32)
 Threats:
o Physical Tampering: Physical access to ESP32 devices can allow attackers to
inject malicious firmware or extract sensitive data.
o Firmware Vulnerabilities: Insecure or outdated firmware may contain
vulnerabilities.
o Malicious Code Injection: Attackers could inject malware into the device’s
memory.
 Mitigations:
o Use secure boot on the ESP32 to prevent unauthorized firmware changes.
o Encrypt sensitive data stored on the ESP32, such as Wi-Fi credentials.
o Implement over-the-air (OTA) updates with secure signing to patch
vulnerabilities.
c. MQTT Server Security
 Threats:
o Denial of Service (DoS) Attacks: An attacker could flood the server with
requests, leading to downtime.
o Unauthorized Access: If the MQTT server is not securely configured,
unauthorized devices or users could connect.
o Data Injection: An attacker could publish incorrect temperature/humidity data.
 Mitigations:
o Use access control lists (ACLs) to restrict topics that each client can access.
o Ensure the MQTT server supports rate-limiting and IP blocking.
o Enable multi-factor authentication (MFA) where possible to protect access to
the server.
d. Data Integrity and Confidentiality in Database
 Threats:
o Data Leakage: Sensitive temperature and humidity data could be exposed.
o Data Tampering: Unauthorized users could modify or delete data, affecting
historical records.
o Insider Threats: Privileged users might misuse their access.
 Mitigations:
o Encrypt data at rest and in transit between the MQTT server and database.
o Apply role-based access control (RBAC) for database access.
o Audit logs to monitor access and changes, especially for sensitive data.
e. Dashboard Security
 Threats:
o Cross-Site Scripting (XSS) and SQL Injection: If the dashboard is accessible
via a web interface, these vulnerabilities could expose the system.
o Unauthorized Access: If authentication isn’t enforced, attackers might access
or manipulate data on the dashboard.
 Mitigations:
o Implement input validation and output encoding to prevent XSS and SQL
injection.
o Use strong, multi-factor authentication for dashboard access.
o Apply role-based permissions to restrict access to sensitive controls on the
dashboard.
f. Network Security
 Threats:
o Network Sniffing: Open networks could allow attackers to sniff MQTT traffic.
o Rogue Access Points: Attackers could set up fake access points to intercept
ESP32 communications.
 Mitigations:
o Use a secure network (WPA2/WPA3) for device connectivity.
o Segment the network to isolate IoT devices from critical infrastructure.
o Monitor network traffic for anomalies that could indicate rogue access points or
intrusions.
g. Availability and Continuity Risks
 Threats:
o System Downtime: Outages in any system component (MQTT server, database,
dashboard) can disrupt data flow.
o Data Loss: Loss of historical data could impact monitoring and analysis.
 Mitigations:
o Set up automated backups for historical data.
o Use failover configurations and high-availability setups for critical servers.
o Implement alert systems for rapid response to component failure.
