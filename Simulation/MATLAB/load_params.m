% AUTO-GENERATED FILE from project_config.json
% Run this script to load parameters into the workspace

%% Physical Plant
mass = 0.016;
g = 9.81;
x_eq = 0.02;
R_coil = 15.0;
L_coil = 0.01;
K_mag = 0.0001;

%% Control System
Ts = 1 / 5000; % Sampling Time
Kp = 1000.0;
Ki = 50.0;
Kd = 25.0;

disp('Aether-Lock Parameters Loaded Successfully');
