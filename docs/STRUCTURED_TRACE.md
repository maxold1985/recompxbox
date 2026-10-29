# Structured trace logger

`TraceLogger` writes newline-delimited JSON (JSONL) events with a
monotonically increasing `seq` number. Supported event types:
`cpu` (EIP/EAX/ESP), `mmio` (read/write, EIP, address, value, width),
`kernel` (EIP, ordinal, name), `interrupt` (EIP, vector), and `message`.

Example:
```json
{"seq":0,"type":"cpu","eip":65536,"eax":42,"esp":131072}
{"seq":1,"type":"mmio","op":"write","eip":65538,"address":4244635904,"value":1,"width":4}
```

The logger is **not yet attached to the QEMU TCG execution loop**.
To obtain real CPU/MMIO events, instrument QEMU's CPU tracepoints,
memory region callbacks and exception paths, then forward them to
the logger or translate QEMU trace output to this JSONL schema.
Similarly, kernel events must be emitted by the future Xbox HLE
dispatcher. No synthetic event should be represented as a real guest
observation.

The logger flushes every event for crash diagnostics. It is not
thread-safe; callers must synchronize concurrent producers. Trace
files can contain sensitive guest data and become large.
