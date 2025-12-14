%% Aether-Lock: PID Design & Tuning
% Calculate gains based EXCLUSIVELY on loaded and identified parameters.
% PREREQUISITE: Run 'identify_physics.m' before this script to have k_x and k_i.

clc;

%% 1. Parameter Loading (SSOT)
if exist('load_params.m', 'file')
    load_params;
else
    error('File load_params.m missing. Run the Python script.');
end

% Check existence of identified parameters
if ~exist('k_x', 'var') || ~exist('k_i', 'var')
    error('Parameters k_x and k_i not found. Run script "identify_physics.m" first!');
end

fprintf('--- PID DESIGN STARTED ---\n');
fprintf('Mass: %.4f kg | Stiffness k_x: %.4f | Gain k_i: %.4f\n', mass, k_x, k_i);

%% 2. Hardware Chain Gain Calculation
% Estimated Sensor Sensitivity (based on physical data if not calibrated)
% Here we use a theoretical estimate based on the B-field at the equilibrium point.
% B_eq was calculated in identify_physics; we recalculate it or use a nominal value.
% For now, we use a generic approach: We want to map the +/- 5mm range to +/- 0.5V
S_v_estimated = 20.0; % [Volts/m] (Example: 20V per meter -> 0.2V per cm)

% Total Gain: [Meters] -> [Volts] -> [ADC Bits] -> [PID] -> [PWM Bits] -> [Amps]
% K_sensor_adc = (Volt/Meter) * (Bit_ADC / V_ref)
K_sensor_adc = S_v_estimated * (ADC_max / V_ref); 

% K_actuator = (V_supply / R_coil) / PWM_max  [Amps / Bit_PWM]
I_max_driver = 12.0 / R_coil; 
K_actuator = I_max_driver; % Since our PID outputs 0-1, we consider 1 = I_max

fprintf('Driver Max Current: %.2f A\n', I_max_driver);

%% 3. PID Design (Pole Placement)
% Objective: Stabilize the unstable system.
% Unstable pole alpha = sqrt(k_x / m)
alpha = sqrt(abs(k_x) / mass);

% We choose bandwidth > alpha (at least 1.5x or 2x)
omega_c = alpha * 2.0; 
damping = 0.75; 

fprintf('Target Bandwidth: %.2f rad/s (Natural instability: %.2f rad/s)\n', omega_c, alpha);

% Physical Formulas (in SI units: Newtons, Meters, Seconds)
Kp_phys = (mass * omega_c^2 + k_x) / k_i;
Kd_phys = (2 * damping * omega_c * mass) / k_i;
Ki_phys = Kp_phys * 5.0; % Fast integral for static disturbances

%% 4. Normalization for Firmware
% The firmware works with normalized units (PID output 0.0-1.0)
% We must scale the physical gains.

% KP_fw: [Output Unit] / [Error Unit]
% [Output Unit] = 1.0 (corresponds to I_max_driver Amps)
% [Error Unit] = 1.0 (corresponds to 1 Meter in the error calculated in meters)
% Therefore, we divide by the driver current gain
Kp_fw = Kp_phys / I_max_driver;
Kd_fw = Kd_phys / I_max_driver;
Ki_fw = Ki_phys / I_max_driver;

fprintf('\n=== PID SUGGESTED GAINS (Copy to JSON) ===\n');
fprintf('Kp : %.2f\n', Kp_fw);
fprintf('Ki : %.2f\n', Ki_fw);
fprintf('Kd : %.2f\n', Kd_fw);

fprintf('\nNote: These values are the theoretical starting point.\n');
fprintf('      Fine tuning will need to be done experimentally.\n');