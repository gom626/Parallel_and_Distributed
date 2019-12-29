#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1000
#define SERV_TCP_PORT 15600

typedef struct simulate_{
    int id;
    char *ip;   /* Don't modify deeply in a thread. It is shared. */
    unsigned short port;
    int num;
    int path_num;
    char **paths;   /* Don't modify deeply in a thread. It is shared. */
    unsigned int rand_seed;
}simulate;

void* request_thread_main(void *param)
{
    simulate *sim = (simulate *) param;
    char request[BUF_SIZE];
    char buf[BUF_SIZE];
    int cn;

    for (int i=0; i<sim->num; i++) {

        int sockfd;
        struct sockaddr_in serv_addr;
		
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(sim->ip);
		serv_addr.sin_port = htons(SERV_TCP_PORT);


        if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Client : can¡¯t open stream socket\n");
			i--;
            break;
        }

        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Client : can¡¯t connect to server \n");
            i--;
            close(sockfd);
            break;
        }
	//	printf("%d\n",sim->path_num);
        int access_cnt = rand() % sim->path_num;

        cn = snprintf(request, BUF_SIZE, "GET %s HTTP/1.0\r\n", sim->paths[access_cnt]);
        request[cn] = 0;

        printf("[Thread %d][#%d] Send GET %s HTTP/1.0\n", sim->id, i, sim->paths[access_cnt]);

        if(write(sockfd, request, cn) == -1) {
            printf("Client : writen error  \n");
			close(sockfd);
            break;
        }

        int read_bytes = 0;
        cn = read(sockfd, buf, BUF_SIZE);

        int status = -1;
        if(sscanf(buf, "%*s %d %*s\r\n", &status) != 1) {
            printf("parsing error : %s\n", buf);
            continue;
        }

        while(cn > 0) {
            read_bytes += cn;
            cn = read(sockfd, buf, BUF_SIZE);
        }

        if (status == 200) {
            printf("[Thread %d][#%d] Receive %d Bytes. (status : %d)\n", sim->id, i, read_bytes, status);
        }
        else {
            printf("[Thread %d][#%d] Error (status : %d)\n", sim->id, i, status);
        }

        close(sockfd);
    }

    pthread_exit(0);
}
int main(int argc, char **argv)
{
    int thread_num;
    simulate sim;
    simulate *sim_schedule;
    pthread_t *tids;

    if (argc <= 3) {
        printf("Usage : %s  <쓰레드> <접근 개수>  <the number of request paths> <path 1> <path 2> ... \n",argv[0]);
        return -1;
    }

    sim.ip = "127.0.0.1";
    sim.port = 15600;
    thread_num = atoi(argv[1]);
	sim.num = atoi(argv[2]);
    sim.path_num = 1;
    sim.paths = &argv[3];

    printf("Creating %d reqeust threads!\n", thread_num);

    sim_schedule = malloc(sizeof(*sim_schedule) * thread_num);
    tids = malloc(sizeof(*tids) * thread_num);

    for(int i=0; i<thread_num; ++i) {
        sim_schedule[i] = sim;
        sim_schedule[i].id = i;
        sim_schedule[i].rand_seed = time(NULL) + i;
    }

    double start, finish;


    for(int i=0; i<thread_num; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i], &attr, request_thread_main, &sim_schedule[i]);
    }

    for(int i=0; i<thread_num; ++i) {
        pthread_join(tids[i], NULL);
    }

    printf("Elapsed Time : %e seconds.\n", finish - start);

    free(sim_schedule);
    free(tids);

    return 0;
}

