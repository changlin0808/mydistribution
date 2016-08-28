#include "fcgi_config.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include "make_log.h"

#include "fcgi_stdio.h"
#include "util_cgi.h"
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <hiredis/hiredis.h>

void get_ip(char* ip, char* onlyip)
{   

    char* p = NULL;
    p = strstr(ip, "source ip address: ");
    p = p + strlen("source ip address: ");
    char* begin = p;
    while(*p != 'f')
    {
        p++;
    }
    int len = p - begin;
    strncpy(onlyip, begin, len);

}


int main ()
{
    char *file_buf = NULL;
    char boundary[256] = {0};
    char content_text[256] = {0};
    char filename[256] = {0};
    char fdfs_file_path[256] = {0};
    char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};
    char fdfs_file_url[512] = {0};
    
    // connect to redis; create redis table 
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if(c->err)
    {
        redisFree(c);
        return -1;
    }


    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n"
                "\r\n");

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
            printf("No data from standard input\n");
        }
        else {
            int i, ch;
            char *begin = NULL;
            char *end = NULL;
            char *p, *q, *k;

            //==========> 开辟存放文件的 内存 <===========

            file_buf = malloc(len);
            if (file_buf == NULL) {
                printf("malloc error! file size is to big!!!!\n");
                return -1;
            }

            begin = file_buf;
            p = begin;
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
                //putchar(ch);
                *p = ch;
                p++;
            }

            //===========> 开始处理前端发送过来的post数据格式 <============
            //begin deal
            end = p;

            p = begin;

            //get boundary
            p = strstr(begin, "\r\n");
            if (p == NULL) {
                printf("wrong no boundary!\n");
                goto END;
            }

            strncpy(boundary, begin, p-begin);
            boundary[p-begin] = '\0';
            //printf("boundary: [%s]\n", boundary);

            p+=2;//\r\n
            //已经处理了p-begin的长度
            len -= (p-begin);

            //get content text head
            begin = p;

            p = strstr(begin, "\r\n");
            if(p == NULL) {
                printf("ERROR: get context text error, no filename?\n");
                goto END;
            }
            strncpy(content_text, begin, p-begin);
            content_text[p-begin] = '\0';
            //printf("content_text: [%s]\n", content_text);

            p+=2;//\r\n
            len -= (p-begin);

            //get filename
            // filename="123123.png"
            //           ↑
            q = begin;
            q = strstr(begin, "filename=");
            
            q+=strlen("filename=");
            q++;

            k = strchr(q, '"');
            strncpy(filename, q, k-q);
            filename[k-q] = '\0';

            trim_space(filename);
            //printf("filename: [%s]\n", filename);

            //get file
            begin = p;     
            p = strstr(begin, "\r\n");
            p+=4;//\r\n\r\n
            len -= (p-begin);

            begin = p;
            // now begin -->file's begin
            //find file's end
            p = memstr(begin, len, boundary);
            if (p == NULL) {
                p = end-2;    //\r\n
            }
            else {
                p = p -2;//\r\n
            }
        
            //begin---> file_len = (p-begin)

            //=====> 此时begin-->p两个指针的区间就是post的文件二进制数据
            //======>将数据写入文件中,其中文件名也是从post数据解析得来  <===========
            
            int fd = 0;
            fd = open(filename, O_CREAT|O_WRONLY, 0644);
            if (fd < 0) {
                printf("open %s error\n", fdfs_file_host_name);
                goto END; 
            }

            ftruncate(fd, (p-begin));
            write(fd, begin, (p-begin));
            close(fd);

// --------------------------------  upload flow ---------------------   start ----
char file_id[1024] = {0};


    // filename 


    pid_t pid;
    int pfd[2];

    if (pipe(pfd) < 0) {
        printf("pip error\n");
    }
    int pfd1[2];

    if (pipe(pfd1) < 0) {
        printf("pip1 error\n");
    }   





        pid = fork();

            if (pid < 0) {
        printf("fork error\n");
    }





    if (pid == 0) {
        //child
        //关闭读管道
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", filename, NULL);

        printf("execlp fdfs_upload_file error\n");
        close(pfd[1]);
    }


    //关闭写管道
    close(pfd[1]);
    wait(NULL);

    //read from 读管道
    read(pfd[0], file_id, 1024);



    //printf("get file_id[%s]\n", file_id);

    fd = open("file.txt", O_CREAT|O_WRONLY, 0644);
    if (fd < 0) {
        printf("open %s error\n", fdfs_file_host_name);
        goto END; 
    }
    write(fd, file_id, strlen(file_id));
    trim_space(file_id);
    close(fd);
    pid = fork();
    if(pid == 0)
    {
        close(pfd1[0]);
        dup2(pfd1[1], STDOUT_FILENO);
        execlp("fdfs_file_info", "fdfs_file_info", "/etc/fdfs/client.conf", file_id, NULL);
        printf("execlp fdfs_upload_file error\n");
        close(pfd1[1]);
    }
    close(pfd1[1]);
    wait(NULL);
    char ip[128] = { 0 };
    read(pfd1[0], ip, sizeof(ip));
    close(pfd1[0]);
    char file_url[1024] = { 0 };

    char onlyip[128] = { 0 };
    get_ip(ip, onlyip);
    trim_space(onlyip);
    sprintf(file_url, "http://%s/%s", onlyip, file_id);
    printf("%s", file_url);

// --------------------------------  upload flow ---------------------   end 
// --------------------------------- redis command ------------------ start

redisReply* r = redisCommand(c, "hset FILE_URL %s %s", filename, file_url);
if(r->type != REDIS_REPLY_INTEGER)
{
    freeReplyObject(r);
    redisFree(c);
    exit(1);
}
    freeReplyObject(r);


            //===============> 将该文件存入fastDFS中,并得到文件的file_id <============
            //================ > 得到文件所存放storage的host_name <=================
END:

            memset(boundary, 0, 256);
            memset(content_text, 0, 256);
            memset(filename, 0, 256);
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_stat_buf, 0, 256);
            memset(fdfs_file_host_name, 0, 30);
            memset(fdfs_file_url, 0, 512);

            free(file_buf);
            //printf("date: %s\r\n", getenv("QUERY_STRING"));
        }
    } /* while */


    redisFree(c);
    return 0;
}
