% AUTO-GENERATED FILE from project_config.json
% Run this script to load parameters into the workspace

%% Physical Plant (Lumped)
mass = 0.016;
g = 9.81;
x_eq = 0.03;
R_coil = 15.0;
L_coil = 0.01;
K_mag_approx = 0.0001;

%% Coil Geometry (For Advanced Modeling)
geom_R1 = 0.004;
geom_R2 = 0.0125;
geom_L = 0.02;
geom_N = 1000;

%% Control System
Ts = 1 / 5000; % Sampling Time
Kp = 1000.0;
Ki = 50.0;
Kd = 25.0;

disp('Aether-Lock Parameters Loaded Successfully');
