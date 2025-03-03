// log_client.cpp
#include "log_client.h"

Logger& get_logger() {
    static Logger instance;
    return instance;
}