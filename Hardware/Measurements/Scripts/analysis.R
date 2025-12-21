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

# --- 3. DATA CLEANING (Outlier Removal) ---
full_dataset_clean <- full_dataset %>%
  group_by(Label) %>%
  mutate(
    Q1 = quantile(Sensor_Raw, 0.25),
    Q3 = quantile(Sensor_Raw, 0.75),
    IQR = Q3 - Q1,
    Lower_Bound = Q1 - 1.5 * IQR,
    Upper_Bound = Q3 + 1.5 * IQR
  ) %>%
  filter(Sensor_Raw >= Lower_Bound & Sensor_Raw <= Upper_Bound) %>%
  ungroup()

# --- 4. RECALCULATE STATS (After Cleaning) ---
sensor_stats <- full_dataset_clean %>%
  group_by(Label, Distance_mm) %>%
  summarise(
    Mean_Raw = mean(Sensor_Raw, na.rm = TRUE),
    SD_Raw   = sd(Sensor_Raw, na.rm = TRUE),    # Precisione del sensore (Rumore)
    Min_Raw  = min(Sensor_Raw, na.rm = TRUE),
    Max_Raw  = max(Sensor_Raw, na.rm = TRUE),
    Range    = Max_Raw - Min_Raw,              # Escursione del segnale
    Sample_Count = n(),
    .groups = 'drop'
  ) %>%
  arrange(Distance_mm)

# Bias
resting_bias <- sensor_stats %>% 
  filter(Label == "inf") %>% 
  pull(Mean_Raw)

print("=== CLEANED STATISTICAL SUMMARY ===")
print(sensor_stats)

print(paste("Computed Resting Bias (Zero Field):", round(resting_bias, 2)))

# --- 5. VISUALIZATION ---

# A. Calibration Curve (Distance vs. Mean ADC)
# Exclude 'inf' for the linear range plot
calibration_plot <- sensor_stats %>% 
  filter(Distance_mm < 100) %>%
  ggplot(aes(x = Distance_mm, y = Mean_Raw)) +
  geom_point(size = 3, color = "#2c3e50") +
  geom_line(color = "#3498db", linewidth =  1) +
  geom_hline(yintercept = resting_bias, linetype = "dashed", color = "#e74c3c") +
  annotate("text", 
           x = 0,
           y = resting_bias, 
           label = "Resting Bias", 
           color = "#e74c3c",
           hjust = -0.1,                
           vjust = -1, 
           fontface = "bold")                    +
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

# C. Stability and Drift Analysis 
drift_plot <- full_dataset %>% 
  # 1. Filtriamo per la distanza desiderata
  filter(str_detect(Label, "3")) %>%
  # 2. Creiamo una colonna "Sample_Index" che va da 1 a N
  # Questo evita l'errore del formato Time
  mutate(Sample_Index = row_number()) %>% 
  
  ggplot(aes(x = Sample_Index, y = Sensor_Raw)) +
  geom_line(alpha = 0.3, color = "black") +
  # La linea di tendenza ora funzionerà perfettamente
  geom_smooth(method = "lm", color = "#e67e22", se = TRUE) + 
  labs(title = "Sensor Stability Over Samples (Target: 3.0cm)",
       subtitle = "Linear regression (orange) indicates potential signal drift",
       x = "Sample Number (Sequence)",
       y = "Raw ADC Value") +
  theme_minimal()

print(drift_plot)

# --- 6. EXPORTING RESULTS ---

# Define the output directory (relative to the 'Script' folder)
processed_dir <- "../Data/Processed"

# Create the folder if it doesn't exist to avoid errors
if (!dir.exists(processed_dir)) {
  dir.create(processed_dir, recursive = TRUE)
  print(paste("Created directory:", processed_dir))
}

# --- 6. EXPORTING RESULTS ---

# 1. Define folder paths (relative to the 'Script' folder)
output_dir <- "../Output"
processed_dir <- "../Data/Processed"

# 2. Create directories if they don't exist
if (!dir.exists(output_dir)) dir.create(output_dir, recursive = TRUE)
if (!dir.exists(processed_dir)) dir.create(processed_dir, recursive = TRUE)

# 3. SAVE TABLES (Formatted for European Excel - Semicolon separator)
# Save the master cleaned dataset
write_excel_csv2(full_dataset_clean, file.path(processed_dir, "full_cleaned_dataset.csv"))

# Save the statistical summary in BOTH folders as requested
write_excel_csv2(sensor_stats, file.path(processed_dir, "sensor_statistical_summary.csv"))
write_excel_csv2(sensor_stats, file.path(output_dir, "sensor_summary_report.csv"))

# 4. SAVE PLOTS AS PNG
# Save Calibration Curve
ggsave(filename = file.path(output_dir, "calibration_curve.png"), 
       plot = calibration_plot, width = 8, height = 6, dpi = 300)

# Save Noise Boxplot
ggsave(filename = file.path(output_dir, "noise_analysis.png"), 
       plot = noise_plot, width = 8, height = 6, dpi = 300)

# Save Drift Analysis (Stability)
ggsave(filename = file.path(output_dir, "stability_drift_analysis.png"), 
       plot = drift_plot, width = 8, height = 6, dpi = 300)

print("=== EXPORT COMPLETE ===")
print(paste("Tables saved in:", processed_dir, "and", output_dir))
print(paste("Plots saved as PNG in:", output_dir))