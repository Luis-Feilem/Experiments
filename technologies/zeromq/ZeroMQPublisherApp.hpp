#ifndef ZEROMQ_PUBLISHER_APP_HPP
#define ZEROMQ_PUBLISHER_APP_HPP

#include "../../core/interfaces/IPublisherApp.hpp"
#include "ZeroMQPublisher.hpp"

class ZeroMQPublisherApp : public IPublisherApp {
private:
    ZeroMQPublisher publisher;

};

#endif // ZEROMQ_PUBLISHER_APP_HPP
