#' @title Aether-Lock: Coil-Sensor Coupling Analysis
#' @author Michele Bisignano
#' @description Analyzes the linear relationship between Coil PWM and Hall Sensor ADC readings.
#'              Calculates the compensation factor for the firmware to cancel out EMI.

# --- 1. LIBRARIES & CONFIGURATION ---
library(tidyverse)

# Define paths
INPUT_FILE  <- "../Data/Raw/coil_linearity_test.csv"
OUTPUT_PLOT <- "../Output/coil_linearity_clean.png"

# Analysis Parameters
RESIDUAL_THRESHOLD_SD <- 3  # Standard Deviations to define an outlier

# --- 2. DATA ACQUISITION ---
if (!file.exists(INPUT_FILE)) {
  stop(paste("CRITICAL ERROR: File not found at", INPUT_FILE))
}

# Robust loading: attempts comma first, then semicolon
df_raw <- read_csv(INPUT_FILE, show_col_types = FALSE)

if (ncol(df_raw) < 3) {
  message("Found unconventional delimiter. Retrying with semicolon...")
  df_raw <- read_delim(INPUT_FILE, delim = ";", show_col_types = FALSE)
}

# Standardize Column Names
# Expected format: [Time_ms, PWM_Percent, Raw_ADC]
colnames(df_raw)[1:3] <- c("time_ms", "pwm_percent", "raw_adc")

# Data Type Enforcement & Cleaning
df_processed <- df_raw %>%
  mutate(across(c(pwm_percent, raw_adc), ~ as.numeric(as.character(.x)))) %>%
  filter(!is.na(pwm_percent), !is.na(raw_adc))

message("✅ Data successfully loaded. Observation count: ", nrow(df_processed))

# --- 3. STATISTICAL OUTLIER REMOVAL ---
# We use a two-pass linear regression to identify and remove EMI spikes/voltage drops
prelim_model <- lm(raw_adc ~ pwm_percent, data = df_processed)

df_analysis <- df_processed %>%
  mutate(
    predicted = predict(prelim_model),
    residual  = raw_adc - predicted,
    # Flag outliers based on Standard Deviation of residuals
    is_outlier = abs(residual) > (sd(residual) * RESIDUAL_THRESHOLD_SD)
  )

df_clean <- df_analysis %>% filter(!is_outlier)

message("📊 Outlier removal complete. Points removed: ", sum(df_analysis$is_outlier))

# --- 4. COUPLING FACTOR CALCULATION ---
# Final model based on clean data
final_model <- lm(raw_adc ~ pwm_percent, data = df_clean)
coeffs      <- coef(final_model)

slope     <- coeffs["pwm_percent"]      # ADC change per 1% PWM
intercept <- coeffs["(Intercept)"]      # Sensor baseline (Coil OFF)

# The firmware requires the total delta from 0% to 100% PWM
# Formula: Slope * 100
total_coupling_factor <- round(abs(slope * 100), 0)

# --- 5. RESULTS REPORTING ---
cat("\n==============================================\n")
cat("       ANALYSIS RESULTS (Aether-Lock)         \n")
cat("==============================================\n")
cat(sprintf("Linear Slope:      %.4f ADC/%%\n", slope))
cat(sprintf("Intercept (Bias):  %.2f ADC\n", intercept))
cat(sprintf("R-Squared:         %.4f\n", summary(final_model)$r.squared))
cat("----------------------------------------------\n")
cat(sprintf(">>> COIL_SENSOR_COUPLING_ADC: %d\n", total_coupling_factor))
cat("----------------------------------------------\n")
cat("Copy the value above into your hardware JSON config.\n\n")

# --- 6. VISUALIZATION ---
plot_theme <- theme_minimal(base_size = 12) +
  theme(
    plot.title = element_text(face = "bold", size = 14),
    panel.grid.minor = element_blank()
  )

p <- ggplot(df_analysis, aes(x = pwm_percent, y = raw_adc)) +
  # Plot outliers in light red
  geom_point(data = filter(df_analysis, is_outlier), 
             color = "#e41a1c", alpha = 0.3, size = 1) +
  # Plot valid data in dark blue
  geom_point(data = filter(df_analysis, !is_outlier), 
             color = "#377eb8", alpha = 0.6, size = 1.5) +
  # Regression line
  geom_smooth(data = df_clean, method = "lm", 
              color = "#4daf4a", size = 1.2, se = FALSE) +
  labs(
    title = "Coil-Sensor Coupling Linearity Analysis",
    subtitle = paste("Estimated Coupling:", total_coupling_factor, "ADC points @ 100% Duty Cycle"),
    x = "PWM Duty Cycle [%]",
    y = "Raw Sensor Reading [ADC]",
    caption = paste("Outliers removed using", RESIDUAL_THRESHOLD_SD, "SD Residual Threshold")
  ) +
  plot_theme

# Save output
ggsave(OUTPUT_PLOT, plot = p, width = 9, height = 6, dpi = 300)
print(p)