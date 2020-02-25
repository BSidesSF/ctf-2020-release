#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#ifdef DEBUG
#define NODE_DEBUG(...) do{fprintf(stderr, __VA_ARGS__);}while(0)
#else
#define NODE_DEBUG(...) do{} while(0)
#endif

#define NODE_NAME_SZ 31

typedef enum {
  NODE_UINT,
  NODE_INT,
  NODE_STR,
  NODE_TIME,
  NODE_SECRET,
  // Must be last!
  NODE_MAX,
} node_type;

char *node_type_names[NODE_MAX];
char *node_type_descs[NODE_MAX];
size_t node_size[NODE_MAX];

struct _node_common_t;
struct _secret_data_t;

typedef void (*node_process_func_t)(struct _node_common_t*);

typedef struct _node_common_t {
  node_type type;
  char name[NODE_NAME_SZ+1];
  node_process_func_t print_func;
  node_process_func_t set_func;
  struct _node_common_t *next;
} node_common_t;

typedef struct {
  node_common_t;
  uint64_t val;
  char _[8];
} node_uint_t;

typedef struct {
  node_common_t;
  int64_t val;
  char _[8];
} node_int_t;

typedef struct {
  node_common_t;
  char *val;
  char _[8];
} node_str_t;

typedef struct {
  node_common_t;
  struct tm *(*time_func)(const time_t*);
  time_t when;
} node_time_t;

typedef struct {
  node_common_t;
  struct _secret_data_t *secret;
  void *_;
} node_secret_t;

typedef struct _secret_data_t {
  char secret_header[4];
  char key[32];
  // pointers padding
  char _[8];
  size_t pt_len;
  void (*decrypt_data)(char *, char *, size_t);
  size_t (*encrypt_data)(char *, char *);
  char *data;
} secret_data_t;

/** Function prototypes */
node_common_t *get_node_head();
node_common_t *alloc_node(node_type t);
node_common_t *add_node(node_type t);
void insert_node(node_common_t *n);
void delete_node(node_common_t *n);
void free_node(node_common_t *n);
node_common_t *get_node_by_name(const char *name);
void set_node_name(node_common_t *n, const char *name);
