#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

// Tangram Pro-generated serializer and transport
#include "Serializer.hpp"
#include "TangramTransportTypes.h"
#include "hi_DerivedEntityFactory.hpp"
#include "TangramTransport.hpp"
#include "LMCPSerializer.hpp"

// Tangram Pro-generated messages
#include "hi/ethanToMichael.hpp"
#include "hi/michaelToEthan.hpp"
#include "hi/messageStruct.hpp"


using namespace tangram;
using namespace genericapi;
using namespace serializers;
using namespace transport;
//using namespace std::chrono_literals;

bool sendMessage(Message& m, std::shared_ptr<TangramTransport> tport, LMCPSerializer& ser) {
    static std::vector<uint8_t> buffer;

    buffer.clear();

    if (!ser.serialize(m, buffer)) {
        std::cerr << "Failed to serialize message " << m.getName() << std::endl;
        return false;
    }

    std::string topic = "afrl.cmasi." + m.getName();

    if (!tport->publish(buffer.data(), buffer.size(), topic)) {
        std::cerr << "Failed to publish message " << m.getName() << std::endl;
        return false;
    }

    return true;
}


bool recvMessage(
    Message& msg,
    std::shared_ptr<TangramTransport> tport,
    LMCPSerializer& ser
) {
    static std::vector<uint8_t> buffer;

    buffer.resize(tport->getMaxReceiveSize());
    int32_t count = tport->recv(buffer.data(), buffer.size());
    if (count < 0) {
        std::cerr << "Failed to receive bytes for " << msg.getName() << std::endl;
        return false;
    }
    buffer.resize(count);
    std::cout << "Received bytes for " << msg.getName() << std::endl;

    if (ser.deserialize(buffer, msg)) {
        std::cout << "Deserialized " << msg.getName() << std::endl;
        return true;
    } else {
        std::cerr << "Failed to deser " << msg.getName() << std::endl;
    }

    return false;
}

uint8_t recvEitherMessage(
    Message& msg1,
    std::shared_ptr<TangramTransport> tport,
    LMCPSerializer& ser
) {
    static std::vector<uint8_t> buffer;

    buffer.resize(tport->getMaxReceiveSize());
    int32_t count = tport->recv(buffer.data(), buffer.size());
    if (count < 0) {
        std::cerr << "Failed to receive bytes for " << msg1.getName() << std::endl;
        return false;
    }
    buffer.resize(count);

    if (ser.deserialize(buffer, msg1)) {
        std::cout << "Deserialized " << msg1.getName() << std::endl;
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    // Collect args
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    // First try to set configuration from environment variables. Then, try to set from args
    // Args should override env
    std::string ip = "127.0.0.1";
    char* maybe_value = std::getenv("TANGRAM_TRANSPORT_zeromq_transport_HOSTNAME");
    if (maybe_value != nullptr) {
        ip = maybe_value;
    }
    if (args.size() > 1) {
        ip = args[1];
    }

    std::string pub_port = "6667";
    std::string sub_port = "6668";
    // Expected to be of form 6667,6668 (pub_port,sub_port)

    maybe_value = std::getenv("TANGRAM_TRANSPORT_zeromq_transport_PORTS");
    if (maybe_value != nullptr) {
        std::string ports(maybe_value);
 
        // split the ports at the comma
        auto comma_pos = ports.find(",");
        if (comma_pos == std::string::npos) {
            std::cerr << "Unexpected lack of comma in PORTS env variable" << std::endl;
        } else {
            // pub port, then sub port
            pub_port = ports.substr(0, comma_pos);
            sub_port = ports.substr(comma_pos + 1);
        }
    }
    if (args.size() > 2) {
        sub_port = args[2];
    }
    if (args.size() > 3) {
        pub_port = args[3];
    }

    // Configure the factory & serializer
    //afrl::cmasi::DerivedEntityFactory factory;
    hi::DerivedEntityFactory factory; 
    LMCPSerializer serializer(&factory);

    // Configure the transport
    std::shared_ptr<TangramTransport> tx = std::shared_ptr<TangramTransport>(TangramTransport::createTransport());
    std::shared_ptr<TangramTransport> rx = std::shared_ptr<TangramTransport>(TangramTransport::createTransport());
    if (tx == nullptr || rx == nullptr) {
        std::cerr << "Failed to create transport" << std::endl;
        exit(1);
    }

    rx->setOption("SubscribeIP", ip);
    rx->setOption("SubscribePort", sub_port);
    tx->setOption("PublishIP", ip);
    tx->setOption("PublishPort", pub_port);

    if (-1 == tx->open(TTF_WRITE)) {
        std::cerr << "Failed to open tx transport" << std::endl;
        return 1;
    }
    std::cout << "Opened tx transport" << std::endl;
    if (-1 == rx->open(TTF_READ)) {
        std::cerr << "Failed to open rx transport" << std::endl;
        return 1;
    }
    std::cout << "Opened rx transport" << std::endl;

    // Subscribe to the topic that the message will come in on
    rx->subscribe("hi.michaelToEthan");


    // Give the transport time to initialize & connect to the proxy
    //std::this_thread::sleep_for(10ms);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));


    hi::messageStruct mess; 
    mess.setNum(1.0, true);
        
    hi::ethanToMichael e2m;
    e2m.setWaypoint(&mess, true);


    if (!sendMessage(e2m, tx, serializer)) {
        std::cerr << "Failed to send first MichaelToEthan" << std::endl;
        return 1;
    }
    std::cout << "Sent MichaelToEthan" << std::endl;

    hi::michaelToEthan m2e;
    auto msgid = recvEitherMessage(m2e, rx, serializer);
    if (msgid == 1) {
        std::cout << "Received michaelToEthan" << std::endl;
    } 
   else {
        std::cerr << "Failed to receive a proper message to start any sequence" << std::endl;
        return 1;
    }

    return 0;
}
