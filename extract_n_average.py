from datetime import datetime
import os

import pandas as pd


CD = os.getcwd()
FN = 'Output.txt'


def main():
    # Grab all csv filenames.
    for dp, dn, fn in os.walk(CD):
        csv_files = fn[0:-2]

    # Concatinate all csv files.
    print('Loading and combining data...')
    data_frame = pd.concat(
            (pd.read_csv(f) for f in csv_files),
            ignore_index=True
        )
    df_len = len(data_frame.index)

    # Segregate data according to day same date.
    segregated_data = {}
    for i in range(df_len):
        status = round((i+1)/df_len*100, 2)
        print(f'Segregating Data... {status}%', end='\r')
        # if status >= 1.33: break
        sd_keys = segregated_data.keys()
        df_v = data_frame.loc[i].values  # DataFrame Values
        date = df_v[0]

        # Check date if its in `segregate_data`.
        if date not in sd_keys:
            segregated_data[date] = [df_v]
        else:
            segregated_data[date].append(df_v)
    print('\nData Segregated!')

    # Calculate the average mean of temperature for that date.
    data_final = 'date\tave temperature\tave humidity\n'
    sd_keys = segregated_data.keys()
    for key in sd_keys:
        # Sort data chronologically.
        data_temp = {}
        for data in segregated_data[key]:
            date = data[0]; time = data[1]
            temperature = data[3]; humidity = data[4]
            # Calculate time in seconds since Unix epoch.
            dt = date + ' ' + time
            ts = datetime.strptime(dt, "%m/%d/%Y %H:%M:%S").timestamp()
            data_temp[ts] = (temperature, humidity)
        keys = list(data_temp.keys()); keys.sort()
        d = 0; t = 0
        for k in keys:
            d += data_temp[k][0]
            t += data_temp[k][1]
        data_final += \
            f'{key}\t{round(d/len(keys), 2)}\t{round(t/len(keys), 2)}\n'

    # Save final data to a text file.
    with open(FN, 'w') as f:
        f.writelines(data_final)
    print("Script Done!")


if __name__ == "__main__":
    main()
