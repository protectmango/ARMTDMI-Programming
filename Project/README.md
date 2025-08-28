graph TD
    subgraph Microcontroller
        A[ADC Driver `adc_read()`] --> B{ADC Data};
        B --> C[Application Layer `main.c`];
    end

    subgraph Data Buffering and Protocol Stacks
        C --> D[IP Header Added];
        D --> E[TCP Header Added];
        E --> F[HTTP Header Added];
    end

    subgraph Hardware Interface
        F --> G[Data Encapsulated in Ethernet Frame];
        G --> H[SPI Bus (Serial Peripheral Interface)];
        H --> I[ENC28J60 Ethernet Controller];
    end

    subgraph Physical Network
        I --> J[Ethernet PHY];
        J --> K[Ethernet Cable];
        K --> L[Local Router];
    end

    L --> M[The Internet];

    M --> N[Web Server];
    N --> O[Website];

    style A fill:#f9f,stroke:#333,stroke-width:2px
    style B fill:#bbf,stroke:#333,stroke-width:2px
    style C fill:#f9f,stroke:#333,stroke-width:2px
    style D fill:#f9f,stroke:#333,stroke-width:2px
    style E fill:#f9f,stroke:#333,stroke-width:2px
    style F fill:#f9f,stroke:#333,stroke-width:2px
    style G fill:#f9f,stroke:#333,stroke-width:2px
    style H fill:#bbf,stroke:#333,stroke-width:2px
    style I fill:#f9f,stroke:#333,stroke-width:2px
    style J fill:#f9f,stroke:#333,stroke-width:2px
    style K fill:#bbf,stroke:#333,stroke-width:2px
    style L fill:#f9f,stroke:#333,stroke-width:2px
    style M fill:#bbf,stroke:#333,stroke-width:2px
    style N fill:#f9f,stroke:#333,stroke-width:2px
    style O fill:#bbf,stroke:#333,stroke-width:2px