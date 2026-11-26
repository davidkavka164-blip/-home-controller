# HOME CONTROLLER CLOUD — working status
## Verified 2026-11-26
REV06 telemetry, TLS ISRG Root X1, mobile data, power-cycle and router-outage recovery. Reference in reference/REV06. No production controller modified.
## Next gate
Confirm stale-data indication; inspect broker ACL capabilities for a separate command-only publisher restricted to home-test/esp1/command. Existing home-app remains read-only. Do not grant global write.
## Control protocol design
Test only, no physical actuators initially: virtual LED state, then optional GPIO2 after board confirmation. Explicit SET not TOGGLE; short expiry, unique command ID, reject duplicates and malformed commands; no retained commands; session-specific nonce to reject previous-session commands; acknowledgment contains command ID and applied logical state. Browser must wait for acknowledgment and mark stale/unknown on timeout. MQTT transport acknowledgment alone is not execution confirmation.
Local control remains independent of cloud. Real actuator integration requires project-specific safety review, authorization and physical tests.
## Remaining
Broker ACL verification, command firmware and browser, negative tests, dashboard, individual project integration. Free-tier limits and browser credential exposure require review before production.
