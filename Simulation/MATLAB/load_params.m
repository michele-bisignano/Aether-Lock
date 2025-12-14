% AUTO-GENERATED FILE from project_config.json
% Run this script to load parameters into the workspace

%% Physical Plant (Lumped)
mass = 0.007;
g = 9.81;
x_eq = 0.03;
R_coil = 36.0;
L_coil = 0.01;

%% Magnetic Properties
m_mag_val = 0.1511; 
core_amp_factor = 40.0;

%% Coil Geometry
geom_R1 = 0.006;
geom_R2 = 0.00825;
geom_L = 0.02;
geom_N = 1000;
mu0_val = 4*pi*1e-7;

%% Sensor & Hardware Specs
V_supply = 3.3;
Hall_Sens_mV_G = 1.4; 
Sensor_Offset = 0.01;
V_zero_theoretical = 1.65;
ADC_max = 4095;
V_ref = 3.3;
PWM_max = 1023;


%% Control System Targets & Initial Gains
Ts = 1 / 5000;
Kp_config = 101.85;
Ki_config = 509.24;
Kd_config = 2.72;

disp('✅ Aether-Lock Parameters Loaded Successfully');
