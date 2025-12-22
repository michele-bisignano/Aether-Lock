# Aether-Lock: Coil-Sensor Coupling Analysis
# Author: Michele Bisignano
# Purpose: Calculate the Feedforward term to cancel coil noise from sensor readings.

library(tidyverse)

# --- 1. CONFIGURATION ---
# Ensure the file name matches what you saved
input_file <- "../Data/Raw/coil_linearity_test.csv"
output_plot <- "../Output/coil_linearity_clean.png"

# --- 2. DATA LOADING (ROBUST) ---
library(readr) # Ensure readr is loaded

# Attempt 1: Read with comma (US/Arduino Standard)
data_raw <- read_delim(input_file, delim = ",", col_names = TRUE, show_col_types = FALSE)

# Check: If only 1 column was found, the delimiter was probably wrong (;)
if (ncol(data_raw) < 2) {
  print("⚠️ Comma delimiter failed. Trying semicolon...")
  data_raw <- read_delim(input_file, delim = ";", col_names = TRUE, show_col_types = FALSE)
}

# Final Check: Do we have at least 3 columns?
if (ncol(data_raw) < 3) {
  stop("CRITICAL ERROR: The CSV file does not have 3 columns. Check 'coil_linearity_test.csv' with Notepad.")
}

# --- FORCE COLUMN NAMES ---
# We don't trust the file header. We strictly rename the first 3 columns.
# The order MUST be: Time, PWM, Sensor
colnames(data_raw)[1] <- "Time_ms"
colnames(data_raw)[2] <- "PWM_Percent"
colnames(data_raw)[3] <- "Raw_ADC"

# Cleaning: Ensure they are numeric (in case of repeated headers or errors)
data_raw <- data_raw %>%
  mutate(
    PWM_Percent = as.numeric(PWM_Percent),
    Raw_ADC = as.numeric(Raw_ADC)
  ) %>%
  filter(!is.na(PWM_Percent) & !is.na(Raw_ADC)) # Removes empty rows or errors

print("✅ Data loaded successfully with columns:")
print(colnames(data_raw))
print(head(data_raw)) # Print first rows for verification

# --- 3. OUTLIER CLEANING (Residual Method) ---
# Perform a preliminary linear fit
model_prelim <- lm(Raw_ADC ~ PWM_Percent, data = data_raw)

# Calculate the error (residual) of each point relative to the line
data_aug <- data_raw %>%
  mutate(
    Predicted = predict(model_prelim),
    Residual = Raw_ADC - Predicted,
    # Define outliers as points deviating more than 2 Standard Deviations from the mean error
    Is_Outlier = abs(Residual) > 2 * sd(Residual)
  )

# Filter valid data
data_clean <- data_aug %>% filter(Is_Outlier == FALSE)

print(paste("Points removed (Noise):", nrow(data_raw) - nrow(data_clean)))

# --- 4. FINAL COEFFICIENT CALCULATION ---
# Re-run the model only on valid data
model_final <- lm(Raw_ADC ~ PWM_Percent, data = data_clean)
coeffs <- coef(model_final)

slope <- coeffs["PWM_Percent"]      # ADC points per 1% PWM
intercept <- coeffs["(Intercept)"]  # Value at 0 PWM (Coil OFF Bias)

# --- CALCULATE PARAMETER FOR FIRMWARE ---
# The firmware uses PWM from 0.0 to 1.0 (not %).
# Therefore, Total Coupling is: Slope * 100
# Example: If it drops 1.2 points per 1%, it drops 120 points for 100%
coupling_factor_total = slope * 100

print("=== ANALYSIS RESULTS ===")
print(paste("Slope (points per 1% PWM):", round(slope, 4)))
print(paste("Intercept (Bias):", round(intercept, 2)))
print("------------------------------------------------")
print(paste(">>> COIL_SENSOR_COUPLING_ADC (to put in JSON):", round(abs(coupling_factor_total), 0)))
print("------------------------------------------------")

# --- 5. PLOTTING ---
# Comparative Plot: Red = Discarded, Blue = Valid, Line = Model
p <- ggplot(data_aug, aes(x = PWM_Percent, y = Raw_ADC)) +
  # Discarded points in light red
  geom_point(data = subset(data_aug, Is_Outlier), color = "red", alpha = 0.3, size = 1) +
  # Valid points in dark blue
  geom_point(data = subset(data_aug, !Is_Outlier), color = "darkblue", alpha = 0.6, size = 1.5) +
  # Final trend line
  geom_smooth(data = data_clean, method = "lm", color = "green", size = 1.5, se = FALSE) +
  
  labs(title = "Coil Linearity Analysis (Outlier Removal)",
       subtitle = paste("Calculated Coupling Factor:", round(abs(coupling_factor_total), 0), "ADC points @ 100% PWM"),
       x = "PWM Duty Cycle [%]",
       y = "Sensor Reading [Raw ADC]") +
  theme_minimal()

# Save and show
ggsave(output_plot, plot = p, width = 8, height = 6)
print(p)