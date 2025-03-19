#include <fs/inode.h>
#include <list.h>
#include <mm.h>

static list_t inode_list;

inode_t *ialloc(uint64 id) {
    // find inode by id
    for (list_t *p = &inode_list; p != NULL; p = p->next) {
        inode_t *ip = (inode_t *)p->node;
        if (ip->id == id) {
            ip->refcnt++;
            return ip;
        }
    }

    // allocate new inode
    inode_t *ip = (inode_t *)kmalloc(sizeof(inode_t));
    ip->id = id;
    ip->size = 0;
    ip->refcnt = 1;
    list_push(&inode_list, ip);
    return ip;
}

void ifree(inode_t *ip) {
    if (ip->refcnt > 2) {
        ip->refcnt--;
        return;
    }
    list_remove(&inode_list, ip);
    kmfree(ip);
}