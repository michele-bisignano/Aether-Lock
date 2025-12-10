%% Aether-Lock: Physics Identification & Model Matching
% This script uses symbolic computation (Ground Truth - Thick Solenoid) to:
% 1. Identify the parameters (n, K) for the simplified control model.
% 2. Calculate the required Equilibrium Current.
% 3. Extract the linearized coefficients (kx, ki) for the PID design.

clear; clc; close all;

%% 1. Load Parameters (SSOT)
if exist('load_params.m', 'file')
    load_params;
else
    error('File load_params.m not found. Please run the Python generation script first.');
end

% Initial parameters for geometric fitting
I_guess = 0.0794;      % Test current for shape analysis [A]
mu_val = 4*pi*1e-7 * core_amp_factor; % Effective Permeability [H/m]

%% 2. Symbolic Model (Ground Truth - Thick Solenoid)
syms z real       % Distance
syms L R1 R2 real % Geometry
syms mu0 N I real % Physics
syms m_mag real   % Dipole Moment

% --- B-Field Formula (Thick Solenoid) ---
num_pos = R2 + sqrt(R2^2 + (z + L/2)^2);
den_pos = R1 + sqrt(R1^2 + (z + L/2)^2);
term1 = (z + L/2) * log(num_pos / den_pos);

num_neg = R2 + sqrt(R2^2 + (z - L/2)^2);
den_neg = R1 + sqrt(R1^2 + (z - L/2)^2);
term2 = (z - L/2) * log(num_neg / den_neg);

PreFactor = (mu0 * N * I) / (2 * L * (R2 - R1));
Bz_sym = PreFactor * (term1 - term2);

% --- Force Calculation (Gradient) ---
% F = m * (dB/dz)
Force_sym = m_mag * diff(Bz_sym, z);

fprintf('✅ Symbolic Model constructed successfully.\n');

%% 3. Identification of 'n' and 'K' (Fitting)
% Create a numeric function to simulate reality
F_num_func = matlabFunction(Force_sym, 'Vars', {z, L, R1, R2, mu0, N, I, m_mag});

% Generate data points around equilibrium using the guess current
range = 0.005; 
z_vector = linspace(x_eq - range, x_eq + range, 200);
F_data_guess = abs(F_num_func(z_vector, geom_L, geom_R1, geom_R2, mu_val, geom_N, I_guess, m_mag_val));

% Log-Log Regression: ln(F) = ln(K_tot) - n*ln(z)
Y = log(F_data_guess);
X = log(z_vector);
coeffs = polyfit(X, Y, 1);

n_identified = -coeffs(1);           % Exponent n

% Force in x_eq
F_true_at_eq = abs(F_num_func(x_eq, geom_L, geom_R1, geom_R2, mu_val, geom_N, I_guess, m_mag_val));

% Inverse formula: K = (F * x^n) / I
K_mag_identified = (F_true_at_eq * (x_eq^n_identified)) / I_guess;

fprintf('\n=== IDENTIFICATION RESULTS ===\n');
fprintf('Identified Exponent (n) : %.4f\n', n_identified);
fprintf('Physical Constant (K)   : %.4e\n', K_mag_identified);

%% 4. Equilibrium Current Calculation
% Balance: F_mag = F_gravity
% (K * I) / x^n = m * g
% I = (m * g * x^n) / K

F_weight = mass * g;
I_eq_calc = (F_weight * (x_eq^n_identified)) / K_mag_identified;

fprintf('\n=== REAL OPERATING POINT ===\n');
fprintf('Target Weight Force     : %.4f N\n', F_weight);
fprintf('Required Current (I_eq) : %.4f A\n', I_eq_calc);

%% 5. Linearization (Taylor) at Operating Point
% Recalculate coefficients using the REAL required current

% A. Position Stiffness (k_x)
% Using the derivative of the simplified model: dF/dz = -n * F / z
k_x = -n_identified * F_weight / x_eq; 
% Note: k_x is physically negative (force decreases with distance),
% but in the differential equation (m*a = mg - F), it becomes a positive
% term driving instability. We use absolute value for the model A matrix.

% B. Current Gain (k_i)
% dF/dI = F / I
k_i = F_weight / I_eq_calc;

fprintf('\n=== LINEAR MODEL COEFFICIENTS ===\n');
fprintf('k_x (Stiffness)     : %.4f N/m\n', abs(k_x));
fprintf('k_i (Current Gain)  : %.4f N/A\n', k_i);

% Open Loop Poles
lambda = sqrt(abs(k_x) / mass);
fprintf('Unstable Pole       : +%.2f rad/s\n', lambda);

%% 6. Final Verification Plot (Linear Scale)
% Recalculate "True" force (Pisa model) using the CALCULATED equilibrium current
F_real_final = abs(F_num_func(z_vector, geom_L, geom_R1, geom_R2, mu_val, geom_N, I_eq_calc, m_mag_val));

% Calculate "Simplified Model" force using the calculated current
F_simple_final = (K_mag_identified * I_eq_calc) ./ (z_vector .^ n_identified);

figure('Name', 'Model Validation');
plot(z_vector*1000, F_real_final, 'b', 'LineWidth', 2); hold on;
plot(z_vector*1000, F_simple_final, 'r--', 'LineWidth', 2);
yline(F_weight, 'g-.', 'Weight Force');
xline(x_eq*1000, 'k:', 'Equilibrium');

grid on;
legend('Real Model (Thick Solenoid)', 'Simplified Model (PID)', 'Gravity');
xlabel('Distance [mm]');
ylabel('Force [N]');
title(sprintf('Model Comparison at I = %.2f A', I_eq_calc));
subtitle(sprintf('Check the overlap between Blue and Red lines at the vertical black line'));

%% 7. Wide Range Plot (0 to 1 Meter)
% Visualization of the global force behavior.
% WARNING: We start at 0.005 (5mm) to avoid division by zero.

z_wide = linspace(0.005, 1.0, 1000); % Vector from 1mm to 1 meter

% Recalculate forces over the entire range
F_real_wide = abs(F_num_func(z_wide, geom_L, geom_R1, geom_R2, mu_val, geom_N, I_eq_calc, m_mag_val));
F_simple_wide = (K_mag_identified * I_eq_calc) ./ (z_wide .^ n_identified);
figure('Name', 'Global Force Behavior');

% --- Subplot 1: Linear Scale (What you asked for) ---
subplot(2,1,1);
plot(z_wide, F_real_wide, 'b', 'LineWidth', 2); hold on;
plot(z_wide, F_simple_wide, 'r--', 'LineWidth', 2);
yline(F_weight, 'g-.', 'Target Weight');

% Limit the Y-axis to see something useful (the peak at 1mm hides everything else)
% Showing only the first 20cm
ylim([0, F_weight * 5]); 
xlim([0, 0.2]); 
xlabel('Distance [m]'); ylabel('Force [N]');
title('Linear Scale (Zoom on first 20cm)');
legend('Real Model', 'Simplified Model (Ki/x^n)');
grid on;

% --- Subplot 2: Log-Log Scale (To see the full 1m range) ---
subplot(2,1,2);
loglog(z_wide, F_real_wide, 'b', 'LineWidth', 2); hold on;
loglog(z_wide, F_simple_wide, 'r--', 'LineWidth', 2);
yline(F_weight, 'g-.');

xlabel('Distance [m]'); ylabel( 'Force [N]');
title('Log-Log Scale (Full 0-1m Range)');
grid on;