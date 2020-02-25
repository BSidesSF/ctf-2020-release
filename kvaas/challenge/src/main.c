#include "prompt.h"
#include "nodes.h"
#include <string.h>
#include <stdlib.h>
#include "ctf.h"

typedef struct {
  char *cmd;
  char *help;
  void (*cmd_func)();
} cmd_entry;

void cmd_help();
void cmd_exit();
void cmd_add();
void cmd_edit();
void cmd_list();
void cmd_delete();
void cmd_copy();

cmd_entry commands[] = {
  {"add", "Add a key/value pair", &cmd_add},
  {"list", "List the key/values", &cmd_list},
  {"edit", "Edit value", &cmd_edit},
  {"delete", "Delete a key/value", &cmd_delete},
  {"copy", "Copy a key/value", &cmd_copy},
  {"help", "Help on Commands", &cmd_help},
  {"exit", "Exit", &cmd_exit},
  {NULL, NULL, NULL},
};

int main(int argc, char **argv) {
  SETUP_CTF();
#ifdef DEBUG
  if (sizeof(node_uint_t) != sizeof(node_int_t) ||
      sizeof(node_uint_t) != sizeof(node_str_t) ||
      sizeof(node_uint_t) != sizeof(node_time_t) ||
      sizeof(node_uint_t) != sizeof(node_secret_t) ||
      sizeof(secret_data_t) != sizeof(node_secret_t)) {
    printf("U: %#02lx I: %#02lx S: %#02lx T: %#02lx S: %#02lx SD: %#02lx\n",
        sizeof(node_uint_t),
        sizeof(node_int_t),
        sizeof(node_str_t),
        sizeof(node_time_t),
        sizeof(node_secret_t),
        sizeof(secret_data_t));
  }
  printf("sizeof(node_type) == %lu\n", sizeof(node_type));
#endif
  while(1) {
    char *cmd = prompt_user("command");
    if (!strcmp(cmd, ""))
      continue;
    int found = 0;
    if (!cmd)
      return 0;
    cmd_entry *p = commands;
    while(p->cmd) {
      if(!strcmp(p->cmd, cmd)) {
        p->cmd_func();
        found = 1;
        break;
      }
      p++;
    }
    if (!found) {
      printf("Command %s not found!\n", cmd);
    }
  }
}

void cmd_add() {
  char *name = prompt_user_key(NULL);
  // Which type
  int i;
  printf("Node type?\n");
  for (i = 0; i < NODE_MAX; i++) {
    printf("(%4s) %s\n", node_type_names[i], node_type_descs[i]);
  }
  node_type found = NODE_MAX;
  while (found == NODE_MAX) {
    char *buf = prompt_user("type");
    for (i = 0; i < NODE_MAX; i++) {
      if (!strcmp(buf, node_type_names[i])) {
        found = i;
        break;
      }
    }
  }
  node_common_t *n = add_node(found);
  if (!n) {
    printf("Error making new node!\n");
    return;
  }
  set_node_name(n, name);
  if (n->set_func)
    n->set_func(n);
}

void cmd_edit() {
  char *buf = prompt_user_key(NULL);
  node_common_t *n = get_node_by_name(buf);
  if (!n) {
    printf("node not found!\n");
    return;
  }
  if (n->set_func)
    n->set_func(n);
}

void cmd_copy() {
  char *buf = prompt_user_key("source key");
  node_common_t *n = get_node_by_name(buf);
  if (!n) {
    printf("node not found!\n");
    return;
  }
  buf = prompt_user_key("new key");
  node_common_t *new_node = malloc(node_size[n->type]);
  memcpy(new_node, n, node_size[n->type]);
  set_node_name(new_node, buf);
  insert_node(new_node);
}

void cmd_list() {
  node_common_t *p = get_node_head();
  for (;p;p = p->next) {
    printf("%32s: ", p->name);
    if (p->print_func)
      p->print_func(p);
    printf("\n");
  }
}

void cmd_delete() {
  char *name = prompt_user_key(NULL);
  node_common_t *n = get_node_by_name(name);
  if (!n) {
    printf("Node not found!\n");
    return;
  }
  delete_node(n);
}

void cmd_help() {
  cmd_entry *p = commands;
  while(p->cmd) {
    printf("%-12s %s\n", p->cmd, p->help);
    p++;
  }
  printf("\n");
}

void cmd_exit() {
  exit(0);
}
