# Aether-Lock: Coil-Sensor Coupling Analysis (Super Robust)
# Author: Michele Bisignano

# --- 0. ENVIRONMENT SETUP ---
# In RStudio: Menu "Session" -> "Set Working Directory" -> "To Source File Location"
library(tidyverse)
library(readr)

# --- 1. PATH CONFIGURATION ---
# Relative path from Scripts folder to Raw folder
input_file  <- "../Data/Raw/coil_linearity_test.csv"
output_plot <- "../Output/coil_linearity_clean.png"

# Verify file existence
if (!file.exists(input_file)) {
  stop(paste("❌ ERROR: File not found!", normalizePath(input_file, mustWork = FALSE)))
}

print(paste("📂 Reading file:", input_file))

# --- 2. LOADING DATA AS TEXT ---
# Read EVERYTHING as characters (col_types = "c") to prevent R from guessing wrong types
# Try comma first; if it fails, try semicolon
data_raw <- read_delim(input_file, delim = ",", col_names = TRUE, col_types = cols(.default = "c"))

if (ncol(data_raw) < 2) {
  print("⚠️ Comma separator failed. Retrying with semicolon...")
  data_raw <- read_delim(input_file, delim = ";", col_names = TRUE, col_types = cols(.default = "c"))
}

print("--- Columns found: ---")
print(colnames(data_raw))

# --- 3. COLUMN SELECTION AND CLEANING ---
# Search for the correct columns intelligently
col_names <- colnames(data_raw)
idx_pwm <- grep("PWM", col_names, ignore.case = TRUE)
idx_adc <- grep("Raw", col_names, ignore.case = TRUE)

if (length(idx_pwm) == 0 || length(idx_adc) == 0) {
  # Fallback: If names are not found, use columns 2 and 3 by default
  print("⚠️ Column names not found. Using columns 2 and 3 by default.")
  idx_pwm <- 2
  idx_adc <- 3
} else {
  idx_pwm <- idx_pwm[1] # Take the first match
  idx_adc <- idx_adc[1]
}

# Create a clean dataframe by renaming and converting types
data_clean <- data_raw %>%
  select(
    PWM_Str = all_of(idx_pwm), 
    ADC_Str = all_of(idx_adc)
  ) %>%
  mutate(
    # Replace comma with period AND remove any potential spaces
    PWM_Percent = as.numeric(gsub(",", ".", PWM_Str)),
    Raw_ADC     = as.numeric(gsub(",", ".", ADC_Str))
  ) %>%
  # Remove rows that became NA (e.g., repeated headers or errors)
  filter(!is.na(PWM_Percent) & !is.na(Raw_ADC))

print(paste("✅ Valid rows found:", nrow(data_clean)))

if (nrow(data_clean) < 10) {
  stop("❌ ERROR: Too few valid data points after conversion. Check the CSV!")
}

# --- 4. OUTLIER ANALYSIS ---
model_prelim <- lm(Raw_ADC ~ PWM_Percent, data = data_clean)

data_aug <- data_clean %>%
  mutate(
    Predicted = predict(model_prelim),
    Residual = Raw_ADC - Predicted,
    # Remove points too far from the line (Voltage Sag or Errors)
    Is_Outlier = abs(Residual) > 2 * sd(Residual)
  )

data_final <- data_aug %>% filter(Is_Outlier == FALSE)

# --- 5. FINAL CALCULATION ---
model_final <- lm(Raw_ADC ~ PWM_Percent, data = data_final)
coeffs <- coef(model_final)

slope <- coeffs["PWM_Percent"]
coupling_factor <- slope * 100 # For 100% PWM

print("========================================")
print(paste(">>> COIL_SENSOR_COUPLING_ADC:", round(abs(coupling_factor), 0)))
print("========================================")

# --- 6. PLOT ---
p <- ggplot(data_aug, aes(x = PWM_Percent, y = Raw_ADC)) +
  geom_point(aes(color = Is_Outlier), alpha = 0.5) +
  geom_smooth(data = data_final, method = "lm", color = "green", se = FALSE) +
  scale_color_manual(values = c("FALSE"="blue", "TRUE"="red")) +
  labs(title = "Coil-Sensor Coupling Analysis",
       subtitle = paste("Calculated Factor:", round(abs(coupling_factor), 0)),
       x = "PWM %", y = "ADC Raw") +
  theme_minimal()

ggsave(output_plot, plot = p, width = 8, height = 6)
print(p)
