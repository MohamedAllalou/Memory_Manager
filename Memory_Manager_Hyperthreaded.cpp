// COEN 346
// Dec 03 2022
// C++ program to implement a memory manager

// Mohamed Allalou 40010809
// Ayman Chaoui 40021914

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <unordered_map>
#include <semaphore.h>

using namespace std;

// Structure to store the page information
struct Page {
    string variableId;
    unsigned int value;
    int lastAccessTime;
};

// Global Variables
mutex m;                                 // Mutual Exclusion
sem_t sem;                               // Semaphore
int clock = 0;                           // Clock
vector<Page> mainMemory;                 // Main Memory
unordered_map<string, Page> diskMemory;  // Disk Memory
string command;                          // Command

// Function to store the page
void Store(string variableId, unsigned int value) {
    m.lock();
    // Check if page is in the main memory
    bool isPageInMainMemory = false;
    for (int i = 0; i < mainMemory.size(); i++) {
        // If page is found then update the value
        if (mainMemory[i].variableId == variableId) {
            mainMemory[i].value = value;
            isPageInMainMemory = true;
            break;
        }
    }
    // If page is not in the main memory
    if (!isPageInMainMemory) {
        // Check if main memory is full
        if (mainMemory.size() < mainMemory.capacity()) {
            // Create the page and store
            Page page;
            page.variableId = variableId;
            page.value = value;
            page.lastAccessTime = clock;
            mainMemory.push_back(page);
        } else {
            // Find the page with least recently accessed time
            int leastRecentlyAccessedTime = INT_MAX;
            int index = -1;
            for (int i = 0; i < mainMemory.size(); i++) {
                if (mainMemory[i].lastAccessTime < leastRecentlyAccessedTime) {
                    leastRecentlyAccessedTime = mainMemory[i].lastAccessTime;
                    index = i;
                }
            }
            // Swap the page in the disk memory with the least recently accessed page in main memory
            Page page = mainMemory[index];
            diskMemory[page.variableId] = page;
            mainMemory.erase(mainMemory.begin() + index);
            // Store the new page
            page.variableId = variableId;
            page.value = value;
            page.lastAccessTime = clock;
            mainMemory.push_back(page);
            // Print the SWAP event
            cout << "Clock: " << clock << ", Memory Manager, SWAP: Variable " << variableId << " with Variable " << page.variableId << endl;
        }
    }
    m.unlock();
}

// Function to release the page
void Release(string variableId) {
    m.lock();
    // Search the page in the main memory
    for (int i = 0; i < mainMemory.size(); i++) {
        // If page is found then remove the page
        if (mainMemory[i].variableId == variableId) {
            mainMemory.erase(mainMemory.begin() + i);
            break;
        }
    }
    m.unlock();
}

// Function to lookup the page
int Lookup(string variableId) {
    m.lock();
    // Search the page in the main memory
    bool isPageInMainMemory = false;
    int value = -1;
    for (int i = 0; i < mainMemory.size(); i++) {
        // If page is found then return the value
        if (mainMemory[i].variableId == variableId) {
            value = mainMemory[i].value;
            isPageInMainMemory = true;
            break;
        }
    }
    // If page is not in the main memory
    if (!isPageInMainMemory) {
        // Search the page in the disk memory
        unordered_map<string, Page>::iterator it = diskMemory.find(variableId);
        // If page is found then
        if (it != diskMemory.end()) {
            // Move the page to the main memory
            Page page = it->second;
            mainMemory.push_back(page);
            diskMemory.erase(variableId);
            // If main memory is full then swap the page
            if (mainMemory.size() > mainMemory.capacity()) {
                // Find the page with least recently accessed time
                int leastRecentlyAccessedTime = INT_MAX;
                int index = -1;
                for (int i = 0; i < mainMemory.size(); i++) {
                    if (mainMemory[i].lastAccessTime < leastRecentlyAccessedTime) {
                        leastRecentlyAccessedTime = mainMemory[i].lastAccessTime;
                        index = i;
                    }
                }
                // Swap the page in the disk memory with the least recently accessed page in main memory
                Page page = mainMemory[index];
                diskMemory[page.variableId] = page;
                mainMemory.erase(mainMemory.begin() + index);
                // Print the SWAP event
                cout << "Clock: " << clock << ", Memory Manager, SWAP: Variable " << variableId << " with Variable " << page.variableId << endl;
            }
            // Return the value
            value = page.value;
        }
    }
    m.unlock();
    return value;
}

// Function to update the clock
void UpdateClock() {
    while (true) {
        m.lock();
        clock++;
        m.unlock();
        sem_post(&sem);
    }
}

int main() {
    // Read the "memconfig.txt" file
    ifstream memconfigFile;
    memconfigFile.open("memconfig.txt");
    int numPages;
    memconfigFile >> numPages;
    mainMemory.reserve(numPages);
    memconfigFile.close();

    // Read the "processes.txt" file
    ifstream processesFile;
    processesFile.open("processes.txt");
    int numCores;
    processesFile >> numCores;
    int numProcesses;
    processesFile >> numProcesses;
    queue<pair<int, int>> processesQueue;
    for (int i = 0; i < numProcesses; i++) {
        int startTime;
        processesFile >> startTime;
        int duration;
        processesFile >> duration;
        processesQueue.push({startTime, duration});
    }
    processesFile.close();

    // Read the "commands.txt" file
    ifstream commandsFile;
    commandsFile.open("commands.txt");
    vector<string> commands;
    while (getline(commandsFile, command)) {
        commands.push_back(command);
    }
    commandsFile.close();

    // Update the clock in a separate thread
    thread clockThread(UpdateClock);

    // Create threads for each process
    vector<thread> processThreads;
    for (int i = 0; i < numProcesses; i++) {
        processThreads.push_back(thread([&, i]() {
            int startTime = processesQueue.front().first;
            int duration = processesQueue.front().second;
            processesQueue.pop();
            while (clock < startTime) {
                sem_wait(&sem);
            }
            // Print the process start event
            cout << "Clock: " << clock << ", Process " << i + 1 << ": Started." << endl;
            for (int j = 0; j < duration; j++) {
                sem_wait(&sem);
                // Split the command
                stringstream ss(commands[j]);
                string commandType;
                getline(ss, commandType, ' ');
                if (commandType == "Store") {
                    string variableId;
                    getline(ss, variableId, ' ');
                    unsigned int value;
                    ss >> value;
                    // Print the store command event
                    cout << "Clock: " << clock << ", Process " << i + 1 << ", Store: Variable " << variableId << ", Value: " << value << endl;
                    // Store the page
                    Store(variableId, value);
                } else if (commandType == "Lookup") {
                    string variableId;
                    getline(ss, variableId, ' ');
                    // Print the lookup command event
                    cout << "Clock: " << clock << ", Process " << i + 1 << ", Lookup: Variable " << variableId << ", ";
                    // Lookup the page
                    int value = Lookup(variableId);
                    cout << "Value: " << value << endl;
                } else if (commandType == "Release") {
                    string variableId;
                    getline(ss, variableId, ' ');
                    // Print the release command event
                    cout << "Clock: " << clock << ", Process " << i + 1 << ", Release: Variable " << variableId << endl;
                    // Release the page
                    Release(variableId);
                }
            }
            // Print the process end event
            cout << "Clock: " << clock << ", Process " << i + 1 << ": Finished." << endl;
        }));
    }

    // Join the threads
    clockThread.join();
    for (int i = 0; i < processThreads.size(); i++) {
        processThreads[i].join();
    }

    return 0;
}