/*
	Original author of the starter code
	Tanzir Ahmed
	Department of Computer Science & Engineering
	Texas A&M University
	Date: 2/8/20

	Please include your Name, UIN, and the date below
	Name: Blake Dejohn
	UIN: 531002472
	Date: 1/31/24
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;

/* Verifies if the commandline argument passed to client.cpp "num_of_servers"
   is one of the following numbers (2,4,8,16). */
void verify_server_count(int num_of_servers)
{
	/*
		TODO: Allow only 2, 4, 8 or 16 servers
		If the number of servers is any other number
		Print "There can only be 2, 4, 8 or 16 servers\n"
		In the next line, print "Exiting Now!\n"
	*/
	//checking for correct number of servers
	if(num_of_servers == 2 || num_of_servers == 4 || num_of_servers == 8 || num_of_servers == 16){

	}
	else{
		cout << "There can only be 2, 4, 8 or 16 servers\n" << endl;
	}
	cout << "Exiting Now!\n";
}

/* Establishes a new channel with the server by sending appropriate NEWCHANNEL_MSG message.
   Sets this new channel as main channel (point chan to new channel) for communication with the server. */
void set_new_channel(FIFORequestChannel *chan, FIFORequestChannel **channels, int buffersize, int serverInt)
{
	//create a new message
	MESSAGE_TYPE message = NEWCHANNEL_MSG;

	//using cwrite to write the message
	chan->cwrite(&message, sizeof(message));
	//creating a buffer
	char* buffer = new char[buffersize];
	//using cread to read the message
	chan->cread(buffer, buffersize);
	//naming the channel
	string cName(buffer);
	//creating the channel
	FIFORequestChannel* channel = new FIFORequestChannel(cName, FIFORequestChannel::CLIENT_SIDE);
	channels[serverInt] = channel;
	delete[] buffer;
	buffer = nullptr;
	cout << channels;

	/*  TODO:
		1)request new channel
		  Hint: use cwrite and cread

		2)open channel on client side

		3)set the above channel as the main channel

		Remember to clean up any object you allocated memory to
	*/
}

/* Send datamsg request to server using "chan" FIFO Channel and
   cout the response with serverID */
void req_single_data_point(FIFORequestChannel *chan,
						   int person,
						   double time,
						   int ecg)
{
	char buffer[MAX_MESSAGE];
	datamsg msg(person, time, ecg);
	memcpy(buffer, &msg, sizeof(datamsg));
	chan->cwrite(buffer, sizeof(datamsg));
	serverresponse response(-1,-1);
	chan->cread(&response, sizeof(serverresponse));

	/*
		TODO: Reading a single data point
		Hint: Go through datamsg and serverresponse classes
		1) sending a data msg
			Hint: use cwrite

		2) receiving response
			Hint: use cread
	*/

	// Don't Change the cout format present below
	cout << response.ecgData <<" data on server:"<< response.serverId << " on channel " << chan->name() << endl << endl;
}

/* Sends 1000 datamsg requests to one server through FIFO Channel and
   dumps the data to x1.csv regardless of the patient ID */
