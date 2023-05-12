# MySQL Replication Benchmark Scripts

This repository contains two files for benchmarking MySQL replication:

1.  `measure.py`: This is a Python script that measures how long it takes for a MySQL follower to 
catch up with its leader in a leader-follower replication setup. It continuously monitors two values: 
`Exec_Master_Log_Pos` and `Seconds_Behind_Master`. **It should run on the follower.**
    
2.  `test.cpp`: This is a C++ program that stress-tests a MySQL database by performing a large number 
of updates within a specified period of time, and measures the time taken to perform each update 
operation. It uses multiple-threading to perform these updates concurrently. **It should run on the leader.**
    

## Prerequisites

-   A MySQL server (tested with MySQL 5.6 and above)
-   Leader-follower replication setup
-   Python 3.6 or above
-   A C++ compiler that supports C++17
-   MySQL C++ connector installed (`sudo apt-get install libmysqlcppconn-dev`)

## Building

This repository includes a Makefile for building the C++ program:

`make` 

This will produce an executable named `testapp`.

To clean up the build artifacts:

`make clean` 

## Usage

### Python Script

To run the `measure.py` script:

`python3 measure.py` 

Please make sure to set your MySQL credentials in the script before running it.

### C++ Program

The `testapp` program supports two commands:

-   Prepare: This command prepares a specified number of rows in the MySQL table. To use it:

`./testapp prepare <num_rows>` 

-   Update: This command starts a specified number of threads, each of which performs updates on the 
MySQL table for a specified duration. To use it:

`./testapp update <end_id> <num_threads> <num_seconds>` 

