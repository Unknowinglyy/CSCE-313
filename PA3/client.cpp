#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;


void patient_thread_function (int patient_number, int n, BoundedBuffer& request_buffer) {
    // functionality of the patient threads

    //take in a patient number
    //for n requests, produce a datamsg that is (patient number, time, ECCNO) and push it to request_buffer
    //  -time dependent on current requests
    //  -at 0 -> time = 0.000; at 1 -> time = 0.004; at 2 time -> 0.008; ...
    for(int i = 0; i < n; i++){
        double seconds = i * 0.004;
        datamsg msg = datamsg(patient_number, seconds, EGCNO);
        request_buffer.push((char*) &msg, sizeof(datamsg));
    }
}

void file_thread_function (string filename, BoundedBuffer& request_buffer,FIFORequestChannel* chan, int message_size) {
    // functionality of the file thread

    //get file size, to do this we use a filemsg(0,0) object and filename
    //from PA1
    filemsg fmsg(0, 0);
    int len = sizeof(filemsg) + filename.size() + 1;
    std::vector<char> buf(len); //need for variable length
    memcpy(buf.data(), &fmsg, sizeof(filemsg));
    strcpy(buf.data() + sizeof(filemsg), filename.c_str());
    chan->cwrite(buf.data(), len);
    __int64_t file_size = chan->cread(buf.data(), len);
    //open output file in write mode; allocate the memory with fseek from the beginning of the file to the file size; close the file
    FILE* file = fopen(filename.c_str(), "r");
    fseek(file, 0, SEEK_END);
    fclose(file);
    //initialize offset to 0
    __int64_t offset = 0;
    //while offset < file size, produce a filemsg(offset, msg) + filename and push to request_buffer
    while(offset < file_size){
        filemsg* msg = new filemsg(offset, message_size);
        //if offset + message_size > file size, then the length of the message should be file size - offset
        if(offset + message_size > file_size){
            //delete the old message and create a new one with the correct length
            delete msg;
            msg = new filemsg(offset, file_size - offset);
        }
        request_buffer.push((char*) msg, sizeof(filemsg));
        offset += message_size;
    }
}

void worker_thread_function (BoundedBuffer& request_buffer, BoundedBuffer& response_buffer, FIFORequestChannel* chan, int message_size) {
    // functionality of the worker threads
    
    //forever loop
    while(true){
        //pop a message from request_buffer
        char* msg = new char[message_size];
        request_buffer.pop(msg, message_size);
        //find the type of message (from server.cpp)
        MESSAGE_TYPE m = *((MESSAGE_TYPE*) msg);
        //if datamsg
        if(m == DATA_MSG){
            //send the request across the FIFO channel to the server
            chan->cwrite(msg, sizeof(datamsg));
            //receive the response from the server
            double response;
            chan->cread(&response, sizeof(response));
            //create pair of patient number and response from the server and push it to response_buffer
            datamsg* dmsg = (datamsg*) msg;
            pair<int, double> p(dmsg->person, response);
            response_buffer.push((char*) &p, sizeof(p));
            //delete the message
            delete[] msg;
        }
        //if filemsg
        if(m == FILE_MSG){
            //collect the file name from the message popped from request_buffer
            //filename is appeneded to the end of the filemsg
            filemsg* fmsg = (filemsg*) msg;
            char* filename = msg + sizeof(filemsg);
            //open the file in update mode
            FILE* file = fopen(filename, "a");
            //fseek(SEEK_SET) to the offset of the filemsg
            fseek(file, fmsg->offset, SEEK_SET);
            //write the buffer from the server
            fwrite(msg, 1, fmsg->length, file);
            //close the file
            fclose(file);
            //delete the message
            delete[] msg;
        }
        //if quitmsg
        if(m == QUIT_MSG){
            //delete the message
            delete[] msg;
            
            //break out of the loop
            break;
        }
    }
    //need quit condition to break out of loop
}

