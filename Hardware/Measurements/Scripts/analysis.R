# Aether-Lock: Sensor Characterization Analysis
# Author: Michele Bisignano

#install.packages(c("tidyverse", "readxl"))

library(tidyverse)
library(readxl)

# --- 1. CONFIGURATION ---
file_path <- "../Data/Raw/sensor_data.xlsx"

# Sheet List (Based on your naming convention)
# Mapping sheet names to real-world distances in millimeters
# "inf" is treated as 999mm (a high dummy value) for baseline/zero calculation
sheets_mapping <- list(
  "0cm"   = 0,
  "1cm" = 10,
  "1,5cm" = 15,
  "2cm"   = 20,
  "2,5cm" = 25,
  "3cm"   = 30,
  "3,5cm" = 35,
  "4cm"   = 40,
  "inf" = 999 # Special code for "Infinite / No Magnet"
)

# --- 2. DATA IMPORT AND MERGE ---
df_list <- list()

for (sheet_name in names(sheets_mapping)) {
  # Read the specific sheet
  temp_df <- read_excel(file_path, sheet = sheet_name)
  
  # Get the mapped distance
  dist_val <- sheets_mapping[[sheet_name]]
  
  # Clean and process data
  # Uses 'contains' for robustness against spaces or special characters
  temp_df <- temp_df %>%
    mutate(Distance_mm = dist_val,
           Label = sheet_name) %>%
    select(
      Time       = contains("Time"),     # Matches "Timestamp", "Time", etc.
      Sensor_Raw = contains("Sensor"),   # Matches "Sensor Raw", "sensor raw"
      Voltage    = contains("Volt"),     # Matches "Voltage (V)", "voltage"
      Distance_mm, 
      Label
    )
  
  df_list[[sheet_name]] <- temp_df
}
# Merge all sheets into one single master dataset
full_dataset <- bind_rows(df_list)

# --- 3. STATISTICAL SUMMARY ---
# Calculate metrics for each distance (Mean, Noise/SD, Ranges)
sensor_stats <- full_dataset %>%
  group_by(Label, Distance_mm) %>%
  summarise(
    Mean_Raw = mean(Sensor_Raw, na.rm = TRUE),
    SD_Raw   = sd(Sensor_Raw, na.rm = TRUE),    # Standard Deviation = Noise Floor
    Min_Raw  = min(Sensor_Raw, na.rm = TRUE),
    Max_Raw  = max(Sensor_Raw, na.rm = TRUE),
    Sample_Count = n(),
    .groups = 'drop'
  ) %>%
  arrange(Distance_mm)

print("=== SENSOR STATISTICAL SUMMARY TABLE ===")
print(sensor_stats)

# --- 4. ZERO BIAS (RESTING STATE) CALCULATION ---
# The mean value at "infinite" distance is our magnetic bias (Zero)
resting_bias <- sensor_stats %>% 
  filter(Label == "inf") %>% 
  pull(Mean_Raw)

print(paste("Computed Resting Bias (Zero Field):", round(resting_bias, 2)))

# --- 5. VISUALIZATION ---

# A. Calibration Curve (Distance vs. Mean ADC)
# Exclude 'inf' for the linear range plot
calibration_plot <- sensor_stats %>% 
  filter(Distance_mm < 100) %>%
  ggplot(aes(x = Distance_mm, y = Mean_Raw)) +
  geom_point(size = 3, color = "#2c3e50") +
  geom_line(color = "#3498db", size = 1) +
  geom_hline(yintercept = resting_bias, linetype = "dashed", color = "#e74c3c") +
  annotate("text", x = 30, y = resting_bias + 50, label = "Resting Bias (Zero)", color = "#e74c3c") +
  labs(title = "SS49E Hall Sensor Calibration Curve",
       subtitle = "Distance vs. Mean ADC Value (0-4095 range)",
       x = "Magnet Distance [mm]",
       y = "Mean Raw ADC Reading") +
  theme_minimal()

print(calibration_plot)

# B. Noise Analysis (Boxplot)
# Higher boxes indicate higher signal jitter/interference
noise_plot <- ggplot(full_dataset, aes(x = reorder(Label, Distance_mm), y = Sensor_Raw, fill = Label)) +
  geom_boxplot(alpha = 0.7) +
  labs(title = "Signal Noise Distribution by Distance",
       subtitle = "Taller boxes indicate increased sensor jitter",
       x = "Distance Category",
       y = "Raw ADC Readings") +
  theme_minimal() +
  theme(legend.position = "none")

print(noise_plot)

# C. Stability and Drift Analysis (Example: 1.5 cm)
# Checks if the sensor value shifts over time (e.g., due to heat)
drift_plot <- full_dataset %>% 
  filter(Label == "1.5") %>%
  ggplot(aes(x = Time, y = Sensor_Raw)) +
  geom_line(alpha = 0.4, color = "black") +
  geom_smooth(method = "lm", color = "#e67e22") + # Linear regression trend
  labs(title = "Sensor Stability over Time (Target: 1.5cm)",
       subtitle = "The orange line indicates potential signal drift",
       x = "Relative Time [s]",
       y = "Raw ADC Value") +
  theme_minimal()

print(drift_plot)

# --- 6. EXPORTING RESULTS ---
# ggsave("../Output/calibration_curve.png", calibration_plot, width = 8, height = 6)
# write_csv(full_dataset, "../Data/Processed/processed_sensor_data.csv")

