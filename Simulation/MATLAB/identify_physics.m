%% Aether-Lock: Physics Identification & Taylor Expansion
% This script uses the "Ground Truth" model (Thick Solenoid) to:
% 1. Find the best exponent 'n' for the simplified model.
% 2. Calculate the 2nd order Taylor expansion around equilibrium.

clear; clc; close all;

%% 1. Load Parameters (Single Source of Truth)
% Load data generated from the JSON
load_params; 

% Parameters for symbolic calculation (Nominal values)
I_eq_guess = 0.5;   % Estimated equilibrium current (A)
mu0_val = 4*pi*1e-7;
m_mag_val = 1;      % Unitary value for shape fitting

%% 2. Symbolic Definition
syms z real       % Distance variable
syms L R1 R2 real % Geometry
syms mu0 N I real % Electromagnetism
syms m_mag real   % Dipole

%% 3. B-Field Formula (Thick Solenoid)
% Partial terms for readability
num_pos = R2 + sqrt(R2^2 + (z + L/2)^2);
den_pos = R1 + sqrt(R1^2 + (z + L/2)^2);
term1 = (z + L/2) * log(num_pos / den_pos);

num_neg = R2 + sqrt(R2^2 + (z - L/2)^2);
den_neg = R1 + sqrt(R1^2 + (z - L/2)^2);
term2 = (z - L/2) * log(num_neg / den_neg);

PreFactor = (mu0 * N * I) / (2 * L * (R2 - R1));
Bz_sym = PreFactor * (term1 - term2);

% Force Calculation (Gradient)
Force_sym = m_mag * diff(Bz_sym, z);
latex(Force_sym)
%% 4. Numerical Analysis to Find 'n'
% Create a numerical function by substituting the loaded geometric parameters
F_num_handle = matlabFunction(Force_sym, 'Vars', {z, L, R1, R2, mu0, N, I, m_mag});

% Generate a spatial vector around the equilibrium point
z_vector = linspace(x_eq * 0.5, x_eq * 1.5, 100); 
F_values = F_num_handle(z_vector, geom_L, geom_R1, geom_R2, mu0_val, geom_N, I_eq_guess, m_mag_val);

% Log-log slope calculation (Exponent n)
% ln(F) = C - n*ln(z)  ->  d(lnF)/d(lnz) = -n
idx = find(z_vector >= x_eq, 1);
log_F = log(abs(F_values));
log_z = log(z_vector);
slope = (log_F(idx+1) - log_F(idx)) / (log_z(idx+1) - log_z(idx));
n_identified = -slope;

fprintf('------------------------------------------------\n');
fprintf('Equilibrium Point: %.3f m\n', x_eq);
fprintf('Identified Exponent (n): %.4f\n', n_identified);
fprintf('------------------------------------------------\n');

%% 5. Taylor Expansion (Symbolic & Numeric)
% Expand the force F(z) around equilibrium.
% Taylor Series: F(z) ≈ F(z0) + F'(z0)*(z - z0) + 0.5*F''(z0)*(z - z0)^2

% Define a symbolic variable for the equilibrium position
syms z0 real 

% --- A. LITERAL (SYMBOLIC) TAYLOR EXPANSION ---
fprintf('Calculating Literal Taylor Expansion (this might take a moment)...\n');

% Calculate Taylor series of Force_sym with respect to 'z' around point 'z0'
% 'Order' 3 includes terms up to (z-z0)^2
Taylor_F_literal = taylor(Force_sym, z, 'ExpansionPoint', z0, 'Order', 3);

% Simplify the result to make the LaTeX output slightly more compact
% (Note: For a thick solenoid, this will still be a very large formula)
Taylor_F_literal = simplify(Taylor_F_literal);

fprintf('Literal Taylor calculated.\n');
fprintf('\n--- LaTeX Code for Literal Expansion ---\n');
disp(Taylor_F_literal);
fprintf('----------------------------------------\n');

% --- B. NUMERICAL TAYLOR EXPANSION ---
fprintf('\n🔢 Calculating Numerical Taylor Expansion (around x_eq = %.4f)...\n', x_eq);

% 1. Substitute geometric parameters (L, R1, R2, etc.) into the original symbolic Force
%    leaving only 'z', 'I', and 'z0' (if any remains) as symbols.
Force_semi_num = subs(Force_sym, ...
    {L, R1, R2, mu0, N, m_mag}, ...
    {geom_L, geom_R1, geom_R2, mu0_val, geom_N, m_mag_val});

% 2. Compute the Taylor expansion numerically around the specific point x_eq
Taylor_F_numeric = taylor(Force_semi_num, z, 'ExpansionPoint', x_eq, 'Order', 3);

fprintf('\n📜 Taylor Expansion of Magnetic Force (Approx. for Control):\n');
% Use vpa (Variable Precision Arithmetic) to keep it readable (4 significant digits)
pretty(vpa(Taylor_F_numeric, 4));
%% 6. Verification Plot
figure;
loglog(z_vector, abs(F_values), 'b', 'LineWidth', 2); hold on;
% Plot of the ideal 1/z^n model for comparison
K_fit = abs(F_values(idx)) * (x_eq^n_identified);
F_ideal = K_fit ./ (z_vector.^n_identified);
loglog(z_vector, F_ideal, 'r--', 'LineWidth', 1.5);
xline(x_eq, 'k:', 'Equilibrium');
legend('Thick Model (Ground Truth)', sprintf('Simple Model (n=%.2f)', n_identified));
grid on;
title('Model Comparison and n Identification');
xlabel('Distance z [m]'); ylabel('Force [N]');