#ifndef __CMD_H__
#define __CMD_H__

#include <string.h>
#define CMD_MAX 16

typedef struct {
	char *str;
	void (*handler)(char*);
} cmd_t;

struct _cmd_set_t {
	cmd_t *set;
	int len;
	struct _cmd_set_t *next;
};

typedef struct _cmd_set_t cmd_set_t;

#define TOSET(x) __##x##_set
extern void __cmd_register_set(cmd_set_t *set);

/* public API follows */
#define CMD_DECLARE_LIST(x) \
	static cmd_set_t TOSET(x); \
	cmd_t x[]

#define CMD_REGISTER_LIST(x) \
	__cmd_register_set(&TOSET(x));\
	TOSET(x).set = x; \
	TOSET(x).len = sizeof(x) / sizeof(cmd_t);

extern void cmd_init(void);
extern void cmd_poll(void);
extern void cmd_send(char *buf);
#endif
