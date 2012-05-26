#include <stddef.h>
#include <stdlib.h>

//#define MP_DEBUG
//#define MP_DEBUG_INFO

#ifdef MP_DEBUG
#include <stdio.h>
#endif

struct node {
    struct node *n, *p;
    void *v;
};

struct mempool {
    size_t       blk_size;
    unsigned int num_blks;

    void **buffer;
    size_t num_buffers;

    struct node *available_nodes;
    struct node *used_nodes;
};

static void add_to_allocated(struct mempool *mp, struct node *n)
{
    n->n = mp->available_nodes;
    if(n->n)
        n->n->p = n;
    n->p = NULL;
    mp->available_nodes = n;
}

static void set_up_nodes(struct mempool *mp, int buffer_num)
{
    int node_and_blk_size = sizeof(struct node) + mp->blk_size;

    int i;
    for(i=0; i<mp->num_blks; i++)
    {
        struct node *n = mp->buffer[buffer_num] + i*node_and_blk_size;
        n->v = mp->buffer[buffer_num] + i*node_and_blk_size + sizeof(struct node);

        add_to_allocated(mp, n);

#ifdef MP_DEBUG_INFO
        printf("i = %d in node setup\n", i);
        printf("cur = %p, next = %p, prev = %p, available_node_ptr = %p\n", n, n->n, n->p, mp->available_nodes);
#endif
    }
}

struct mempool *mp_create(size_t blk_size, unsigned int num_blks)
{
    struct mempool *mp = malloc(sizeof(*mp));
    mp->blk_size = blk_size;
    mp->num_blks = num_blks;
    mp->available_nodes = mp->used_nodes = NULL;

    int node_and_blk_size = sizeof(struct node) + blk_size;
    mp->num_buffers = 1;
    mp->buffer = malloc(sizeof(*mp->buffer));
    mp->buffer[0] = malloc(node_and_blk_size * num_blks);

#ifdef MP_DEBUG
    int total_size = node_and_blk_size * num_blks;
    printf("allocated %d bytes (%.3lf KB) (%.3lf MB)\n",
           total_size,
           (double)total_size/1024,
           (double)total_size/1024/1024);
#endif

    set_up_nodes(mp, 0);

    return mp;
}

void mp_destroy(struct mempool *mp)
{
    int i;
    for(i=0; i<mp->num_buffers; i++)
        free(mp->buffer[i]);
    free(mp->buffer);
    free(mp);
}

void *mp_alloc(struct mempool *mp)
{
    struct node *n = mp->available_nodes;

    /* grow mempool if necessary */
    if(!n)
    {
        mp->num_buffers++;
        mp->buffer = realloc(mp->buffer, sizeof(*mp->buffer) * mp->num_buffers);
        int node_and_blk_size = sizeof(struct node) + mp->blk_size;
        mp->buffer[mp->num_buffers-1] = malloc(node_and_blk_size * mp->num_blks);
        set_up_nodes(mp, mp->num_buffers-1);
        mp->num_blks *= 2;
        n = mp->available_nodes;
    }
    
    /* fix up allocated nodes */
    if(n->n)
        n->n->p = NULL;
    mp->available_nodes = n->n;

    /* add on to used nodes */
    n->n = mp->used_nodes;
    if(n->n)
        n->n->p = n;
    mp->used_nodes = n;
    
    return n->v;
}

void mp_free(struct mempool *mp, void *v)
{
    struct node *n = v - sizeof(struct node);

    if(mp->used_nodes == n)
    {
        /* this happens to be what used points to */
        mp->used_nodes = n->n;
        add_to_allocated(mp, n);
    }
    else
    {
        /* somewhere in the middle of used */
        if(n->p)
            n->p->n = n->n;
        if(n->n)
            n->n->p = n->p;
        add_to_allocated(mp,n);
    }
}

#ifdef MP_DEBUG

void dump_state(struct mempool *mp)
{
    printf("mempool state\n");
    printf("  block size = %ld\n", mp->blk_size);
    printf("  num blocks = %d\n", mp->num_blks);
    printf("  buffer loc = %p\n", mp->buffer);
    printf("  available:\n");
    struct node *n = mp->available_nodes;
    while(n)
    {
        printf("    %p (n=%p, p=%p, v=%p)\n", n, n->n, n->p, n->v);
        n=n->n;
    }
    printf("  used:\n");
    n = mp->used_nodes;
    while(n)
    {
        printf("    %p (n=%p, p=%p, v=%p)\n", n, n->n, n->p, n->v);
        n=n->n;
    }
}

void count_nodes(struct mempool *mp)
{
    int total_size = (sizeof(struct node) + mp->blk_size) * mp->num_blks;
    printf("allocated %d bytes (%.3lf KB) (%.3lf MB)\n",
           total_size,
           (double)total_size/1024,
           (double)total_size/1024/1024);
    
    printf("node count:\n");
    struct node *n = mp->available_nodes;

#ifdef MP_DEBUG_INFO
    printf("available_node_ptr = %p\n", n);
#endif

    int i = 0;
    while(n != NULL)
    {
        n = n->n;
#ifdef MP_DEBUG_INFO
        printf("n = %p\n", n);
#endif
        i++;
    }
    printf("  available nodes = %d\n", i);
    n = mp->used_nodes;
    i = 0;    
    while(n != NULL)
    {
        n = n->n;
        i++;
    }
    printf("  used nodes = %d\n", i);
}

struct test
{
    char buf[500];
    void *blah;
};

int main()
{
    struct mempool *mp = mp_create(sizeof(struct test), 1);
    dump_state(mp);
    //count_nodes(mp);

    printf("allocating an object\n");
    struct test *t = mp_alloc(mp);
    printf("t = %p\n", t);
    int i;
    for(i=0; i<500; i++)
        t->buf[i] = i;
    
    dump_state(mp);
    //count_nodes(mp);

    printf("allocating another object\n");
    struct test *s = mp_alloc(mp);
    printf("s = %p\n", s);
    for(i=0; i<500; i++)
        s->buf[i] = i+12;
    
    dump_state(mp);

    printf("freeing that first object\n");
    mp_free(mp, t);
    dump_state(mp);

    printf("freeing second object\n");
    mp_free(mp, s);
    dump_state(mp);

    printf("creating many objects\n");
    for(i=0; i<20000; i++)
        mp_alloc(mp);
    
    count_nodes(mp);
    //dump_state(mp);

    mp_destroy(mp);
    
    return 0;
}
#endif
