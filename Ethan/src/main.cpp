#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "TangramTransport.hpp"
#include "TangramTransportZMQ.hpp"
#include "DirectSerializer.hpp"
#include "hi_DerivedEntityFactory.hpp"

#include "hi/ethanToMichael.hpp"
#include "hi/michaelToEthan.hpp"

using namespace tangram;
using namespace genericapi;
using namespace serializers;
using namespace transport;

int init_transports(std::shared_ptr<TangramTransport> &tx, std::shared_ptr<TangramTransport> &rx, bool addSubs = true);
bool sendMessage(Message &m, std::shared_ptr<TangramTransport> tport, DirectSerializer &ser);
bool recvMessage(Message &msg, std::shared_ptr<TangramTransport> tport, DirectSerializer &ser);
int add_subscriptions(std::shared_ptr<TangramTransport> &rx);
int handle_messages(std::shared_ptr<TangramTransport> &tx, std::shared_ptr<TangramTransport> &rx, DirectSerializer &ser);

int main()
{
    std::cout << "Starting component" << std::endl;

    std::shared_ptr<TangramTransport> tx;
    std::shared_ptr<TangramTransport> rx;
    if (init_transports(tx, rx) != 0)
    {
        std::cerr << "Failed to initialize transports" << std::endl;
        exit(1);
    }

    hi::DerivedEntityFactory factory;
    DirectSerializer serializer(&factory);

    auto ret = handle_messages(tx, rx, serializer);
    if (ret != 0)
    {
        std::cout << "Early exit while handling messages: " << ret << std::endl;
        return ret;
    }

    std::cout << "Done handling messages" << std::endl;

#ifdef DO_END_SLEEP
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
#endif
}

int init_transports(
    std::shared_ptr<TangramTransport> &tx,
    std::shared_ptr<TangramTransport> &rx,
    bool addSubs)
{
    TangramTransport::resetTransportOptions();

    // Configure the transport
    tx = std::make_shared<TangramTransportZMQ>();
    rx = std::make_shared<TangramTransportZMQ>();
    if (tx == nullptr || rx == nullptr)
    {
        std::cerr << "Failed to create transport" << std::endl;
        exit(1);
    }

    std::string ip = "127.0.0.1";
    //std::string sub_port = "6667";
    //std::string pub_port = "6668";
    //ports need to match when testing through tangram
    std::string sub_port = "6668";
    std::string pub_port = "6667";

    char *maybe_value = std::getenv("TANGRAM_TRANSPORT_zeromq_transport_HOSTNAME");
    if (maybe_value != nullptr)
    {
        ip = maybe_value;
        std::cout << "Using env hostname" << std::endl;
    }

    maybe_value = std::getenv("TANGRAM_TRANSPORT_zeromq_transport_PORTS");
    if (maybe_value != nullptr)
    {
        std::string ports(maybe_value);
        std::cout << "Using env ports" << std::endl;

        // split the ports at the comma
        auto comma_pos = ports.find(",");
        if (comma_pos == std::string::npos)
        {
            std::cerr << "Unexpected lack of comma in PORTS env variable" << std::endl;
        }
        else
        {
            // Pub port, then sub port, because it should be reversed from the proxy
            // (Proxy in platform will sub on 6667, and pub on 6668)
            pub_port = ports.substr(0, comma_pos);
            sub_port = ports.substr(comma_pos + 1);
        }
    }

    std::cout << "Using IP " << ip << std::endl;
    std::cout << "Using Sub Port " << sub_port << std::endl;
    std::cout << "Using Pub Port " << pub_port << std::endl;

    rx->setOption("SubscribeIP", ip);
    rx->setOption("SubscribePort", sub_port);
    tx->setOption("PublishIP", ip);
    tx->setOption("PublishPort", pub_port);

    if (-1 == tx->open(TTF_WRITE))
    {
        std::cerr << "Failed to open tx transport" << std::endl;
        return 1;
    }
    std::cout << "Opened tx transport" << std::endl;

        // To run on tangram rx->open(TTF_READ))
    // To run on local rx->open(TTF_READ | TTF_BROKERLESS)
    if (-1 == rx->open(TTF_READ))
    {
        std::cerr << "Failed to open rx transport" << std::endl;
        return 1;
    }
    std::cout << "Opened rx transport" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (addSubs)
    {
        add_subscriptions(rx);
    }

    return 0;
}

bool sendMessage(Message &m, std::shared_ptr<TangramTransport> tport, DirectSerializer &ser)
{
    static std::vector<uint8_t> buffer;

    buffer.clear();

    if (!ser.serialize(m, buffer))
    {
        std::cerr << "Failed to serialize message " << m.getName() << std::endl;
        return false;
    }

    std::string topic = "messages." + m.getName();

    if (!tport->publish(buffer.data(), buffer.size(), topic))
    {
        std::cerr << "Failed to publish message " << m.getName() << std::endl;
        return false;
    }
    std::cout << "Sent message to: " << topic << std::endl;

    return true;
}

bool recvMessage(
    Message &msg,
    std::shared_ptr<TangramTransport> tport,
    DirectSerializer &ser)
{
    static std::vector<uint8_t> buffer;

    buffer.resize(tport->getMaxReceiveSize());
    int32_t count = tport->recv(buffer.data(), buffer.size());
    if (count < 0)
    {
        std::cerr << "Failed to receive bytes for " << msg.getName() << std::endl;
        return false;
    }
    buffer.resize(count);
    std::cout << "Received bytes for " << msg.getName() << std::endl;

    if (ser.deserialize(buffer, msg))
    {
        std::cout << "Deserialized " << msg.getName() << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Failed to deser " << msg.getName() << std::endl;
    }

    return false;
}

int add_subscriptions(std::shared_ptr<TangramTransport> &rx)
{
    rx->subscribe("messages.RequestNumber");
    rx->subscribe("messages.michaelToEthan");
    std::cout << "Subscribed to messages.RequestNumber" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return 0;
}

int handle_messages(
    std::shared_ptr<TangramTransport> &tx,
    std::shared_ptr<TangramTransport> &rx,
    DirectSerializer &ser)
{
    while (true)
    {
        hi::michaelToEthan ovrsr3;
        hi::ethanToMichael ovrsr4;

        try
        {
            if (!sendMessage(ovrsr4, tx, ser))
            {
                std::cerr << "sendMessage failed" << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception in sendMessage: " << e.what() << std::endl;
        }

        try
        {
            if (!recvMessage(ovrsr3, rx, ser))
            {
                std::cerr << "recvMessage failed" << std::endl;
                return 1;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception in recvMessage: " << e.what() << std::endl;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 0;
}
