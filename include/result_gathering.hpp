#ifndef RESULT_GATHERING_H
#define RESULT_GATHERING_H

#include "ticket_map.h"
#include "delay_map.h"
#include <string>

TicketMap* group_tickets(const std::string& filename);
void generate_csv(DelayMap* delay_map, const std::string& sales_filename, const std::string& output_filename);

#endif // RESULT_GATHERING_H