void histogram_thread_function (BoundedBuffer& response_buffer, HistogramCollection& hc, int message_size) {
    // functionality of the histogram threads

    //forever loop
    while(true){
        //need a quit condition where a special pair is pushed to the response_buffer which will break out of the loop
        //pop a response from response_buffer
        char* msg = new char[message_size];
        response_buffer.pop(msg, message_size);
        //check the datamsg values
        
        //create a pair from the response
        pair<int, double> value = *((pair<int, double>*) msg);
        //if the pair is the special pair, break out of the loop
        if(value.first == -1 && value.second == -1){
            //delete the message
            delete[] msg;
            //then break out of the loop
            break;
        }
        //call histogram collection update function with the patient number and response (HC.update(response->patient_number, response->value))
        hc.update(value.first, value.second);
        //delete the message
        delete[] msg;
    }
    //need quit condition to break out of loop
}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
		}
	}
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    
	// initialize overhead (including the control channel)
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
    // record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* create all threads here */
    //array of producer threads (if data, p elements, if file, 1 element)
    thread* producer_threads = new thread[p];
    thread* file_thread = new thread[1];

    //would this work better as a vector?
    vector<FIFORequestChannel*> FIFOs(w);

    //array of worker threads (w elements)
    thread* worker_threads = new thread[w];

    //array of histogram threads (if data h elements, if file no elements)
    //might as well make it no matter what
    thread* histogram_threads = new thread[h];

    //if datamsgs
    if(f == "") {
        //create p patient threads (store in producer array)
        for (int i = 0; i < p; i++) {
            producer_threads[i] = thread(patient_thread_function, i+1, n, ref(request_buffer));
        }
        //create h histogram threads (store in histogram array)
        for (int i = 0; i < h; i++) {
            histogram_threads[i] = thread(histogram_thread_function, ref(response_buffer), ref(hc),m);
        }
        MESSAGE_TYPE message = NEWCHANNEL_MSG;
        for (int i = 0; i < w; i++) {
            //create w channels within worker threads (store in FIFO array)
            chan->cwrite (&message, sizeof (message));
            //server uses 30 bytes for the name of the new channel so client needs to also
            char* buffer = new char[30];
            //get channel name from server by reading from control channel
            chan->cread(buffer, 30);
            string new_channel_name = buffer;
            FIFOs.at(i) = new FIFORequestChannel(new_channel_name, FIFORequestChannel::CLIENT_SIDE);
            //create w worker threads (store in worker array)
            worker_threads[i] = thread(worker_thread_function, ref(request_buffer), ref(response_buffer), FIFOs.at(i), m);

            //delete buffer
            delete[] buffer;
        }
    }
    //if file transfer
    else if(f != "") {
        //create 1 file thread (store in producer array)
        file_thread[0] = thread(file_thread_function, f, ref(request_buffer), chan, m);
        //same as before, need to prompt server with NEWCHANNEL_MSG
        MESSAGE_TYPE message = NEWCHANNEL_MSG;
        for (int i = 0; i < w; i++) {
            chan->cwrite (&message, sizeof (message));
            char* buffer = new char[30];
            chan->cread(buffer, 30);
            string new_channel_name = buffer;
            //create w channels within worker threads (store in FIFO array)
            FIFOs.at(i) = new FIFORequestChannel(new_channel_name, FIFORequestChannel::CLIENT_SIDE);
            //create w worker threads (store in worker array)
            worker_threads[i] = thread(worker_thread_function, ref(request_buffer), ref(response_buffer), FIFOs.at(i), m);
        }
    }

	/* join all threads here (order is important, probably want to join producer threads then consumer threads)*/
    //join patient threads first, then worker threads, then histogram threads
    //if data, close producer threads first
    if(f == "") {
        for (int i = 0; i < p; i++) {
            producer_threads[i].join();
        }
    }
    //if file, close file thread first
    else{
        file_thread[0].join();
    }
    //the producer threads/file threads have ended, so we can now push quit messages to the request buffer to signal to worker threads to stop
    //each worker thread needs a quit message, so we push w quit messages to the request buffer
    for (int i = 0; i < w; i++) {
        MESSAGE_TYPE q = QUIT_MSG;
        request_buffer.push((char*) &q, sizeof(MESSAGE_TYPE));
    }

    //then, no matter what, close worker threads then histogram threads
    for (int i = 0; i < w; i++) {
        worker_threads[i].join();
    }
    //same idea as before, however, this time we will push unique pair values to the response buffer to signal to the histogram threads to stop
    //each histogram thread needs a unique pair value, so we push h unique pair values to the response buffer
    for (int i = 0; i < h; i++) {
        pair<int, double> q(-1, -1);
        response_buffer.push((char*) &q, sizeof(pair<int, double>));
    }
    for (int i = 0; i < h; i++) {
        histogram_threads[i].join();
    }

	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    //quit and close all channels in FIFO array
    for (int i = 0; i < w; i++) {
        MESSAGE_TYPE q = QUIT_MSG;
        FIFOs.at(i)->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
        delete FIFOs.at(i);
    }

	// quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;
    
    //delete all threads here
    delete[] producer_threads;
    delete[] file_thread;
    delete[] worker_threads;
    delete[] histogram_threads;

    //don't need dynamically allocated memory in request and response buffers
    

	// wait for server to exit
	wait(nullptr);
}
