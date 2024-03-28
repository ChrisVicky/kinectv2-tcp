# `Tx` Controller

- Controller for the `Tx`: Transmission and Camera
- Data workflow

## Interaction with `Center` Controller

```note
 ┌────┐                                        ┌────────┐
 │ Tx │                                        │ Center │
 └─┬──┘                                        └───┬────┘
   │`MQTT.Init()`                     `MQTT.Init()`│ ─┐
   │                  ┌────────┐                   │  │
   │                  │ Broker │     `tcp.Listen()`│  │
   │ "/online"        └───┬────┘                   │  │
   ├─────────────────────►│           `AddWorker()`│  │ Image Transmission
   │                      │              "/ipport" │  │ Controller
   │`tcp.Connect()`       │◄───────────────────────┤  │
   │                      │                        │  │
   │`Image`[TCP]          │                        │  │
   ├──────────────────────────────────────────────►│  │
   ├──────────────────────────────────────────────►│  │
   ├──────────────────────────────────────────────►│ ─┘
   │                      │                        │ ─┐
   │                   "/client/WiFi-Tx/{ID}/start"│  │
   │`cmd(START)`          │◄───────────────────────┤  │
   │"/client/WiFi-Tx/{ID}/work"                    │  │ Tx Start/Stop
   ├─────────────────────►│                        │  │ Controller
   │                      │                        │  │
   │                    "/client/WiFi-Tx/{ID}/stop"│  │
   │`cmd(STOP)`           │◄───────────────────────┤  │
   │                      │                        │ ─┘
   ▼                      ▼                        ▼
```

> NOTE: `cmd(START)` start a transmission binary with default arguments
> and is continuous infinite loop until `cmd(STOP)` is received.
