__NOVEMBERIZING/NET__
=====================

"novemberizing/net" 은 작고 가벼운 네트워크 라이브러리입니다.
이 라이브러리는 "c" 로 작성되었습니다.

## 목적

"novemberizing/net" 은 멀티 플랫폼을 지원할 것입니다.
여러 플랫폼에서 쉽게 클라이언트/서버를 작성이 가능하도록 할 것이며,
최선의 성능을 보장하도록 할 것입니다.

> 현재 지원은 리눅스만 지원하고 있습니다.

## 현재 상태

개발 중에 있는 버전으로 아직 서비스 가능한 버전을 내놓지는 않고 있습니다.

## 컴파일

```
$ ./configure
$ make
$ sudo make install
```

## 예제

### 클라이언트

`ncat` 을 이용하여 간단하게 서버를 실행시킨 후에 클라이언트가 서버로 메시지를 보내고 받는 예제입니다.
블로킹 소켓 형태로 메시지를 받을 때까지 RECV & CONNECT 는 무한히 기다리게 됩니다.

```c
/**
 * simple echo server
 * ncat -l 2000 -k -c 'xargs -n1 echo'
 */
xclient * client = xclientnew(AF_INET, SOCK_STREAM, IPPROTO_TCP);
struct sockaddr_in addr;
addr.sin_family = PF_INET;
addr.sin_port = htons(2000);
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
socklen_t addrlen = sizeof(struct sockaddr_in);
xclientconnect(client, xaddressof(addr), addrlen);
xclientsend(client, "hello\n", 6);
char buffer[8];
int ret = xclientrecv(client, buffer, 5);
buffer[5] = 0;
printf("[recv:%d] %s\n", ret, buffer);
xclientrem(client);
```

만약에, 논블록 읽기를 수행하고 싶으면, 접속 후에 `xclient_nonblock_on(...)` 함수를 호출하면 됩니다.
논블록 설정 후에 쓰기가 정상적으로 동작하는 것인 아직 소켓 버퍼가 충분히 남아 있기 때문입니다.

```c
buffer[0] = 0;
client = xclientnew(AF_INET, SOCK_STREAM, IPPROTO_TCP);
addr.sin_family = PF_INET;
addr.sin_port = htons(2000);
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
addrlen = sizeof(struct sockaddr_in);
xclientconnect(client, xaddressof(addr), addrlen);
xclient_nonblock_on(client);
xclientsend(client, "hello\n", 6);
ret = xclientrecv(client, buffer, 5);
buffer[5] = 0;
printf("[recv:%d] %s\n", ret, buffer);
xclientrem(client);
```

만약에, 접속 역시 논블록으로 수행하고 싶으면 접속 함수의 호출 전에 논블록으로 수행하는 객체임을 설정하면 됩니다.

```c
buffer[0] = 0;
client = xclientnew(AF_INET, SOCK_STREAM, IPPROTO_TCP);
xclientmaskadd(client, xsocket_mask_nonblock);
addr.sin_family = PF_INET;
addr.sin_port = htons(2000);
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
addrlen = sizeof(struct sockaddr_in);
ret = xclientconnect(client, xaddressof(addr), addrlen);
xcheck(ret != xsuccess, "working nonblock connect");
printf("connect => %d\n", ret);
xclient_nonblock_on(client);
// NONBLOCLING CONNECT 상태에서 WRITE 가 가능하다.
ret = xclientsend(client, "hello\n", 6);
xclient_nonblock_off(client);
printf("send => %d\n", ret);
ret = xclientrecv(client, buffer, 5);
buffer[5] = 0;
printf("[recv:%d] %s\n", ret, buffer);
xclientrem(client);
```

만약에 접속 혹은 읽기 가능한 이벤트를 기다리고 싶다면, 아래 처럼 WAIT 함수를 호출하면 됩니다.

```c
buffer[0] = 0;
client = xclientnew(AF_INET, SOCK_STREAM, IPPROTO_TCP);
xclientmaskadd(client, xsocket_mask_nonblock);
addr.sin_family = PF_INET;
addr.sin_port = htons(2000);
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
addrlen = sizeof(struct sockaddr_in);
ret = xclientconnect(client, xaddressof(addr), addrlen);
xcheck(ret != xsuccess, "working nonblock connect");
printf("connect => %d\n", ret);

if(client->status & xclient_status_connecting)
{
xuint32 status = xclientwait(client, xclient_event_connect, 1000000000);
xcheck((status & xclient_event_connect) != xclient_event_connect, "not connect (%08x)", status);
if((status & xclient_event_connect) != xclient_event_connect)
{
    xclientclose(client);
}
}

// xclient_nonblock_on(client);
// NONBLOCLING CONNECT 상태에서 WRITE 가 가능하다.
ret = xclientsend(client, "hello\n", 6);
// xclient_nonblock_off(client);
printf("send => %d\n", ret);
xuint32 status = xclientwait(client, xclient_event_read, 1000000000);
printf("status (0x%08x)\n", status);
ret = xclientrecv(client, buffer, 5);
if(ret > 0)
{
buffer[5] = 0;
printf("[recv:%d] %s\n", ret, buffer);
}
xclientrem(client);
```
