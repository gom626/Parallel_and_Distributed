#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "http_lib.h"

#define MAX_REQUEST_LEN 300
#define REQUEST_QUEUE_SIZE  1000
#define SERV_TCP_PORT 15600

struct request {
    char buffer[MAX_REQUEST_LEN+1];
    int sockfd_cnt;
};

struct request_queue {
    struct request *reqs[REQUEST_QUEUE_SIZE];
    int front, rear;
    pthread_mutex_t mutex;
    pthread_cond_t cv_notfull, cv_notempty;
};

int request_queue_init(struct request_queue *q);
int request_queue_destroy(struct request_queue *q);
int request_queue_push(struct request_queue *q, struct request *req);
int request_queue_pop(struct request_queue *q, struct request **p_req);
void * thread_main(void * param);

int main(int argc, char *argv[])
{
	int sockfd,newsockfd,aa;
	struct sockaddr_in cli_addr,serv_addr;
	int thread_cnt;
	struct request_queue queue;
	struct request *buf;
	int cnt,i;
	
	if(argc!=2){
		printf("사용법 : %s  [쓰레드 개수]", argv[0]);
		return -1;
	}

	request_queue_init(&queue);
	
    thread_cnt=atoi(argv[1]);

	printf("Creating %d thread pool!!\n",thread_cnt);
	for(i=0;i<thread_cnt;i++){
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid,&attr,thread_main,&queue);
	}	

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server : can’t open stream socket\n"); return(0); }
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_TCP_PORT);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Server : can’t bind local address \n"); return(0); }
	
	listen(sockfd, 5);
    aa = sizeof(cli_addr) ;
	

	for(;;){
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &aa);
	if (newsockfd < 0) {
		printf("Server : accept error  \n"); return(0); }
		buf=(struct request *)malloc(sizeof(struct request));
		if ((cnt=read(newsockfd, buf, 1024)) <= 0) {
			printf("Server : readn error  \n");
			close(sockfd);
			free(buf);
			continue;
		}
		else{
			buf->buffer[cnt]='\0';
			buf->sockfd_cnt;
			request_queue_push(&queue,buf);
		}
	}
	request_queue_destroy(&queue);
	
	return 0;
}

int request_queue_init(struct request_queue *q)
{
    q->front = 0;
    q->rear = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cv_notfull, NULL);
    pthread_cond_init(&q->cv_notempty, NULL);
    return 0;
}

int request_queue_destroy(struct request_queue *q)
{
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cv_notfull);
    pthread_cond_destroy(&q->cv_notempty);
    return 0;
}

int request_queue_push(struct request_queue *q, struct request *req)
{
    pthread_mutex_lock(&q->mutex);
    int new_rear = (q->rear+1) % REQUEST_QUEUE_SIZE;
    while (new_rear == q->front) {
        pthread_cond_wait(&q->cv_notfull, &q->mutex);
        new_rear = (q->rear+1) % REQUEST_QUEUE_SIZE;
    }
    q->reqs[q->rear] = req;
    q->rear = new_rear;
    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->cv_notempty);
    return 0;
}

int request_queue_pop(struct request_queue *q, struct request **p_req)
{
    pthread_mutex_lock(&q->mutex);
    while (q->front == q->rear) {
        pthread_cond_wait(&q->cv_notempty, &q->mutex);
    }
    *p_req = q->reqs[q->front];
    q->front = (q->front+1) % REQUEST_QUEUE_SIZE;
    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->cv_notfull);
    return 0;
}
void* thread_main(void* param)
{
    struct request_queue *q = (struct request_queue*) param;

    while(1)
    {
        struct request *req;
        request_queue_pop(q, &req);
        process_request(req->buffer, req->sockfd_cnt);
        close(req->sockfd_cnt);
        free(req);
    }
}

