%% Aether-Lock: Physics Identification & Taylor Analysis
clear; clc; close all;

%% 1. Load Parameters (Single Source of Truth)
% This must load ALL necessary variables. If it fails here,
% it means the Python script was not executed or is incomplete.
if exist('load_params.m', 'file')
    load_params;
else
    error('File load_params.m not found. Run the Python script first.');
end

% Extra parameters for graphical fitting
I_guess = 0.0794;

%% 2. Symbolic Model (Ground Truth)
syms z real       % Distance
syms L R1 R2 real % Geometry
syms mu0 N I real % Physics
syms m_mag real   % Dipole

% --- B-Field Formula (Thick Solenoid) ---
num_pos = R2 + sqrt(R2^2 + (z + L/2)^2);
den_pos = R1 + sqrt(R1^2 + (z + L/2)^2);
term1 = (z + L/2) * log(num_pos / den_pos);

num_neg = R2 + sqrt(R2^2 + (z - L/2)^2);
den_neg = R1 + sqrt(R1^2 + (z - L/2)^2);
term2 = (z - L/2) * log(num_neg / den_neg);

PreFactor = (mu0 * N * I) / (2 * L * (R2 - R1));
Bz_sym = PreFactor * (term1 - term2);

% Apply Core Amplification Factor (Read from load_params)
Bz_sym = Bz_sym * core_amp_factor;

% --- Force Calculation ---
Force_sym = m_mag * diff(Bz_sym, z);

fprintf('✅ Symbolic Model built.\n');

%% --- Display Equation with Numbers ---
% Substitute the constants loaded from load_params
Force_with_numbers = subs(Force_sym, ...
    {L, R1, R2, mu0, N, m_mag}, ...
    {geom_L, geom_R1, geom_R2, mu0_val, geom_N, m_mag_val});

fprintf('\n=== FORCE EQUATION (Substituted Parameters) ===\n');
fprintf('F(z, I) = \n');
pretty(vpa(Force_with_numbers, 3)); 
fprintf('==============================================\n');

%% 3. Identification of 'n' and 'K' (Fitting)
F_num_func = matlabFunction(Force_sym, 'Vars', {z, L, R1, R2, mu0, N, I, m_mag});

% Fitting range
range = 0.005; 
z_vector = linspace(x_eq - range, x_eq + range, 200);
F_data_guess = abs(F_num_func(z_vector, geom_L, geom_R1, geom_R2, mu0_val, geom_N, I_guess, m_mag_val));

% Log-Log Regression
Y = log(F_data_guess);
X = log(z_vector);
coeffs = polyfit(X, Y, 1);

n_identified = -coeffs(1);
K_tot_fit = exp(coeffs(2));
K_mag_identified = K_tot_fit / I_guess;

fprintf('\n=== IDENTIFICATION RESULTS ===\n');
fprintf('Identified exponent n : %.4f\n', n_identified);
fprintf('Physical constant K   : %.4e\n', K_mag_identified);

%% 4. Calculation of the TRUE Equilibrium Current
F_weight = mass * g;
I_eq_calc = (F_weight * (x_eq^n_identified)) / K_mag_identified;

fprintf('\n=== ACTUAL OPERATING POINT ===\n');
fprintf('Weight Force        : %.4f N\n', F_weight);
fprintf('Equilibrium Current : %.4f A\n', I_eq_calc);

%% 5. Linearization (Taylor)
k_x = -n_identified * F_weight / x_eq; 
k_i = F_weight / I_eq_calc;

fprintf('\n=== LINEAR MODEL COEFFICIENTS ===\n');
fprintf('k_x (Stiffness)     : %.4f N/m\n', abs(k_x));
fprintf('k_i (Current Gain)  : %.4f N/A\n', k_i);
lambda = sqrt(abs(k_x) / mass);
fprintf('Unstable poles      : +/- %.2f rad/s\n', lambda);

%% 6. Graphical Verification
F_real_final = abs(F_num_func(z_vector, geom_L, geom_R1, geom_R2, mu0_val, geom_N, I_eq_calc, m_mag_val));
F_simple_final = (K_mag_identified * I_eq_calc) ./ (z_vector .^ n_identified);

figure('Name', 'Model Validation');
plot(z_vector*1000, F_real_final, 'b', 'LineWidth', 2); hold on;
plot(z_vector*1000, F_simple_final, 'r--', 'LineWidth', 2);
yline(F_weight, 'g-.', 'Target');
xline(x_eq*1000, 'k:', 'Equilibrium');
grid on; legend('Real', 'Simple', 'Weight');
xlabel('Distance [mm]'); ylabel('Force [N]');
title(sprintf('Comparison at I = %.2f A', I_eq_calc));

%% 8. Sensor Calibration & Linearization
% Use variables loaded from load_params (without an exist check)
Sens_Hall_VT = Hall_Sens_mV_G * 10; 

% Derivative of the B-field with respect to z (calculated at the equilibrium point)
Bz_eq_sym = subs(Bz_sym, I, I_eq_calc);
dB_dz_sym = diff(Bz_sym, z);
dB_dz_val = double(subs(dB_dz_sym, {z, I, L, R1, R2, mu0, N, m_mag}, ...
                                   {x_eq, I_eq_calc, geom_L, geom_R1, geom_R2, mu0_val, geom_N, m_mag_val}));

% B and Volts at equilibrium
B_at_eq = double(subs(Bz_eq_sym, {z, L, R1, R2, mu0, N, m_mag}, ...
                                 {x_eq, geom_L, geom_R1, geom_R2, mu0_val, geom_N, m_mag_val}));
V_at_eq = V_zero_theoretical + (B_at_eq * Sens_Hall_VT);

% Resulting sensitivity
dV_dz = Sens_Hall_VT * dB_dz_val; 
K_sens_code = 1 / dV_dz; 

fprintf('\n=== SENSOR CALIBRATION (Theoretical) ===\n');
fprintf('B-Field at %.1f mm     : %.4f Tesla\n', x_eq*1000, B_at_eq);
fprintf('Voltage at %.1f mm     : %.4f V\n', x_eq*1000, V_at_eq);
fprintf('Inverse Sensitivity    : %.6f m/V\n', K_sens_code);