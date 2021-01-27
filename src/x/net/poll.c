#define _GNU_SOURCE

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>

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
        o->internal = xfree(o->internal);
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
        descriptor->parent = (xdescriptorlist *) o;

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

static const xuint64 __page = 64; // TODO:

static xuint32 __xpoll_convert_pollevents(xuint32 mask)
{
    xuint32 result = 0;
    if(mask & xdescriptor_event_read)
    {
        result |= POLLIN;
    }
    if(mask & xdescriptor_event_write)
    {
        result |= POLLOUT;
    }
    if(mask & xdescriptor_event_error)
    {
        result |= POLLERR;
    }
    if(mask & xdescriptor_event_pri)
    {
        result |= POLLPRI;
    }
    if(mask & xdescriptor_event_readhup)
    {
        result |= POLLRDHUP;
    }
    if(mask & xdescriptor_event_hup)
    {
        result |= POLLHUP;
    }
    if(mask & xdescriptor_event_invalid)
    {
        result |= POLLNVAL;
    }
    if(mask & xdescriptor_event_readband)
    {
        result |= POLLRDBAND;
    }
    if(mask & xdescriptor_event_writeband)
    {
        result |= POLLRDBAND;
    }
    if(mask & xdescriptor_event_writeband)
    {
        result |= POLLRDBAND;
    }
    return result;
}

extern void xpollwait(xpoll * o)
{
    xcheck(o == xnil, "null pointer");
    if(o)
    {
        xsynclock(o->sync);
        if(o->internalsize < o->descriptors)
        {
            o->internalsize = (o->descriptors / __page + 1) * __page;
            if(o->internal)
            {
                o->internal = realloc(o->internal, o->internalsize * sizeof(struct pollfd));
            }
            else
            {
                o->internal = malloc(o->internalsize * sizeof(struct pollfd));
            }
        }
        else if(o->internal == xnil)
        {
            o->internalsize = (o->descriptors / __page + 1) * __page;
            o->internal = malloc(o->internalsize * sizeof(struct pollfd));
        }
        xdescriptor * node = o->head;
        struct pollfd * fds = (struct pollfd *) o->internal;
        for(xuint64 i = 0; i < o->descriptors && node; i++)
        {
            xcheck(node->value.f < 0, "check this");
            fds[i].fd = node->value.f;
            fds[i].events = __xpoll_convert_pollevents(node->interest);
            fds[i].events |= (POLLERR | POLLHUP | POLLNVAL | POLLPRI | POLLRDBAND | POLLRDHUP | POLLWRBAND);
            fds[i].revents = 0;
            node = node->next;
        }
        // 부하기 심하다. 그렇기 때문에, 노드를 하나씩 지날 때마다 LOCK & UNLOCK
        // 을 호출하는 구조로 가지고 가고 싶다.
        // 고민스럽다. 완벽하게 스레드 세이프티 해야 하는데, ...
        xsyncunlock(o->sync);
    }
}