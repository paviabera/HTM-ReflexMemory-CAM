import openpyxl
from openpyxl import Workbook
import nbformat
from nbconvert import PythonExporter
import pathlib
import csv
from tqdm import tqdm
from IPython import get_ipython
from HTM import run_algorithm 




inputSources = [
    # yyyy-mm-dd-hh-mm-ss format
    "hourly_numentaTM_speed_7578.csv",
    "hourly_numentaTM_iio_us-east-1_i-a2eb1cd9_NetworkIn.csv",
    "hourly_numentaTM_exchange-3_cpc_results.csv",
    "hourly_numentaTM_exchange-3_cpm_results.csv",
    "hourly_numentaTM_exchange-2_cpc_results.csv",
    "hourly_numentaTM_exchange-2_cpm_results.csv",
    "hourly_numentaTM_exchange-4_cpc_results.csv",
    "hourly_numentaTM_exchange-4_cpm_results.csv",
    "hourly_numentaTM_rogue_agent_key_hold.csv",
    "hourly_numentaTM_TravelTime_451.csv",
    "hourly_numentaTM_occupancy_6005.csv",
    "hourly_numentaTM_speed_t4013.csv",
    "hourly_numentaTM_TravelTime_387.csv",
    "hourly_numentaTM_occupancy_t4013.csv",
    "hourly_numentaTM_speed_6005.csv",
    "hourly_numentaTM_art_daily_flatmiddle.csv",
    "hourly_numentaTM_art_daily_jumpsdown.csv",
    "hourly_numentaTM_art_daily_jumpsup.csv",
    "hourly_numentaTM_art_daily_no_noise.csv",
    "hourly_numentaTM_art_daily_nojump.csv",
    "hourly_numentaTM_art_daily_perfect_square_wave.csv",
    "hourly_numentaTM_art_daily_small_noise.csv",
    "hourly_numentaTM_art_flatline.csv",
    "hourly_numentaTM_art_increase_spike_density.csv",
    "hourly_numentaTM_art_load_balancer_spikes.csv",
    "hourly_numentaTM_art_noisy.csv",
    "hourly_numentaTM_ec2_cpu_utilization_24ae8d.csv",
    "hourly_numentaTM_ec2_cpu_utilization_53ea38.csv",
    "hourly_numentaTM_ec2_cpu_utilization_5f5533.csv",
    "hourly_numentaTM_ec2_cpu_utilization_77c1ca.csv",
    "hourly_numentaTM_ec2_cpu_utilization_825cc2.csv",
    "hourly_numentaTM_ec2_cpu_utilization_ac20cd.csv",
    "hourly_numentaTM_ec2_cpu_utilization_c6585a.csv",
    "hourly_numentaTM_ec2_cpu_utilization_fe7f93.csv",
    "hourly_numentaTM_ec2_disk_write_bytes_c0d644.csv",
    "hourly_numentaTM_ec2_network_in_257a54.csv",
    "hourly_numentaTM_ec2_request_latency_system_failure.csv",
    "hourly_numentaTM_elb_request_count_8c0756.csv",
    "hourly_numentaTM_rds_cpu_utilization_cc0c53.csv",
    "hourly_numentaTM_rds_cpu_utilization_e47b3b.csv",
    "hourly_numentaTM_grok_asg_anomaly.csv",
    "hourly_numentaTM_ec2_disk_write_bytes_1ef3de.csv",
    "hourly_numentaTM_ec2_network_in_5abac7.csv",
    "hourly_numentaTM_rogue_agent_key_updown.csv",
    "hourly_numentaTM_ambient_temperature_system_failure.csv",
    "hourly_numentaTM_nyc_taxi.csv",
    "hourly_numentaTM_Twitter_volume_AMZN.csv",
    "hourly_numentaTM_Twitter_volume_FB.csv",
    "hourly_numentaTM_Twitter_volume_GOOG.csv",
    "hourly_numentaTM_Twitter_volume_KO.csv",
    "hourly_numentaTM_Twitter_volume_CVS.csv",
    "hourly_numentaTM_Twitter_volume_PFE.csv",
    "hourly_numentaTM_Twitter_volume_UPS.csv",
    "hourly_numentaTM_Twitter_volume_IBM.csv",
    "hourly_numentaTM_Twitter_volume_AAPL.csv",
    "hourly_numentaTM_Twitter_volume_CRM.csv",
    "hourly_numentaTM_cpu_utilization_asg_misconfiguration.csv",
    "hourly_numentaTM_machine_temperature_system_failure.csv",

# yyyy-mm-dd format
    "monthly_sp500.csv",
   "weekly_dow_jones.csv",
   "weekly_nasdaq.csv",
   "weekly_sp500.csv",
   "monthly_vix_close.csv",
   "monthly_vix_high.csv",
   "monthly_vix_low.csv",
   "monthly_vix_open.csv",
   "daily_natural_gas.csv",
   "daily_oil_prices.csv",


# yyyy-mm format   
   "monthly_gold_prices.csv",
# no date format  
#    "value1_vix_close.csv",
#    "value1_vix_high.csv",
#    "value1_vix_low.csv",
#    "value1_vix_open.csv"
#    "value1_pseudo_periodic_synthetic_1.csv",
#    "value1_pseudo_periodic_synthetic_2.csv",
#    "value1_pseudo_periodic_synthetic_3.csv",
#    "value1_pseudo_periodic_synthetic_4.csv",
#    "value1_pseudo_periodic_synthetic_5.csv",
#    "value1_pseudo_periodic_synthetic_6.csv",
#    "value1_pseudo_periodic_synthetic_7.csv",
#    "value1_pseudo_periodic_synthetic_8.csv",
#    "value1_pseudo_periodic_synthetic_9.csv",
#    "value1_pseudo_periodic_synthetic_10.csv",
]


# Function to write data to an Excel file
def write_to_excel(filename, dataset_name, anomaly_score):
    try:
        # Try to load the workbook if it exists
        workbook = openpyxl.load_workbook(filename)
        sheet = workbook.active
    except FileNotFoundError:
        # Create a new workbook if the file doesn't exist
        workbook = Workbook()
        sheet = workbook.active
        # Write header row
        sheet.append(["Dataset", "Anomaly Score"])

    # Append the data
    sheet.append([dataset_name, anomaly_score])

    # Save the workbook
    workbook.save(filename)




# Excel file name
excel_file = "datasets/anomaly_scores_1.xlsx"

pbar = tqdm(total=len(inputSources))
# Iterate over datasets, run the algorithm, and save results
input_path = pathlib.Path('/workspaces/HTM/datasets/numenta')
for dataset in inputSources:
    print("dataset = ", dataset )
    AnomalyMean, AnomalyStd = run_algorithm(dataset)
    write_to_excel(excel_file, dataset, AnomalyMean)
    print(f"Processed {dataset} with anomaly score {AnomalyMean}. Data saved to {excel_file}")
    # pbar.update(1)
    # break

pbar.close()