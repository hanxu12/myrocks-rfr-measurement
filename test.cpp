#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <mysql_connection.h>
#include <driver.h>
#include <exception.h>
#include <resultset.h>
#include <statement.h>
#include <prepared_statement.h>
#include <mutex>
#include <fstream>

using namespace sql;
std::mutex print_mtx;

std::string random_string(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

void create_table(const std::string &table_name, Connection *conn)
{
    try
    {
        std::cout << "Creating table " << table_name << "..." << std::endl;
        std::unique_ptr<Statement> stmt(conn->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS " + table_name + " (id INT AUTO_INCREMENT PRIMARY KEY, data VARCHAR(255) NOT NULL)");
        std::cout << "Table " << table_name << " created successfully." << std::endl;
    }
    catch (const SQLException &e)
    {
        std::cerr << "Error creating table " << std::endl;
    }
}

void prepare_data(const std::string &table_name, int num_rows, Connection *conn)
{
    try
    {
        std::cout << "Preparing " << num_rows << " rows in " << table_name << "..." << std::endl;
        std::unique_ptr<PreparedStatement> pstmt(conn->prepareStatement("INSERT INTO " + table_name + " (data) VALUES (?)"));
        for (int i = 0; i < num_rows; ++i)
        {
            std::cout << "idx " << i << std::endl;
            pstmt->setString(1, "abc");
            pstmt->execute();
        }
        std::cout << num_rows << " rows prepared in " << table_name << "." << std::endl;
    }
    catch (const SQLException &e)
    {
        std::cerr << "Error preparing data in " << std::endl;
    }
}

void update_data(int thread_id, int start_id, int num_rows, int num_seconds, Connection *conn)
{
    try
    {
        // std::cout << "Thread " << thread_id << " is updating " << num_rows << " rows..." << std::endl;
        std::unique_ptr<PreparedStatement> pstmt(conn->prepareStatement("UPDATE test_table SET data = ? WHERE id = ?"));
        auto start_time = std::chrono::steady_clock::now();
        std::ofstream elapsed_time_file;
        if (thread_id == 1)
            elapsed_time_file.open("elapsed_time.txt", std::ios::app);
        while (true)
        {
            int cur_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count();
            if (cur_sec > num_seconds)
            {
                break;
            }
            // std::cout << "Thread " << thread_id << " is updating " << num_rows << " rows at " << cur_sec << " sec" << std::endl;
            for (int i = 0; i < num_rows; ++i)
            {
                // std::cout << "idx " << i << std::endl;
                auto start = std::chrono::steady_clock::now();
                int id = start_id + i;
                std::string rand_str = random_string(255);
                pstmt->setString(1, rand_str);
                pstmt->setInt(2, id);
                pstmt->executeUpdate();
                auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                if (thread_id == 1)
                {
                    const std::scoped_lock<std::mutex> lock(print_mtx); // scoped lock: https://stackoverflow.com/a/14276591
                    std::cout << "Thread " << thread_id << " updated row id " << id << " for " << elapsed_time << " ms" << std::endl;
                    elapsed_time_file << elapsed_time << std::endl;
                }
                // printf("Thread %d updated row id %d for %ld ms", thread_id, id, elapsed_time);
            }
            // std::this_thread::sleep_for(std::chrono::milliseconds(1000 - elapsed_time)); // compensate for the time spent on updating
        }
        if (thread_id == 1)
            elapsed_time_file.close();
    }
    catch (const SQLException &e)
    {
        std::cerr << "Error in thread " << thread_id << ": " << e.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    // random seed
    std::srand(std::time(0));
    // mysql connection
    std::string host = "tcp://localhost:3306";
    std::string user = "root";
    std::string password = "123456";
    std::string database = "test10";
    std::string table_name = "test_table";
    std::vector<std::thread> threads;                     // thread pool
    std::vector<std::unique_ptr<Connection>> connections; // connection pool
    std::string command = argv[1];
    Driver *driver = get_driver_instance();
    if (command == "prepare")
    {
        if (argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " prepare <num_rows>" << std::endl;
            return 1;
        }
        int num_rows = std::stoi(argv[2]);
        std::unique_ptr<Connection> conn(driver->connect(host, user, password));
        conn->setSchema(database);
        // create_table(table_name, conn.get());
        prepare_data(table_name, num_rows, conn.get());
    }
    else if (command == "update")
    {
        if (argc != 5)
        {
            std::cerr << "Usage: " << argv[0] << " update <num_rows_per_thread> <num_threads> <num_seconds>" << std::endl;
            return 1;
        }
        int num_rows_per_thread = std::stoi(argv[2]);
        int num_threads = std::stoi(argv[3]);
        int num_seconds = std::stoi(argv[4]);
        for (int i = 0; i < num_threads; ++i)
        {
            std::unique_ptr<Connection> conn(driver->connect(host, user, password));
            conn->setSchema(database);
            connections.push_back(std::move(conn));
            int start_id = i * num_rows_per_thread + 1;
            threads.push_back(std::thread(update_data, i, start_id, num_rows_per_thread, num_seconds, connections[i].get()));
        }

        for (auto &t : threads)
        {
            t.join();
        }
    }

    return 0;
}
