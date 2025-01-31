#include "Process.h"

// Constructor
Process::Process() :pid(0), arrival_time(0), cpu_burst_time(0), remaining_time(0), isCompleted(false), isCritical(false) {
}

// Constructor
Process::Process(int pid, int arrival_time, int cpu_burst_time, bool isCritical) {
    this->pid = pid;
    this->arrival_time = arrival_time;
    this->cpu_burst_time = cpu_burst_time;
    this->remaining_time = cpu_burst_time; // timer hasn't started yet, so it's full time
    this->isCompleted = false;
    this->isCritical = isCritical;
}

// Copy Construcor
Process::Process(const Process &p){
    this->pid = p.pid;
    this->arrival_time = p.arrival_time;
    this->cpu_burst_time = p.cpu_burst_time;
    this->remaining_time = p.remaining_time;
    this->isCompleted = p.isCompleted;
    this->isCritical = p.isCritical;
}

// Every time, When your process is running, use this function to update 
// the remaining time and monitor if the process is done or not
void Process::Run(int run_time) {
    // update remaining_time
    this->remaining_time -= run_time;

    // isCompleted updated based on remaining time or not
    isCompleted = (this->remaining_time == 0);
}

// getter functions
int Process::getPid() const {
    return this->pid;
}
int Process::get_arrival_time() const {
	return this->arrival_time;
}
int Process::get_cpu_burst_time() const {
    return this->cpu_burst_time;
}
int Process::get_remaining_time() const {
	return this->remaining_time;
}
bool Process::is_Completed() const {
    return this->isCompleted;
}
bool Process::is_Proc_Critical() const {
    return this->isCritical;
}
// Assignment Operator Overloading
Process & Process::operator=(const Process & p)
{
    this->pid = p.pid;
    this->arrival_time = p.arrival_time;
    this->cpu_burst_time = p.cpu_burst_time;
    this->remaining_time = p.remaining_time;
    this->isCompleted = p.isCompleted;
    this->isCritical = p.isCritical;
    return *this;
}
