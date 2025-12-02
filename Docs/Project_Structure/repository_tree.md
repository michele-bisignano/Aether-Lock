Aether-Lock/
├── .gitignore                      
├── LICENSE                         # MIT License
├── README.md                       
├── project_config.json             # Centralized physics and control parameters
│
├── Docs/                           # Documentation and design specifications
│   ├── Project_Structure/
│   │   └── repository_tree.md      # Visual representation of the project structure (This file)
│   └── Theory/
│       ├── System_Parameters.md    # Table with all params
│       └── Mathematical_Model.md   # Mathematical modeling of the problem
│
├── Tools/                          # Automation scripts and build utilities
│   └── generate_config.py          # Propagates JSON config to C++, MATLAB and md
│
├── Firmware/                       # ESP32 Embedded Code (PlatformIO Project)
│   ├── platformio.ini              # Build Configuration: Board settings, libraries, and compiler flags
│   ├── include/
│   │   └── Config.h                # Costants
│   │
│   ├── src/
│   │   └── main.cpp                # Application Entry Point: Task scheduling and Setup
│   │
│   └── lib/                        # Custom Modular Libraries (Object-Oriented Architecture)
│       ├── Aether_HAL/             # Hardware Abstraction Layer
│       │   ├── Aether_HAL.h        # Interface definition for Sensors (Hall) and Actuators (PWM)
│       │   └── Aether_HAL.cpp      # Implementation of hardware-specific drivers
│       │
│       ├── Aether_Control/         # Control Logic (Hardware Agnostic)
│       │   ├── PID_Controller.h    # Discrete PID algorithm class definition
│       │   └── PID_Controller.cpp  # Implementation of the control loop math
│       │
│       └── Aether_System/          # System Management
│           ├── StateMachine.h      # Finite State Machine (FSM) definition (Idle, Levitation, Error)
│           └── SafetyManager.h     # Emergency handling and fault detection logic
│
├── Hardware/                       # Physical Design and Component Specifications
│   ├── Datasheets/
│   │   ├── SS49E_Hall_Sensor.pdf   # Linear Hall-effect sensor specifications
│   │   └── IRLZ44N_MOSFET.pdf      # Logic-level MOSFET specifications
│   ├── Mechanical/
│   │   └── Mount_Support.stl       # 3D printable file for the coil/sensor support
│   └── Schematics/
│       └── wiring_diagram.pdf      # Circuit diagram and connection map
│
└── Simulation/                     # Model-Based Design (Digital Twin)
    ├── MATLAB/
    │   └── load_params.m # Script for analyzing system stability and plotting results
    └── Simulink/
        └── MagLev_ClosedLoop.slx   # Simulink model for tuning PID gains and simulating physics