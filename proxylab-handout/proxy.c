#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 4
#define SBUFSIZE 16

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";

struct req{
    char* domain;
    char* path;
    char* method;
    char* hdrs;
};
typedef struct req req;		// struct for request

struct cache{
	char* name;
	char* domain;
	unsigned LRUtag;
	unsigned size;
	char* data;
	struct cache *next;
	struct cache *prev;
};
typedef struct cache cache;		//struct for cache

//thread function
void *thread_func(void *vargp);
//request parsing function
int parseLine(int connfd, req* request);
int parseUri(char *uri, char *filename, char *cgiargs);
int parseRequest(char *buf, req *request);
char *parseHdr(char *buf);
//send request to end server
void sendRequest(int clientfd, req* request);
//cache functions
void initCache();
void addCache(char* data, char* name, char* domain);
cache* isHit(req* request);
void evictCache(char* data, char* name, char* domain);

size_t cacheSize;
cache* headcache;
cache* tailcache;
unsigned cacheuse;
sem_t cache_sem;

int main(int argc, char **argv)
{
	int listenfd, connfd;
	int i;
	socklen_t clientLen;
	struct sockaddr_storage clientAddr;
	pthread_t tid;
	sbuf_t sbuf;

	if(argc != 2){
		fprintf(stderr, "usage : %s <port>\n", argv[0]);
		exit(1);
	}
	listenfd = Open_listenfd(argv[1]);

	Sem_init(&cache_sem, 0, 1);
	initCache();
	sbuf_init(&sbuf, SBUFSIZE);

	for(i = 0; i < NTHREADS; i++){
		Pthread_create(&tid, NULL, thread_func, &sbuf);
	}

	while(1){
		clientLen = sizeof(clientAddr);
		connfd = Accept(listenfd, (SA *)&clientAddr, &clientLen);
		sbuf_insert(&sbuf, connfd);
	}
    return 0;
}

