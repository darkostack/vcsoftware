#ifndef SERIAL_LINE_H_
#define SERIAL_LINE_H_

#include "contiki.h"

extern process_event_t serial_line_event_message;

int serial_line_input_byte(unsigned char c);

void serial_line_init(void);

PROCESS_NAME(serial_line_process);

#endif /* SERIAL_LINE_H_ */
