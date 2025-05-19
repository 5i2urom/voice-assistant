#pragma once
#include <stdbool.h>

void start_network_monitor();
void trigger_ping_now();
bool is_server_reachable();
void start_ping_timer();
void stop_ping_timer();