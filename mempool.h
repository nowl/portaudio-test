#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

struct mempool;


struct mempool * mp_create(size_t blk_size, unsigned int num_blks);
void mp_destroy(struct mempool *mp);
void *mp_alloc(struct mempool *mp);
void mp_free(struct mempool *mp, void *v);

#endif  /* __MEMPOOL_H__ */
