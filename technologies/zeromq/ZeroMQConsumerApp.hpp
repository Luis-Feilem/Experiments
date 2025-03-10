#ifndef ZEROMQ_CONSUMER_APP_HPP
#define ZEROMQ_CONSUMER_APP_HPP

#include "../../core/interfaces/IConsumerApp.hpp"
#include "ZeroMQConsumer.hpp"

class ZeroMQConsumerApp : public IConsumerApp {
private:
    ZeroMQConsumer consumer;

};

#endif // ZEROMQ_CONSUMER_APP_HPP
