import subprocess
import time

# Set your MySQL credentials
user = "root"
password = "123456"

# Set the output file name
f1 = open("exec_master_log_pos.txt", "w")
f2 = open("sec_behind_master.txt", "w")

start_time, end_time = None, None
cnt = 0
start_counting = False

while 1:
    # Execute the command
    cmd1 = f'mysql -u{user} -p{password} -e "SHOW SLAVE STATUS\\G" | awk -F\': \' 
\'/Exec_Master_Log_Pos/ {{print $2}}\''
    cmd2 = f'mysql -u{user} -p{password} -e "SHOW SLAVE STATUS\\G" | awk -F\': \' 
\'/Seconds_Behind_Master/ {{print $2}}\''
    output1 = subprocess.check_output(cmd1, shell=True, text=True)
    output2 = subprocess.check_output(cmd2, shell=True, text=True)

    # Get the Exec_Master_Log_Pos value
    exec_master_log_pos = output1.strip()
    sec_behind_master = output2.strip()

    # Write the value to the output file
    f1.write(f"{exec_master_log_pos}\n")
    f2.write(f"{sec_behind_master}\n")
    if sec_behind_master == "0":
        if start_counting:
            cnt += 1
    else:
        if not start_counting:
            start_time = time.time()
            start_counting = True
    if cnt == 5:
        end_time = time.time()
        break
    # Wait for 0.001 second
    time.sleep(0.001)
f1.close()
f2.close()
print(f"Time elapsed: {end_time - start_time} s")
