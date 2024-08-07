#ifndef RESULT_GATHERING_H
#define RESULT_GATHERING_H

#include "ticket_map.h"
#include "delay_map.h"

char* copy_string(char* s);
TicketMap* group_tickets(const char* filename);
void generate_csv(DelayMap* delay_map, const char* sales_filename, const char* output_filename);

#endif // RESULT_GATHERING_H