void req_multiple_data_points(FIFORequestChannel *chan,
							  int person,
							  double time,
							  int ecg)
{
	// TODO: open file "x1.csv" in received directory using ofstream
	ofstream myfile("recieved/x1.csv");

	// set-up timing for collection
	timeval t;
	gettimeofday(&t, nullptr);
	double t1 = (1.0e6 * t.tv_sec) + t.tv_usec;

	// requesting data points
	time = 0.0;

	ecg = ecg + 1;
	// TODO: Receive 1000 data points
	for (int i = 0; i < 1000; i++)
	{
		//first message
		char buffer[MAX_MESSAGE];
		datamsg msg(person, time, 1);
		memcpy(buffer, &msg, sizeof(datamsg));
		chan->cwrite(buffer, sizeof(datamsg));
		serverresponse response(-1,-1);
		chan->cread(&response, sizeof(serverresponse));

		//second message
		datamsg msg2(person, time, 2);
		memcpy(buffer, &msg2, sizeof(datamsg));
		chan->cwrite(buffer, sizeof(datamsg));
		serverresponse response2(-1,-1);
		chan->cread(&response2, sizeof(serverresponse));
		myfile << time << "," << response.ecgData << "," << response2.ecgData << flush;
		time += 0.004;
		myfile << endl;
	}

	// compute time taken to collect
	gettimeofday(&t, nullptr);
	double t2 = (1.0e6 * t.tv_sec) + t.tv_usec;

	// display time taken to collect
	cout << "Time taken to collect 1000 datapoints :: " << ((t2 - t1) / 1.0e6) << " seconds on channel " << chan->name() << endl;

	// closing file
	myfile.close();
}

/* Request Server to send contents of file (could be .csv or any other format) over FIFOChannel and
   dump it to a file in recieved folder */
void transfer_file(FIFORequestChannel *chan,
				   string filename,
				   int buffersize)
{
	// sending a file msg to get length of file
	// create a filemsg, buf, and use cwrite to write the message

	// receiving response, i.e., the length of the file
	// Hint: use cread

	// opening file to receive into

	// set-up timing for transfer
	timeval t;
	gettimeofday(&t, nullptr);
	double t1 = (1.0e6 * t.tv_sec) + t.tv_usec;
	//creating path to file (received)
	string file = "received/" + filename;
	ofstream file2(file);
	//testing to see if file opened correctly
	if(file2.is_open()){
		cout << "File is open" << endl;
	}
	else{
		cout << "File could not open" << endl;
	}
	//default message parameters
	filemsg msg(0,0);
	int size = sizeof(filemsg) + filename.size() + 1;
	char* buffer = new char[size];
	//copying memory from buffer then the actual string value
	memcpy(buffer, &msg, sizeof(filemsg));
	strcpy(buffer + sizeof(filemsg), filename.c_str());

	chan->cwrite(buffer, size);
	//using long long for size of file
	long long int size2 = 0;

	chan->cread(&size2, sizeof(long long int));

	char* buffer2 = new char[buffersize];
	//helper cout for file length and buffersize
	cout << "File length = " << size2 << " and the buffersize is" << buffersize << endl;
	for(int i = 0; i < size2; i = i + buffersize){
		//creating message buffer
		filemsg* msg2 = (filemsg*) buffer;
		//offset increased by buffersize
		msg2->offset = i;

		if(size2 - i < buffersize){
			msg2->length = size2 - i;
		}
		else{
			msg2->length = buffersize;
		}
		//writing to buffer, then reading, then writing to file
		chan->cwrite(buffer, size);
		chan->cread(buffer2, msg2->length);
		file2.write(buffer2, msg2->length);
	}
	//deleting buffers after done with them
	delete[] buffer;
	delete[] buffer2;
	// requesting file
	// TODO: Create a filemsg with the current offset you want
	// to read from. Receive data from the server.

	// compute time taken to transfer
	gettimeofday(&t, nullptr);
	double t2 = (1.0e6 * t.tv_sec) + t.tv_usec;

	// display time taken to transfer
	cout << "Time taken to transfer file :: " << ((t2 - t1) / 1.0e6) << " seconds" << endl
		 << endl;

	// closing file
	file2.close();
}

