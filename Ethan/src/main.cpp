#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <thread>
#include <chrono>
#if defined(WRITE_TO_FILE) || defined(READ_FROM_FILE)
#include <fstream>
#include <sys/stat.h>

#endif
#include <chrono>
#include <cstdlib>
// Message of interest
// Tangram Pro-generated messages
#include "hi/ethanToMichael.hpp"
#include "hi/michaelToEthan.hpp"
#include "hi/messageStruct.hpp"

// Generic transport
#include "TangramTransport.hpp"
static tangram::transport::TangramTransport *transport = nullptr;

#include "hi_DerivedEntityFactory.hpp"


#include "LMCPSerializer.hpp"


//******************************************************************************
/**
 * @brief This function initializes whatever the generic transport is and
 *        configures it statically (without the config file).  Note that a lot
 *        of what's in this function can go away if file-based configuration is
 *        used instead.fffff
 *
 * @return int 0 on success or -1 on error.
 */
static int initTransport(uint64_t flags) {
    // create the transport object
    transport = tangram::transport::TangramTransport::createTransport();
    if (nullptr == transport) {
        fprintf(stderr, "Could not create trffansport!\n");
        return -1;
    }
    std::string hostname = "127.0.0.1";

    char* hn = std::getenv("TANGRAM_TRANSPORT_zeromq_transport_HOSTNAME");
    printf("Hostname: %s\n", hn);
    if(hn)
    {

        hostname.assign(hn);
    }
    // the ports must match when testing on tangram (runnign without zmq proxy)
    transport->resetTransportOptions();
    transport->setOption("PublishIP", hostname);
    transport->setOption("PublishPort", "6667"); 
    transport->setOption("SubscribeIP", hostname);
    transport->setOption("SubscribePort", "6668");
    transport->setOption("PublishID", "0");
    // open it
    if (transport->open(flags) == -1) {
        fprintf(stderr, "Could not open transport!\n");
        return -1;
    }

    return 0;
}

//******************************************************************************
int main(int argc, char *argv[]) {

    // setup the transport
    if (initTransport(TTF_WRITE) != 0) {
        fprintf(stderr, "Transport initialization failed!\n");
        exit(1);
    }

    // some transports require a brief delay between initialization and sending
    // of data
    sleep(1);

    // the message object
    //afrl::cmasi::AirVehicleState msg;


    // initialize the serializer
    tangram::serializers::Serializer *serializer;
    hi::DerivedEntityFactory derivedEntityFactory;

     tangram::serializers::LMCPSerializer lmcpSerializer(&derivedEntityFactory);
     serializer = &lmcpSerializer;



    // dump the message to the console
    //mess.dump();

    //Initialize Message
    hi::messageStruct mess; 
    mess.setNum(1.0, true);

    hi::ethanToMichael e2m;
    e2m.setWaypoint(&mess, true);
    
    e2m.dump();


    while(true) {

        std::vector<uint8_t> bytes;
        if (!serializer->serialize(e2m, bytes)) 
        {
            transport->close();
            delete transport;
            fprintf(stderr, "Failed to serialize message.\n");
            _exit(1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));


        if (transport->publish(&bytes[0], bytes.size(), "e2m", 0) < 0) 
        {
            fprintf(stderr, "Failed to publish message.\n");
            transport->close();
            delete transport;
            _exit(1);
        }
        fprintf(stderr, "SentMessage\n");
    }
    sleep(2);

    printf("Closing transport.\n");
    transport->close();
    delete transport;

    printf("Done\n");
    return 0;
}
