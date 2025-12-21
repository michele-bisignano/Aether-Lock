%% Aether-Lock: Full System Calibration (Robust Version)
% 1. Load theoretical parameters from JSON.
% 2. Import SUMMARY data from CSV with forced type conversion.
% 3. Calculate Sensor Calibration and Core Factor.

clear; clc; close all;

%% 1. MANUAL CONFIGURATION
% Enter the experimental data from your physical Drop-off test
I_drop_test_Amps = 0.187;  % <--- YOUR MEASURED CURRENT [A]
z_drop_test_m = 0.2;     % Measurement distance during the test [m]

%% 2. LOAD JSON PARAMETERS
% Load physical and geometric constants via the Python-generated script
if exist('load_params.m', 'file')
    load_params;
else
    error('load_params.m missing. Please run the Python configuration script first.');
end

%% 3. ROBUST DATA IMPORT
csv_path = '../../Hardware/Measurements/Output/sensor_summary_report.csv';

if ~exist(csv_path, 'file')
    error('CSV file not found at: %s', csv_path);
end

fprintf('Reading sensor summary report...\n');

% Load everything as strings to prevent mixed-format (comma vs point) errors
opts = detectImportOptions(csv_path);
opts.VariableNamingRule = 'preserve';
opts.VariableTypes = repmat({'string'}, 1, length(opts.VariableNames)); 
data_table = readtable(csv_path, opts);

% Helper function to clean European decimal commas and convert to double
clean_to_double = @(col) str2double(strrep(col, ',', '.'));

% Dynamic Column Detection (Independent of Case/Exact format)
all_cols = data_table.Properties.VariableNames;
col_dist_name  = all_cols{contains(all_cols, 'Distance')};
col_adc_name   = all_cols{contains(all_cols, 'Mean_Raw')};
col_label_name = all_cols{contains(all_cols, 'Label')};

% Extract raw vectors as strings
raw_dist_str = data_table.(col_dist_name);
raw_adc_str  = data_table.(col_adc_name);
raw_labels   = data_table.(col_label_name);

% --- A. DETECT RESTING BIAS (Zero Field Value) ---
% Look for the row containing the "inf" (infinity) label
idx_bias = find(raw_labels == "inf");

if isempty(idx_bias)
    error('Could not find row with Label "inf" (Resting Bias) in CSV.');
end

% Convert the Bias ADC value to numerical format
adc_bias_measured = clean_to_double(raw_adc_str(idx_bias));

if isnan(adc_bias_measured)
    error('Error parsing Bias value (Value is not a valid number).');
end

% --- B. CLEAN AND FILTER DATA VECTORS ---
% Convert data columns to double
dist_vals_mm = clean_to_double(raw_dist_str);
adc_vals_raw = clean_to_double(raw_adc_str);

% Filter valid operational data (Exclude NaNs and infinity markers > 100mm)
valid_mask = ~isnan(dist_vals_mm) & ~isnan(adc_vals_raw) & (dist_vals_mm < 100);

% Create final double vectors for interpolation
dist_m = dist_vals_mm(valid_mask) / 1000.0; % Convert mm to meters
adc_vals = adc_vals_raw(valid_mask);

% Sort vectors (interp1 requires strictly increasing X-axis)
[dist_sorted, idx_sort] = sort(dist_m);
adc_sorted = adc_vals(idx_sort);

% Safety check: ensure sufficient data points exist
if length(dist_sorted) < 2
    error('Insufficient valid data points found in CSV after conversion.');
end

fprintf('Data loaded successfully. Detected Resting Bias: %.2f\n', adc_bias_measured);

%% 4. SENSOR PARAMETER CALCULATION
% A. Compute Quiescent Output (Bias in Volts)
bias_volts_calc = adc_bias_measured * (V_ref / ADC_max);

% B. Compute Target ADC value at equilibrium_distance_m
target_dist = x_eq; % From JSON (e.g., 0.02m)
target_adc = interp1(dist_sorted, adc_sorted, target_dist, 'pchip');

% C. Compute Local Sensitivity (m/V) via numerical derivative
delta_x = 0.001; % 1mm delta
adc_p = interp1(dist_sorted, adc_sorted, target_dist - delta_x, 'pchip'); 
adc_m = interp1(dist_sorted, adc_sorted, target_dist + delta_x, 'pchip'); 