int main(int argc, char *argv[])
{
	int person = 0;
	double time = 0.0;
	int ecg = 0;

	int buffersize = MAX_MESSAGE;

	string filename = "";

	bool datachan = false;

	int num_of_servers = 2;

	int opt;
	while ((opt = getopt(argc, argv, "p:t:e:m:f:s:c")) != -1)
	{
		switch (opt)
		{
		case 'p':
			person = atoi(optarg);
			break;
		case 't':
			time = atof(optarg);
			break;
		case 'e':
			ecg = atoi(optarg);
			break;
		case 'm':
			buffersize = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		case 's':
			num_of_servers = atoi(optarg);
			break;
		case 'c':
			datachan = true;
			break;
		}
	}

	/*
		Example Code Snippet for reference, you can delete after you understood the basic functionality.

		FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

		// example data point request
		char buf[MAX_MESSAGE]; // 256
		datamsg x(1, 0.0, 1);

		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question

		serverresponse resp(-1,-1);
		chan->cread(&resp, sizeof(serverresponse)); //answer

		cout << resp.ecgData <<" data on server:"<< resp.serverId << " on channel " << chan->name() << endl << endl;

		// sending a non-sense message, you need to change this
		filemsg fm(0, 0);
		string fname = "teslkansdlkjflasjdf.dat";

		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len);  // I want the file length;

		delete[] buf2;

		// closing the channel
		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));

	*/

	verify_server_count(num_of_servers);

	bool data = ((person != 0) && (time != 0.0) && (ecg != 0));
	bool file = (filename != "");
	// Initialize FIFORequestChannel channels.
	// What should be the size of the object?
	// HINT: We need n channels(one for each of the servers) plus one more for new datachannel request.
	//initialize channels
	FIFORequestChannel** newChannels = new FIFORequestChannel*[num_of_servers + 1];
	for(int i = 0; i <= num_of_servers; i++){
		int x = fork();
		//child process
		if(x == 0){
			string serverID = to_string(i+1);
			string buffer = to_string(buffersize);
			const char* cBuffer = buffer.c_str();
			vector<const char*> arguments = {"./server", "-m", cBuffer, "-s", serverID.c_str(), nullptr};
			arguments.push_back(nullptr);
			execvp(arguments[0], const_cast <char* const*>(arguments.data()));
		}
		else{
			newChannels[i] = new FIFORequestChannel("control_" + to_string(i+1)+ "_", FIFORequestChannel::CLIENT_SIDE);
		}
	}

	int y = 16 / num_of_servers;
	int z = ceil(person / y);
	//creating new channel array
	FIFORequestChannel* chans = newChannels[z];



	// fork to create servers

	// open all requested channels

	// Initialize step and serverId

	if (!filename.empty())
	{
		string patient = filename.substr(0, filename.find('.'));
		std::istringstream ss(patient);
		int patient_id;
		ss >> patient_id;

		if(ss.fail()){
			z = 1;
		}
		else{
			y = 16 / num_of_servers;
			z = ceil (static_cast<float> (patient_id) / y);
		}
		// If ss has a patient id, set serverId using ceil again
		// else, use the first server for file transfer
	}

	// By this line, you should have picked a server channel to which you can send requests to.

	// TODO: Create pointer for all channels

	if (datachan)
	{
		// call set_new_channel
		set_new_channel(chans, newChannels, buffersize, z);
	}

	// request a single data point
	if (data)
	{
		// call req_single_data_point
		req_single_data_point(chans, person, time, ecg);
	}

	// request a 1000 data points for a person
	if (!data && person != 0)
	{
		// call req_multiple_data_points
		req_multiple_data_points(chans, person, time, ecg);
	}

	// transfer a file
	if (file)
	{
		// call transfer_file
		cout << "File transfer requested \n";
		transfer_file(chans, filename, buffersize);
	}
	// close the data channel
	// if datachan has any value, cwrite with channels[0]

	// close and delete control channels for all servers

	// wait for child to close
	MESSAGE_TYPE msg = QUIT_MSG;
	//deleting the channels
	for(int i = 0; i <= num_of_servers; i++){
		newChannels[i]->cwrite(&msg, sizeof(MESSAGE_TYPE));
		delete newChannels[i];
	}
	//deleting the array
	delete[] newChannels;

	//wait for child processes to be done
	wait(NULL);
}
