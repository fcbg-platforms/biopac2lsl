#include <QCoreApplication>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "mpdev/mpdev.h"
#include "lsl/lsl_cpp.h"
#include <algorithm>
#include <conio.h>

using namespace std;

//char* getCmdOption(char **, char **, const std::string&);
//bool cmdOptionExists(char**, char**, const std::string&);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    BOOL analogCH[] = {true, false, false, false};
    uint nChan = 0;
    // count number of active channels
    for (uint k = 0;  k < (sizeof(analogCH)/sizeof(*analogCH)); k++) {
        if (analogCH[k] == true){
            nChan++;
        }
    }

    const double sampleRate = 1.0; // sample rate in ms

    cout << "Hello World!\n";

    MPRETURNCODE retval;

    cout << "Connecting to MP device...";
    retval = connectMPDev(MP36, MPUSB, "auto");
    if(retval != MPSUCCESS){
        cout << "\n";
        cout << "Program failed to connect to MP Device\n";
        cout << "connectMPDev returned with " << retval << " as a return code.\n";
        cout << "Disconnecting...\n";
        disconnectMPDev();
        cout << "Exit\n" << endl;
        return 0;
    } else {
        cout << "[Done]\n";
    }

    cout << "Setting active analog channels...";
    retval = setAcqChannels(analogCH);
    if(retval != MPSUCCESS){
        cout << "\n";
        cout << "Program failed to setup active analog channels\n";
        cout << "setAcqChannels returned with " << retval << " as a return code.\n";
        cout << "Disconnecting...\n";
        disconnectMPDev();
        cout << "Exit\n" << endl;
        return 0;
    } else {
        cout << "[Done]\n";
    }

    cout << "Setting sample rate at " << sampleRate << " ms per sample...";
    retval = setSampleRate(sampleRate);
    if(retval != MPSUCCESS){
        cout << "\n";
        cout << "Program failed to setup sample rate\n";
        cout << "setSampleRate returned with " << retval << " as a return code.\n";
        cout << "Disconnecting...\n";
        disconnectMPDev();
        cout << "Exit\n" << endl;
        return 0;
    } else {
        cout << "[Done]\n";
    }

    cout << "Starting acquisition daemon...";
    retval =  startMPAcqDaemon();
    if(retval != MPSUCCESS){
        cout << "\n";
        cout << "Program failed to start acquisition daemon\n";
        cout << "startMPAcqDaemon returned with " << retval << " as a return code.\n";
        cout << "Disconnecting...\n";
        stopAcquisition();
        disconnectMPDev();
        cout << "Exit\n" << endl;
        return 0;
    } else {
        cout << "[Done]\n";
    }

    cout << "Starting acquisition...";
    retval = startAcquisition();
    if(retval != MPSUCCESS){
        cout << "\n";
        cout << "Program failed to start acquisition\n";
        cout << "startAcquisition returned with " << retval << " as a return code.\n";
        cout << "Disconnecting...\n";
        stopAcquisition();
        disconnectMPDev();
        cout << "Exit\n" << endl;
        return 0;
    } else {
        cout << "[Done]\n";
    }

    // initialize MP buffer
    uint buffer_len_ms = 10; // buffer length in ms
    DWORD buffer_len = buffer_len_ms*sampleRate;
    DWORD valuesRead = 0;
    double * buffer = new double[buffer_len];


    // initialize LSL stream
    lsl::stream_info data_info("MP", "EEG", nChan, 1000/sampleRate, lsl::cf_double64, "whatever");
    lsl::stream_outlet data_outlet(data_info);

    lsl::xml_element channels = data_info.desc().append_child("channels");

    // set channel labels for LSL stream
    char ch_name[50];
    for (uint k = 0;  k < (sizeof(analogCH)/sizeof(*analogCH)); k++) {
        if(analogCH[k]==true){
            sprintf(ch_name,"CH%d",k);
            channels.append_child("channel")
                    .append_child_value("label", ch_name)
                    .append_child_value("type", "EEG")
                    .append_child_value("unit", "uV");
        }

    }

    // push data to LSL outlet
    cout << "Pushing data to LSL outlet...\n";
    cout << "Press q to stop\n";
    bool stop = false;
    double start_time = lsl::local_clock();
    double elapsed_time = 0;
    uint global_nSample = 0;
    double duration = 120;
    int isKey_press = 0;
    int key_press;
    bool is_waiting_quit_confirmation = false;
    while(!stop){
        receiveMPData(buffer,buffer_len, &valuesRead);
        double now = lsl::local_clock();
        uint nSample = static_cast<unsigned int>(valuesRead / nChan);
        global_nSample += nSample;
        std::vector<std::vector<double>> send_buffer(nSample, std::vector<double>(nChan));
        for(uint i=0;i<nSample;i++){
            for(uint j=0;j<nChan;j++){
                send_buffer[i][j] = buffer[nChan*i + j]*1e6;
            }
        }
        data_outlet.push_chunk(send_buffer, now);
        elapsed_time = lsl::local_clock() - start_time;
        if (elapsed_time >= duration){
            stop = true;
        }

        isKey_press = _kbhit();

        if (isKey_press){
            key_press = _getch();
            //cout << "key_press " << key_press << "\n";
            if (is_waiting_quit_confirmation){
                if (key_press == 121){
                    stop = true;
                }
                if (key_press == 110){
                    is_waiting_quit_confirmation = false;
                    cout << "Pushing data to LSL outlet...\n";
                    cout << "Press q to stop\n";
                }
            } else {
                if (key_press == 113){
                    is_waiting_quit_confirmation = true;
                    cout << "Do you really want to stop acquisition? [y/n]\n";
                }
            }
        }
    }

    stopAcquisition();
    disconnectMPDev();
    cout << "Acquisition stopped\n";

    cout << "Read " << global_nSample << " samples in " << elapsed_time << " s\n";
    //cout << "Read " << global_nSample << " (" << elapsed_time*sampleRate*1000 << ") samples in " << elapsed_time << " s\n";
    cout << "Estimated sampling frequency " << global_nSample/elapsed_time << "Hz\n";

    cout << "Press ctrl+c to quit\n";
    return a.exec();
}


/*
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}
*/

