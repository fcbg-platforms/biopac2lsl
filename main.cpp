#include <QCoreApplication>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "mpdev/mpdev.h"
#include "lsl/lsl_cpp.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // ofstream myfile;
    // myfile.open ("example.txt");
    // myfile << "Hello World.\n";
    // myfile.close();

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

    retval = connectMPDev(MP36, MPUSB, "auto");
    cout << "connectMPDev returned " << retval << "\n";

    retval = setAcqChannels(analogCH);
    cout << "setAcqChannels returned " << retval << "\n";

    retval = setSampleRate(sampleRate);
    cout << "setSampleRate returned " << retval << "\n";

    retval =  startMPAcqDaemon();
    cout << "startMPAcqDaemon returned " << retval << "\n";

    retval = startAcquisition();
    cout << "startAcquisition returned " << retval << "\n";

    // initialize MP buffer
    uint buffer_len_ms = 10; // buffer length in ms
    DWORD buffer_len = buffer_len_ms*sampleRate;
    DWORD valuesRead = 0;
    double * buffer = new double[buffer_len];


    // initialize LSL stream
    lsl::stream_info data_info("MP", "EEG", nChan, 1000/sampleRate, lsl::cf_double64, "whatever");
    lsl::stream_outlet data_outlet(data_info);

    lsl::xml_element channels = data_info.desc().append_child("channels");

    // convert the eeg mask to a bits array for ch labelling
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

    bool stop = false;
    double start_time = lsl::local_clock();
    double elapsed_time = 0;
    uint global_nSample = 0;
    double duration = 60;
    while(elapsed_time < duration){
        retval = receiveMPData(buffer,buffer_len, &valuesRead);
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
    }
    cout << "read " << global_nSample << " (" << duration*sampleRate*1000 << ") " << " in " << elapsed_time << "s\n";
    cout << "estimated sampling frequency " << global_nSample/elapsed_time << "Hz\n";



    retval = stopAcquisition();
    cout << "stopAcquisition returned " << retval << "\n";

    disconnectMPDev();

    cout << "press ctrl+c to quit\n";
    return a.exec();
}





