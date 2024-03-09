

#define offset_of(type, member) ((size_t)(&((type *)0)->member))

#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offset_of(type, member)); })

struct list_head {
    struct list_head *next, *prev;
};

static inline void list_init_head(struct list_head *list) 
{
    list->next = list;
    list->prev = list;
}

static inline void list_add(struct list_head *node, struct list_head* head)
{
    struct list_head* next = head->next;
    
    head->next = node;
    next->prev = node;
    node->prev = head;
    node->next = next;
}

static inline void list_add_tail(struct list_head *node, struct list_head* head)
{
    struct list_head* prev = head->prev;

    head->prev = node;
    prev->next = node;
    node->next = head;
    node->prev = prev;
}

static inline void list_del(struct list_head *node)
{
    struct list_head* prev = node->prev;
    struct list_head* next = node->next;

    prev->next = next;
    next->prev = prev;
}

static inline void list_del_init(struct list_head *node)
{
    list_del(node);
    list_init_head(node);
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline int list_is_last(const struct list_head *node, const struct list_head *head)
{
    return node->next == head;
}

#define list_entry(node, type, member) container_of(node, type, member)

#define list_for_each(iter, head) \
    for (iter = (head)->next; iter != (head); iter = iter->next)

#define list_for_each_entry(iter, head, member) \
    for(iter = list_entry((head)->next, typeof(*iter), member); \
        &iter->member != (head); \
        iter = list_entry(iter->member.next, typeof(*iter), member))