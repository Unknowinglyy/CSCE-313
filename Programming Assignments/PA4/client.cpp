#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "TCPRequestChannel.h"
//code gotten from TA
// ecgno to use for datamsgs
#define ECGNO 1

using namespace std;


struct PVPair {
	int p;
	double v;
	
	PVPair (int _p, double _v) : p(_p), v(_v) {}
};

void patient_thread_function (int n, int pno, BoundedBuffer* rb) {
    datamsg d(pno, 0.0, ECGNO);

    // populate data messages
    for (int i = 0; i < n; i++) {
        rb->push((char*) &d, sizeof(datamsg));
        d.seconds += 0.004;
    }
}

void file_thread_function (string fname, TCPRequestChannel* chan, int b, BoundedBuffer* rb) {
    // open receiving file and truncate appropriate number of bytes
    filemsg f(0, 0);
    int buf_size = sizeof(filemsg) + fname.size() + 1;
    char* buf = new char[buf_size];
    memcpy(buf, &f, sizeof(filemsg));
    strcpy(buf + sizeof(filemsg), fname.c_str());
    
    chan->cwrite(buf, buf_size);
	
    __int64_t flen;
    chan->cread(&flen, sizeof(__int64_t));
    
    FILE* myfile = fopen(("received/" + fname).c_str(), "wb");
    fseek(myfile, flen, SEEK_SET);
    fclose(myfile);

    // populate file messages
	__int64_t remainingbytes = flen;
    int buffer = b;
    int offset = 0;
    filemsg* fm = (filemsg*) buf;
    while (remainingbytes > 0) {
        if (buffer > remainingbytes) {
            buffer = remainingbytes;
        }
        offset = flen - remainingbytes;

        fm->offset = offset;
        fm->length = buffer;

        rb->push(buf, buf_size);

        remainingbytes -= buffer;
    }
    delete[] buf;
}

void worker_thread_function (TCPRequestChannel* chan, BoundedBuffer* rb, BoundedBuffer* sb, int m) {
    char* buf = new char[m];
    double result = 0.0;
    char* recvbuf = new char[m];
    
    while (true) {
        rb->pop(buf, m);
        MESSAGE_TYPE* m = (MESSAGE_TYPE*) buf;
    
        if (*m == DATA_MSG) {
            chan->cwrite(buf, sizeof(datamsg));
            chan->cread(&result, sizeof(double));
			PVPair pv(((datamsg*) buf)->person, result);
            sb->push((char*) &pv, sizeof(PVPair));
        }
        else if (*m == FILE_MSG) {
            filemsg* fm = (filemsg*) buf;
            string fname = (char*) (fm + 1);
            chan->cwrite(buf, (sizeof(filemsg) + fname.size() + 1));
			chan->cread(recvbuf, fm->length);

            FILE* myfile = fopen(("received/" + fname).c_str(), "rb+");
			fseek(myfile, fm->offset, SEEK_SET);
            fwrite(recvbuf, 1, fm->length, myfile);
            fclose(myfile);
        }
        else if (*m == QUIT_MSG) {
            chan->cwrite(m, sizeof(MESSAGE_TYPE));
            delete chan;
            break;
        }
    }
    delete[] buf;
    delete[] recvbuf;
}

void histogram_thread_function (BoundedBuffer* rb, HistogramCollection* hc) {
	char buf[sizeof(PVPair)];
	while (true) {
		rb->pop(buf, sizeof(PVPair));
		PVPair pv = *(PVPair*) buf;
		if (pv.p <= 0) {
			break;
		}
		hc->update(pv.p, pv.v);
	}
}


int main (int argc, char *argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 30;		// default capacity of the request buffer
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    string a = "127.0.0.1"; //IP address
    string r = "8080"; //port number
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:a:r:")) != -1) {
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
            case 'a':
                a = optarg;
                break;
            case 'r':
                r = optarg;
                break;
		}
	}
    bool filereq = (f != "");
    
	// fork and exec the server
    // int pid = fork();
    // if (pid == 0) {
    //     execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    // }
    
	// control overhead (including the control channel)
	TCPRequestChannel* chan = new TCPRequestChannel(a, r);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }

    // making worker channels
    vector<TCPRequestChannel*> wchans;
    char* chname = new char[m];
    for (int i = 0; i < w; i++) {
        TCPRequestChannel *worker_channel = new TCPRequestChannel(a,r);
        wchans.push_back(worker_channel);
    }
    delete[] chname;
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* Start all threads here */
    vector<thread> patients;
    thread file;
    if (!filereq) {
        for (int i = 0; i < p; i++) {
            patients.push_back(thread(patient_thread_function, n, (i+1), &request_buffer));
        }
    }
    else {
        file = thread(file_thread_function, f, chan, m, &request_buffer);
    }

    vector<thread> workers;
    for (int i = 0; i < w; i++) {
        workers.push_back(thread(worker_thread_function, wchans[i], &request_buffer, &response_buffer, m));
    }

    vector<thread> hists;
    for (int i = 0; i < h; i++) {
        hists.push_back(thread(histogram_thread_function, &response_buffer, &hc));
    }
	
	/* Join all threads here */
    if (!filereq) {
        for (int i = 0; i < p; i++) {
            patients[i].join();
        }
    }
    else {
        file.join();
    }

    for (int i = 0; i < w; i++) {
        MESSAGE_TYPE q = QUIT_MSG;
        request_buffer.push((char*) &q, sizeof(MESSAGE_TYPE));
    }

    for (int i = 0; i < w; i++) {
        workers[i].join();
    }
	
	for (int i = 0; i < h; i++) {
		PVPair pv(0, 0);
		response_buffer.push((char*) &pv, sizeof(PVPair));
	}
	
	for (int i = 0; i < h; i++) {
		hists[i].join();
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

	// quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;
}