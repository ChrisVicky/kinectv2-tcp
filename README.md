# `Camera` Controller

- XBox2 (KinectV2) camera controller

## Interaction with `Center` Controller

```note
  ┌────────┐                            ┌────────┐
  │ Camera │                            │ Center │
  └───┬────┘                            └───┬────┘
      │`libfreenect2.Init()`                │
      │                       `tcp.Listen()`│
      │                                     │
      │ "Connect()"                         │
      ├────────────────────────────────────►│
      │                                     │
      │                          "Accept()" │
      │◄────────────────────────────────────┤
      │                                     │
      │`Image`                              │
      ├────────────────────────────────────►│
      ├────────────────────────────────────►│
      ├────────────────────────────────────►│
      ▼                                     ▼
```

> Note: Address and Port are pre-configured in the Camera and compiled in the binary.
