%% Aether-Lock: Advanced Physics Validation & Calibration
% Author: Michele Bisignano
% 
% This script integrates:
% 1. Measured Sensor Data (from R)
% 2. Theoretical Ground Truth (Biot-Savart Integration)
% 3. Modeled simplified physics (Power Law Fitting)

clear; clc; close all;

%% 1. EXPERIMENTAL DATA & LOAD PARAMETERS
% Enter the values measured during your physical drop-off test
I_measured_real = 0.187; % [A] Measured current when the angel falls
z_drop_test_m   = 0.020;  % [m] Distance during that specific test

% Load theoretical constants (Single Source of Truth)
if exist('load_params.m', 'file')
    load_params;
else
    error('load_params.m missing. Run the Python configuration script first.');
end

F_weight = mass * g; % The force we need to balance (Gravity)

%% 2. ROBUST SENSOR DATA IMPORT
csv_path = '../../Hardware/Measurements/Output/sensor_summary_report.csv';
if ~exist(csv_path, 'file'), error('CSV summary report not found.'); end

opts = detectImportOptions(csv_path);
opts.VariableNamingRule = 'preserve';
opts.VariableTypes = repmat({'string'}, 1, length(opts.VariableNames)); 
data_table = readtable(csv_path, opts);

clean_to_double = @(col) str2double(strrep(col, ',', '.'));
all_cols = data_table.Properties.VariableNames;
col_dist_name  = all_cols{contains(all_cols, 'Distance')};
col_adc_name   = all_cols{contains(all_cols, 'Mean_Raw')};
col_label_name = all_cols{contains(all_cols, 'Label')};

% Identify Resting Bias
idx_bias = find(string(data_table.(col_label_name)) == "inf");
adc_bias_measured = clean_to_double(data_table.(col_adc_name)(idx_bias));

% Operational vectors
dist_m = clean_to_double(data_table.(col_dist_name)) / 1000.0;
adc_vals = clean_to_double(data_table.(col_adc_name));
valid = (dist_m < 0.09) & ~isnan(dist_m) & ~isnan(adc_vals); % Exclude inf
[dist_sorted, idx_sort] = sort(dist_m(valid));
adc_sorted = adc_vals(valid(idx_sort));

fprintf('✅ Sensor Data Loaded. Resting Bias: %.2f ADC\n', adc_bias_measured);

%% 3. SYMBOLIC THEORY (Biot-Savart Integration)
syms z real
syms L R1 R2 mu0 N I m_mag real

% Integration formula for a Thick Solenoid B-field
num_pos = R2 + sqrt(R2^2 + (z + L/2)^2);
den_pos = R1 + sqrt(R1^2 + (z + L/2)^2);
num_neg = R2 + sqrt(R2^2 + (z - L/2)^2);
den_neg = R1 + sqrt(R1^2 + (z - L/2)^2);

Bz_sym = (mu0 * N * I) / (2 * L * (R2 - R1)) * ...
         ((z + L/2) * log(num_pos / den_pos) - (z - L/2) * log(num_neg / den_neg));

% Theoretical Force = m * grad(B)
Force_sym = abs(m_mag * diff(Bz_sym, z));
F_air_theory_func = matlabFunction(Force_sym, 'Vars', {z, L, R1, R2, mu0, N, I, m_mag});

% Calculate Core Amplification Factor
F_air_at_test = F_air_theory_func(z_drop_test_m, geom_L, geom_R1, geom_R2, mu0_val, geom_N, I_measured_real, m_mag_val);
Calculated_Core_Factor = F_weight / F_air_at_test;

%% 4. SIMPLIFIED MODEL IDENTIFICATION (Fitting Ki/z^n)
% Generate points from the theoretical curve to find the exponent 'n'
z_fit = linspace(x_eq - 0.005, x_eq + 0.005, 100);
F_shape = F_air_theory_func(z_fit, geom_L, geom_R1, geom_R2, mu0_val, geom_N, 1.0, 1.0);
coeffs = polyfit(log(z_fit), log(F_shape), 1);
n_id = -coeffs(1);

% Find 'K' by anchoring the model to the REAL experimental drop point
K_id = (F_weight * (z_drop_test_m^n_id)) / I_measured_real;

fprintf('\n=== IDENTIFICATION RESULTS ===\n');
fprintf('Identified Exponent (n)  : %.4f\n', n_id);
fprintf('Identified Constant (K)  : %.4e\n', K_id);
fprintf('Core Amp Factor          : %.2f\n', Calculated_Core_Factor);
fprintf('\nFINAL POWER-LAW FORMULA:\n');
fprintf('F_mag(z, i) = (%.4e * i) / z^%.2f\n', K_id, n_id);

%% 5. LINEARIZATION (Taylor Coefficients)
k_x = -n_id * F_weight / x_eq; 
k_i = F_weight / I_measured_real;
fprintf('\n=== LINEARIZATION COEFFICIENTS ===\n');
fprintf('k_x (Stiffness) : %.4f N/m\n', abs(k_x));
fprintf('k_i (Gain)      : %.4f N/A\n', k_i);

%% 6. VISUAL VALIDATION PLOTS
figure('Name', 'Aether-Lock: Integrated Calibration', 'Position', [100 100 1200 500]);

% --- PLOT A: Sensor Characterization ---
subplot(1,2,1);
plot(dist_sorted*1000, adc_sorted, 'b-o', 'LineWidth', 1.5, 'MarkerFaceColor', 'b'); hold on;
target_adc = interp1(dist_sorted, adc_sorted, x_eq, 'pchip');
plot(x_eq*1000, target_adc, 'rs', 'MarkerSize', 10, 'MarkerFaceColor', 'r');
yline(adc_bias_measured, 'k--', 'Resting Bias');
xlabel('Distance [mm]'); ylabel('ADC Value (12-bit)');
title('Measured Sensor Calibration');
legend('Measured Curve', 'Equilibrium Setpoint', 'Bias (Zero Field)');
grid on;

% --- PLOT B: Physics Comparison ---
subplot(1,2,2);
z_plot = linspace(0.008, 0.045, 500); % 8mm to 45mm

% 1. Pure Theory (Air, No Core)
F_theory_air = F_air_theory_func(z_plot, geom_L, geom_R1, geom_R2, mu0_val, geom_N, I_measured_real, m_mag_val);

% 2. Modeled Model (Calibrated Simplified Formula)
F_modeled = (K_id * I_measured_real) ./ (z_plot .^ n_id);

plot(z_plot*1000, F_theory_air, 'k:', 'LineWidth', 1.5); hold on;
plot(z_plot*1000, F_modeled, 'r-', 'LineWidth', 2.5);
plot(z_drop_test_m*1000, F_weight, 'go', 'MarkerSize', 12, 'MarkerFaceColor', 'g'); % Measured drop
yline(F_weight, 'k--');

xlabel('Distance [mm]'); ylabel('Force [N]');
title('Force Model vs. Theoretical Physics');
legend('Theory (Biot-Savart - AIR)', 'Calibrated Model (Power Law)', 'Real Experimental Point');
grid on;
ylim([0, F_weight * 4]);