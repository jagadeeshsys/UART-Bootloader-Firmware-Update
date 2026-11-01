flowchart TD
    A["Host PC - Python Uploader"]
    B["Firmware Binary"]
    C["Packet Generation + CRC32"]
    D["UART Serial Communication"]

    subgraph LPC2129["LPC2129 Bootloader"]
        E["UART0 Driver"]
        F["Protocol Parser"]
        G["CRC32 Validation"]
        H["Bootloader State Machine"]
        I["Timer0 Timeout Tracking"]
        J["RAM Flash Mock"]
        K["Firmware Verification"]
    end

    L["Simulated Application Handover"]

    B --> A
    A --> C
    C --> D
    D --> E
    E --> F
    F --> G
    G --> H
    I --> F
    H --> J
    H --> K
    K --> L

    E -. "ACK / NACK" .-> D
    D -. "Response" .-> A