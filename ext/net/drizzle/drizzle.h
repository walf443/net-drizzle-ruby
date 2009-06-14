#include <libdrizzle/drizzle_client.h>

typedef struct {
    drizzle_st* drizzle;
} net_drizzle_st;

typedef struct {
    drizzle_con_st* con;
} net_drizzle_con_st;

typedef struct {
    drizzle_query_st* query;
} net_drizzle_query_st;
