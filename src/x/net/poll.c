#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "../net.h"

extern xpoll * xpollnew(void)
{
    xpoll * o = (xpoll *) calloc(sizeof(xpoll), 1);
    xassertion(o == xnil, "fail to calloc (%d)", errno);

    o->flags = (xobj_mask_allocated | xobj_type_poll);
    o->destruct = xpollrem;

    return o;
}

extern void * xpollrem(void * p)
{
    xpoll * o = (xpoll *) p;
    xcheck(o == xnil, "null pointer");

    if(o)
    {
        xassertion(xobjtype(o) != xobj_type_poll, "invalid object");
        xsynclock(o->sync);

        while(o->head)
        {
            xdescriptor * node = o->head;
            if(node->next)
            {
                node->next->prev = xnil;
            }
            else
            {
                o->tail = xnil;
            }
            node->prev = xnil;
            node->next = xnil;
            node->parent = xnil;
            o->head = node->next;
            o->descriptors = o->descriptors - 1;
        }
        xsyncunlock(o->sync);
        // CHECKOUT ASSERTION LINKED LIST

        if(xobjallocated(o))
        {
            free(o);
            o = xnil;
        }
    }

    return o;
}

extern void xpolladd(xpoll * o, xdescriptor * descriptor)
{
    xcheck(o == xnil && descriptor == xnil, "null pointer");

    if(o && descriptor)
    {
        xassertion(descriptor->parent != xnil, "parent is already exist");
        xassertion(descriptor->prev || descriptor->next, "critical => ...");

        xsynclock(o->sync);
        descriptor->parent = o;

        descriptor->prev = o->tail;
        o->tail = descriptor;

        if(descriptor->prev)
        {
            descriptor->prev->next = descriptor;
        }
        else
        {
            o->head = descriptor;
        }
        o->descriptors = o->descriptors + 1;

        xsyncunlock(o->sync);
    }
}

extern void xpolldel(xpoll * o, xdescriptor * descriptor)
{
    xcheck(o == xnil || descriptor==xnil, "null pointer");

    if(o && descriptor)
    {
        /**
         * 고민스러운 부분이다. o 가 필요 없어 보이는데,
         * 지워야 하나 ...
         */
        xassertion(descriptor->parent != 0, "invalid parent");
        xsynclock(o->sync);
        xdescriptor * prev = descriptor->prev;
        xdescriptor * next = descriptor->next;
        if(prev)
        {
            prev->next = next;
        }
        else
        {
            o->head = next;
        }
        if(next)
        {
            next->prev = prev;
        }
        else
        {
            o->tail = prev;
        }
        xuint32 descriptors = o->descriptors = o->descriptors - 1;
        xsyncunlock(o->sync);
    }
}

extern void xpollwait(xpoll * o)
{
    // TODO: IMPLEMENT THIS
}