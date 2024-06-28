#include "RoundRobin.h"

/*
This is a constructor for RoundRobin Scheduler, you should use the extractProcessInfo function first
to load process information to process_info and then sort process by arrival time;

Also initialize time_quantum
*/
RoundRobin::RoundRobin(string file, int time_quantum) : time_quantum(time_quantum) {
	extractProcessInfo(file);
	sort(processVec.begin(), processVec.end(), [](Process& a, Process& b){
		return a.get_arrival_time() < b.get_arrival_time();
	});
	// vector<Process> readyProc;
	// vector<Process> activeProc;
}

// Schedule tasks based on RoundRobin Rule
// the jobs are put in the order the arrived
// Make sure you print out the information like we put in the document
void RoundRobin::schedule_tasks() {
/* uncomment these two lines; commented to allow compilation */
	int timestep = 0;
	int quantum_remaining = time_quantum;
	while(processVec.size() || processQ.size()) {
        /* Task 1 */
		// get new tasks
        // Go through the processVec
        // if the arrival time is less than or equal the timestep
		// push the process into the processQ vector
		// else go to the next process
		if(processVec.size() > 0){
			while(processVec.at(0).get_arrival_time() <= timestep){
				//if process has arrived before the end of the the current processes running time, then add it to the queue of processes that need to be done
				//move from processVec to processQ
				processQ.push(processVec.at(0));
				processVec.erase(processVec.begin());

				if(processVec.size() == 0){
					break;
				}
			}
		}


        /* Task 2 */
		// is there a task to do?
        // print timestep and set appropriate process not completed flag
        // increase timestep
        // set quantum_remaining to correct value
        if(processQ.empty()) {
			//-1 denotes a NOP
			this->print(timestep, -1, false);
			timestep++;
			continue;
        }

        /* Task 3 */
		// get current task from processQ
		Process& currentTask = processQ.front();

        /* Task 4 */
        // special condition: is the process critical?
		if(currentTask.is_Proc_Critical()){
			this->print(timestep, currentTask.getPid(), currentTask.is_Completed());
			//timestep gets updated by adding the burst time when process is critical
			timestep += currentTask.get_cpu_burst_time();

			currentTask.Run(currentTask.get_remaining_time());
			quantum_remaining = time_quantum;
		}
            // print a new process running with appropriate flag
            // update timestep appropriately
            // Run the process
            // reset quantum_remaining


        /* Task 5 */
		// is it done? Check if remaining time is 0
		//checking if the process is done running
		if(currentTask.get_remaining_time() == 0){
			//update the process and then pop it off the running queue
			currentTask.Run(0);
			this->print(timestep, currentTask.getPid(), true);
			processQ.pop();

			quantum_remaining = time_quantum;
			continue;
		}
            // print a new process running with appropriate flag
            // remove from processQ
            // set quantum_remaining to correct value
        if(quantum_remaining <= 0) {
			if(!processQ.empty()){
				processQ.pop();
			}
			//process is not done running and therefore should be added back to the queue
            Process newProc(currentTask);
			processQ.push(newProc);
			//find quantum remaining is an invalid value, reset it.
			quantum_remaining = time_quantum;
			continue;
        }

        /* Task 6 */
		//has the quantum expired?
            // pop the process from processQ and push it to the back
            // set quantum_remaining to correct value
        // if(false/* condition */) {
        //     continue;
        // }

        /* Task 7 */
		// otherwise...
        // print timestep of an incomplete process
        // run the process for one count
        // decrement quantum remaining
        // increment timestep
		//if all other checks fail, this kicks in for the processes left behind
		this->print(timestep, currentTask.getPid(), false);
		currentTask.Run(1);
		quantum_remaining--;
		timestep++;
	}
}


/*************************** 
ALL FUNCTIONS UNDER THIS LINE ARE COMPLETED FOR YOU
You can modify them if you'd like, though :)
***************************/


// Default constructor
RoundRobin::RoundRobin() {
	time_quantum = 0;
}

// Time quantum setter
void RoundRobin::set_time_quantum(int quantum) {
	this->time_quantum = quantum;
}

// Time quantum getter
int RoundRobin::get_time_quantum() {
	return time_quantum;
}

// Print function for outputting system time as part of the schedule tasks function
void RoundRobin::print(int system_time, int pid, bool isComplete){
	string s_pid = pid == -1 ? "NOP" : to_string(pid);
	cout << "System Time [" << system_time << "].........Process[PID=" << s_pid << "] ";
	if (isComplete)
		cout << "finished its job!" << endl;
	else
		cout << "is Running" << endl;
}

bool
string2bool (const std::string &v)
{
    return !v.empty () &&
        (strcasecmp (v.c_str (), "true") == 0 ||
         atoi (v.c_str ()) != 0);
}

// Read a process file to extract process information
// All content goes to proces_info vector
void RoundRobin::extractProcessInfo(string file){
	// open file
	ifstream processFile (file);
	if (!processFile.is_open()) {
		perror("could not open file");
		exit(1);
	}

	// read contents and populate process_info vector
	string curr_line, temp_num;
	int curr_pid, curr_arrival_time, curr_burst_time;
    bool isCritical;
	while (getline(processFile, curr_line)) {
		// use string stream to seperate by comma
		stringstream ss(curr_line);
		getline(ss, temp_num, ',');
		curr_pid = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_arrival_time = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_burst_time = stoi(temp_num);
		getline(ss, temp_num, ',');
        isCritical = string2bool(temp_num);
		Process p(curr_pid, curr_arrival_time, curr_burst_time, isCritical);

		processVec.push_back(p);
	}

	// close file
	processFile.close();
}
