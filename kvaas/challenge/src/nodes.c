#define _POSIX_C_SOURCE 200809L
#include "nodes.h"
#include "prompt.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void print_node_int(node_common_t *n);
void print_node_str(node_str_t *n);
void print_node_time(node_time_t *n);
void print_node_secret(node_secret_t *n);
void set_node_uint(node_uint_t *n);
void set_node_int(node_int_t *n);
void set_node_str(node_str_t *n);
void set_node_time(node_time_t *n);
void set_node_secret(node_secret_t *n);
size_t encrypt_secret_data(char *key, char *value);
void decrypt_secret_data(char *key, char *value, size_t pt_len);

char *node_type_names[NODE_MAX] = {
  "uint",
  "int",
  "str",
  "time",
  "sec",
};

char *node_type_descs[NODE_MAX] = {
  "Unsigned integer",
  "Signed integer",
  "String",
  "Timestamp",
  "Secret Value",
};

size_t node_size[NODE_MAX] = {
  sizeof(node_uint_t),
  sizeof(node_int_t),
  sizeof(node_str_t),
  sizeof(node_time_t),
  sizeof(node_secret_t),
};

node_common_t *node_head;

node_common_t *get_node_head() {
  return node_head;
}

node_common_t *alloc_node(node_type t) {
  node_common_t *n;
  switch(t) {
    case NODE_UINT:
      n = malloc(sizeof(node_uint_t));
      n->print_func = &print_node_int;
      n->set_func = (node_process_func_t)&set_node_uint;
      break;
    case NODE_INT:
      n = malloc(sizeof(node_int_t));
      n->print_func = &print_node_int;
      n->set_func = (node_process_func_t)&set_node_int;
      break;
    case NODE_STR:
      n = malloc(sizeof(node_str_t));
      n->print_func = (node_process_func_t)&print_node_str;
      n->set_func = (node_process_func_t)&set_node_str;
      break;
    case NODE_TIME:
      n = malloc(sizeof(node_time_t));
      n->print_func = (node_process_func_t)&print_node_time;
      n->set_func = (node_process_func_t)&set_node_time;
      ((node_time_t*)n)->time_func = &gmtime;
      break;
    case NODE_SECRET:
      n = malloc(sizeof(node_secret_t));
      n->print_func = (node_process_func_t)&print_node_secret;
      n->set_func = (node_process_func_t)&set_node_secret;
      node_secret_t *sec = (node_secret_t *)n;
      sec->secret = malloc(sizeof(secret_data_t));
      NODE_DEBUG("Allocated secret data at %p\n", sec->secret);
      sec->secret->encrypt_data = &encrypt_secret_data;
      sec->secret->decrypt_data = &decrypt_secret_data;
      break;
    default:
      printf("Allocating unknown node type!!!");
      return NULL;
  }
  NODE_DEBUG("Allocated node at %p\n", n);
  n->type = t;
  return n;
}

node_common_t *add_node(node_type t) {
  node_common_t *new_node = alloc_node(t);
  if (!new_node) {
    NODE_DEBUG("No node in add_node!!\n");
    return NULL;
  }
  new_node->next = node_head;
  node_head = new_node;
  return new_node;
}

void insert_node(node_common_t *n) {
  n->next = node_head;
  node_head = n;
}

void free_node(node_common_t *n) {
  node_str_t *nstr;
  node_secret_t *nsec;
  NODE_DEBUG("Freeing node at %p\n", n);
  if (n->next)
    NODE_DEBUG("!!!Leaving dangling next: %p\n", n->next);
  switch (n->type) {
    case NODE_STR:
      nstr = (node_str_t *)n;
      NODE_DEBUG("Freeing string pointer: %p\n", nstr->val);
      free(nstr->val);
      break;
    case NODE_SECRET:
      nsec = (node_secret_t *)n;
      NODE_DEBUG("Freeing secret storage: %p, %p\n",
          nsec->secret->data, nsec->secret);
      free(nsec->secret->data);
      free(nsec->secret);
      break;
    default:
      break;
  }
  free(n);
}

void delete_node(node_common_t *n) {
  if (!n) {
    NODE_DEBUG("Deleting NULL node!\n");
    return;
  }
  // Special case
  if (node_head == n) {
    node_head = n->next;
    n->next = NULL;
    free_node(n);
    return;
  }
  node_common_t *p = node_head;
  while(p) {
    if (p->next == n) {
      p->next = p->next->next;
      n->next = NULL;
      free_node(n);
      return;
    }
    p = p->next;
  }
}

node_common_t *get_node_by_name(const char *name) {
  node_common_t *p = node_head;
  while(p) {
    if (!strncmp(name, p->name, NODE_NAME_SZ)) {
      return p;
    }
    p = p->next;
  }
  return NULL;
}

void set_node_name(node_common_t *n, const char *name) {
  strncpy(n->name, name, NODE_NAME_SZ);
  n->name[NODE_NAME_SZ] = '\0';
}

void print_node_int(node_common_t *n) {
  if (n->type == NODE_UINT) {
    printf("%lu", ((node_uint_t *)n)->val);
  } else if (n->type == NODE_INT) {
    printf("%ld", ((node_int_t *)n)->val);
  }
}

void print_node_str(node_str_t *n) {
  printf("%s", n->val);
}

void print_node_time(node_time_t *n) {
  char buf[64];
  struct tm *t = n->time_func(&(n->when));
  strftime(buf, sizeof(buf), "%Y%m%d %H%M%S", t);
  printf("%s", buf);
}

void set_node_uint(node_uint_t *n) {
  unsigned long val;
  char *buf = prompt_user("Num?");
  char *end;
  val = strtoul(buf, &end, 0);
  if (buf[0] == '-' ||
      (val == ULONG_MAX && errno == ERANGE) ||
      (*end != '\0')) {
    printf("Error: out of range!\n");
    return;
  }
  printf("%s %s %lu\n", buf, end, val);
  n->val = val;
}

void set_node_int(node_int_t *n) {
  long val;
  char *buf = prompt_user("Num?");
  char *end;
  val = strtol(buf, &end, 0);
  if ((val == LONG_MAX && errno == ERANGE) ||
      (*end != '\0')) {
    printf("Error: out of range!");
    return;
  }
  n->val = val;
}

void set_node_str(node_str_t *n) {
  char *buf = prompt_user("value");
  if (!buf)
    return;
  n->val = strdup(buf);
  NODE_DEBUG("Allocated node string: %p\n", n->val);
}

void set_node_time(node_time_t *n) {
  n->when = time(NULL);
}

void print_node_secret(node_secret_t *n) {
  printf("[REDACTED]");
}

void set_node_secret(node_secret_t *n) {
  char *buf = prompt_user("key");
  if (!buf)
    return;
  strncpy(n->secret->key, buf, sizeof(n->secret->key));
  n->secret->key[sizeof(n->secret->key)] = '\0';
  buf = prompt_user("value");
  if (!buf)
    return;
  n->secret->data = strdup(buf);
  n->secret->pt_len = n->secret->encrypt_data(n->secret->key, n->secret->data);
}

size_t encrypt_secret_data(char *key, char *value) {
  size_t len = strlen(value);
  size_t key_len = strlen(key);
  for (size_t i=0; i<len; i++){
    value[i] ^= key[i % key_len];
  }
  return len;
}

void decrypt_secret_data(char *key, char *value, size_t pt_len) {
  size_t key_len = strlen(key);
  for (size_t i=0; i<pt_len; i++){
    value[i] ^= key[i % key_len];
  }
}
