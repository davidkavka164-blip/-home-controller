# REV06 reference
Archived equivalent logic of the sketch field-tested on 2026-11-26 (formatting condensed; archive itself not recompiled).
Arduino IDE 1.8.19, ESP32 core 1.0.6, PubSubClient 2.8.
Copy secrets.example.h to secrets.h, enter local credentials and official ISRG Root X1 PEM. Never commit secrets.h.
MQTT TLS 8883; web WSS 8884. Telemetry only, no GPIO outputs.
Observed: certificate verified, MQTT OK, publishes every 10 seconds, browser reception, mobile data reception, recovery after power cycle and router outage.
Not yet explicitly confirmed: stale-data warning. Long-term stability not established.
