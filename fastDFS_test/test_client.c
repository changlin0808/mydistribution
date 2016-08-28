#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#include "make_log.h"

#define LOG_MODULE      "test"
#define LOG_PROC        "fastDFS"


#define FILE_ID_LEN     (256)


int main(int argc, char *argv[])
{
    char file_id[FILE_ID_LEN] = {0};

    if (argc < 2) {
        printf("./a.out [filename]\n");
        exit(1);
    }

    char *file_name = argv[1];


    pid_t pid;
    int pfd[2];

    if (pipe(pfd) < 0) {
        printf("pip error\n");
    }

    pid = fork();
    if (pid < 0) {
        printf("fork error\n");
    }

    if (pid == 0) {
        //child
        //关闭读管道
        close(pfd[0]);

        //将标准输出重定向到写管道
        dup2(pfd[1], STDOUT_FILENO);

        execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", file_name, NULL);

        printf("execlp fdfs_upload_file error\n");
        close(pfd[1]);
    }

    //关闭写管道
    close(pfd[1]);

    wait(NULL);

    //read from 读管道
    read(pfd[0], file_id, FILE_ID_LEN);

    //printf("get file_id[%s]\n", file_id);
    LOG(LOG_MODULE, LOG_PROC, "get file_id[%s]", file_id);



    return 0;
}
