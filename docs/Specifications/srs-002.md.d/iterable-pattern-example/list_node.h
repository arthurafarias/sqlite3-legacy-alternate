typedef struct list_node list_node;
struct list_node {
  void *data;
  list_node *prev;
  list_node *next;
};

list_node *list_node_alloc();
void list_node_dealloc(list_node *);
void list_node_construct(list_node *);
void list_node_destruct(list_node *);