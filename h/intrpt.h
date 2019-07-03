#ifndef HEADERS_INTRPT_H_
#define HEADERS_INTRPT_H_


typedef void interrupt (*intH)(...);

void register_handler(unsigned char id, intH handler);
void restore_old(unsigned char id);
void setup_timer(void (*target)());
char already_set(unsigned char id);
void call_old(unsigned char id);


#endif /* HEADERS_INTRPT_H_ */