void *thread_func(void *vargp)
{
	req request;
	Pthread_detach(Pthread_self());
	while(1){
		int connfd = sbuf_remove(vargp);
		if(parseLine(connfd, &request) == -1){
			Close(connfd);
			continue;
		}
		sendRequest(connfd, &request);
	}
}
int parseLine(int connfd, req* request)
{
    rio_t rio;
    int n;
    char buf[MAXLINE];

    request->domain = NULL;
    request->path = NULL;
    request->hdrs = NULL;
    request->method = NULL;

    /* Read request line and headers */
    Rio_readinitb(&rio, connfd);
    if (Rio_readlineb(&rio, buf, MAXLINE)){
    	if(parseRequest(buf, request) == -1)
    		return -1;
    }else {
    	return -1;
    }

     while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){   
        if(strcmp(buf, "\r\n") == 0)	// header end
            break;
        if(request->hdrs != NULL){
            n = strlen(request->hdrs) + strlen(buf) + 1;
            request->hdrs = Realloc(request->hdrs,strlen(request->hdrs)+ n);
            strcat(request->hdrs, parseHdr(buf));
        } else {
            request->hdrs = Malloc(n+1);
            strcpy(request->hdrs, parseHdr(buf));
        }
    }//다시 한번 보고 수정 요망 
    return 0;
}
int parseRequest(char *buf, req *request)
{
	char *token = NULL;
	char *save;

	token = strtok_r(buf, " ", &save);		//GET

	if (strcasecmp(token, "GET")) {
        return -1;
    }
    if(token)
    	request->method = Malloc(strlen(token)+1);
    else
    	return -1;
    strcpy(request->method, token);

    token = strtok_r(NULL, "//", &save);		//http:
    token = strtok_r(NULL, "/", &save);		//domain
    if(token)
    	request->domain = Malloc(strlen(token)+1);
    else
    	return -1;
    strcpy(request->domain, token);
    token = strtok_r(NULL, " ", &save);		//uri
    if(token)
    	request->path = Malloc(strlen(token)+1);
    else
    	return -1;
    strcpy(request->path, token);

   return 0;
}
char *parseHdr(char* buf)
{
    char* cp, * head;
    size_t size;
    size = strlen(buf);
                                           
    cp = (char*) Malloc(size+1);
    head = (char*) Malloc(100);
    strcpy(cp, buf);

    strcpy(head, strtok(buf, ":"));
    if(strcmp(head, "User-Agent") == 0)
        strcpy(cp, user_agent_hdr); 
    if(strcmp(head, "Connection") == 0) 
        strcpy(cp, connection_hdr);
    if(strcmp(head, "Proxy-Connection") == 0) 
        strcpy(cp, proxy_connection_hdr);

    strcpy(buf, cp);
    Free(head);
    Free(cp);
    return buf;
}
void sendRequest(int clientfd, req* request)
{
	int serverfd;
	char *hostname, *port;
	char requestLine[MAXLINE];
	char buf[MAXLINE];
	char cachebuf[MAX_OBJECT_SIZE];
	int newCacheSize;
	cache* temp;
	size_t bytes;
	rio_t rio;

	cachebuf[0] = '\0';
	hostname = strtok(request->domain, ":");
	port = strtok(NULL, ":");

	if(port == NULL)
		port = "80";

	if((temp = isHit(request)) != NULL){
		Rio_writen(clientfd, temp->data, strlen(temp->data));
	}else{
		serverfd = Open_clientfd(hostname, port);
		if(serverfd == -1){
			return;
		}else{
			sprintf(requestLine, "GET /%s HTTP/1.0\r\n", request->path);
    	   	strcat(requestLine, request->hdrs);
    	   	Rio_writen(serverfd, requestLine, strlen(requestLine));
    	   	Rio_writen(serverfd, "\r\n", 2);
		}
	}
	

	Rio_readinitb(&rio, serverfd);
	newCacheSize = 0;
	while((bytes = Rio_readlineb(&rio, buf, MAXLINE)) > 0){
		if(newCacheSize + bytes <= MAX_OBJECT_SIZE){
            strcat(cachebuf, buf);
        }
        newCacheSize += bytes;
		Rio_writen(clientfd, buf, bytes);
	}
	if(newCacheSize < MAX_OBJECT_SIZE){
		addCache(cachebuf, request->path, request->domain);
	}
	return;
}
void initCache()
{
	headcache = Malloc(sizeof(cache));

	headcache->name = NULL;
	headcache->LRUtag = 1;
	headcache->size = 0;
	headcache->data = NULL;
	headcache->next = NULL;
	headcache->prev = NULL;
	headcache->domain = NULL;

	tailcache = headcache;
	cacheSize = sizeof(cache);
	cacheuse = 1;
	return;
}
void addCache(char* data, char* name, char* domain)
{
	cache* newcache;
	unsigned size;

	P(&cache_sem);
	size = sizeof(cache) + strlen(name) + strlen(data) + strlen(domain) + 3;
	if(size > (MAX_CACHE_SIZE - cacheSize)){
		evictCache(data, name, domain);
	}
	newcache = Malloc(sizeof(cache));
	cacheuse++;
	cacheSize += sizeof(cache) + strlen(name) + strlen(data) + strlen(domain) + 3 ;

	newcache->name = Malloc(strlen(name) + 1);
	strcpy(newcache->name, name);
	newcache->LRUtag = cacheuse;
	newcache->size = sizeof(cache) + strlen(name) + strlen(data) + strlen(domain) + 3 ;
	newcache->data = Malloc(strlen(data) + 1);
	strcpy(newcache->data, data);
	newcache->domain = Malloc(strlen(domain) + 1);
	strcpy(newcache->domain, domain);
	newcache->next = tailcache;
	tailcache->prev = newcache;
	newcache->prev = NULL;

	tailcache = newcache;
	V(&cache_sem);
	return;
}
cache* isHit(req* request)
{
	cache* search;

	P(&cache_sem);
	for(search = tailcache; search != NULL; search = search->next){
		if(search->name == NULL)
			break;
		if(strcmp(request->path, search->name) == 0 && strcmp(request->domain, search->domain) == 0){
			cacheuse++;
			search->LRUtag = cacheuse;
			return search;
		}
	}
	V(&cache_sem);
	return NULL;
}
void evictCache(char* data, char* name, char* domain)
{
	cache* search;
	unsigned requestSize;
	cache* temp;

	P(&cache_sem);
	requestSize = sizeof(cache) + strlen(name) + strlen(data) +strlen(domain) + 3 - (MAX_CACHE_SIZE - cacheSize);
	temp = NULL;

	for(search = tailcache; search != NULL; search = search->next){
		if(search->size >= requestSize){
			if(temp == NULL)
				temp = search;
			else{
				if(search->LRUtag < temp->LRUtag){
					temp = search;
				}
			}
		}
	}
	if(temp->prev)
		temp->prev->next = temp->next;
	if(temp->next)
		temp->next->prev = temp->prev;
	Free(temp -> data);
	Free(temp -> name);
	Free(temp);
	V(&cache_sem);
	return;
}