dV = (adc_p - adc_m) * (V_ref / ADC_max);
dM = 2 * delta_x;
sens_m_per_v = dM / abs(dV);

% D. Security Thresholds computation
adc_at_3cm = interp1(dist_sorted, adc_sorted, 0.03, 'pchip');
idle_threshold = round(target_adc - adc_at_3cm);
safety_min = round(adc_at_3cm); 
safety_max = round(interp1(dist_sorted, adc_sorted, 0.01, 'pchip')); 

fprintf('\n=== 1. SENSOR CALIBRATION (Data-Driven) ===\n');
fprintf('Resting Bias (Inf)  : %.3f V (ADC: %.0f)\n', bias_volts_calc, adc_bias_measured);
fprintf('Target ADC at %.3fm : %d\n', target_dist, round(target_adc));
fprintf('Inverse Sensitivity : %.4f m/V\n', sens_m_per_v);

%% 5. CORE AMPLIFICATION FACTOR CALCULATION
% Using the integration of the Biot-Savart Law for Thick Solenoids
syms z_sym z L R1 R2 mu0 N I m_dipole real

% Thick Solenoid Formula
num_pos = R2 + sqrt(R2^2 + (z + L/2)^2);
den_pos = R1 + sqrt(R1^2 + (z + L/2)^2);
term1 = (z + L/2) * log(num_pos / den_pos);
term2 = (z - L/2) * log((R2 + sqrt(R2^2 + (z - L/2)^2)) / (R1 + sqrt(R1^2 + (z - L/2)^2)));
PreFactor = (mu0 * N * I) / (2 * L * (R2 - R1));
Bz_sym = PreFactor * (term1 - term2);
Force_sym = m_dipole * diff(Bz_sym, z);

F_func = matlabFunction(Force_sym, 'Vars', {z, L, R1, R2, mu0, N, I, m_dipole});

% Calculate theoretical force in Air (Mu_r = 1) at the Drop-off point
F_theory_air = abs(F_func(z_drop_test_m, geom_L, geom_R1, geom_R2, mu0_val, geom_N, I_drop_test_Amps, m_mag_val));

% Real required force (Gravity/Weight)
F_gravity = mass * g;

% Core efficiency factor (Real / Theoretical Air)
Calculated_Core_Factor = F_gravity / F_theory_air;

fprintf('\n=== 2. PHYSICAL SYSTEM CALIBRATION ===\n');
fprintf('Total System Mass   : %.4f kg\n', mass);
fprintf('Gravity Force (Wt)  : %.4f N\n', F_gravity);
fprintf('Theory Force (Air)  : %.4f N (at %.3f A)\n', F_theory_air, I_drop_test_Amps);
fprintf('>>> CORE AMP FACTOR : %.2f <<<\n', Calculated_Core_Factor);

%% 6. JSON CONFIGURATION OUTPUT
fprintf('\n--- COPY THESE VALUES INTO PROJECT_CONFIG.JSON ---\n');
fprintf('"quiescent_output_v": %.3f,\n', bias_volts_calc);
fprintf('"target_adc_value": %d,\n', round(target_adc));
fprintf('"sensitivity_m_per_v": %.4f,\n', sens_m_per_v);
fprintf('"core_amplification_factor": %.1f,\n', Calculated_Core_Factor);
fprintf('"idle_threshold_adc": %d,\n', idle_threshold);
fprintf('"safety_min_adc": %d,\n', safety_min);
fprintf('"safety_max_adc": %d\n', safety_max);

%% 7. VERIFICATION PLOT
figure('Name', 'Sensor Calibration Curve');
plot(dist_sorted*1000, adc_sorted, 'b-o', 'LineWidth', 1.5); hold on;
yline(target_adc, 'g--', 'Target Setpoint');
yline(adc_at_3cm, 'r--', 'Idle Threshold');
xline(target_dist*1000, 'g:');
xlabel('Distance [mm]'); ylabel('ADC Value (Mean)');
title('Sensor Characteristic Curve (Experimentally Derived)');
grid on;