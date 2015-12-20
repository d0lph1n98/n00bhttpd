/*
 * vim:sw=4 ts=4:et sta
 *
 *
 * Copyright (c) 2015, Fakhri Zulkifli <d0lph1n98@yahoo.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of n00bhttpd nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#define LOG "./log/access.log"

int main(int c, char **v)
{
    FILE *fp;
    ssize_t i;
    time_t tm = time(NULL);
    char *index = v[2];
    const char *get = "GET /";
    const char *http = " HTTP";
    char *target = NULL;
    char *start, *end;
    int tcpfd = 0;
    int connfd = 0;
    char content[1024];
    char storage[1024];
    struct sockaddr_in serv_addr;

    memset(content, 0, sizeof(content));
    memset(storage, 0, sizeof(storage));

    if (c != 3)
    {
        printf("Usage: %s <port> <index_page>\n", v[0]);
        return EXIT_FAILURE;
    }

    /*
     * NOTE: Temporarily commented out
     */
    /* if ((chdir(v[2])) < 0)  */
    /* { */
    /*     perror("Error"); */
    /*     return EXIT_FAILURE; */
    /* } */

    if ((tcpfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error");
        return EXIT_FAILURE;
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t)atoi(v[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((bind(tcpfd, (struct sockaddr*) &serv_addr, (socklen_t) sizeof(serv_addr))) < 0)
    {
        perror("Error");
        return EXIT_FAILURE;
    }

    if (listen(tcpfd, 1024) < 0)
    {
        perror("Error");
        return EXIT_FAILURE;
    }

    setsockopt(tcpfd, IPPROTO_TCP, TCP_NODELAY | SO_REUSEPORT, &serv_addr, (socklen_t) sizeof(serv_addr));

    fp = fopen(index, "r");

    if (!fp)
    {
        perror("Error");
        return 1;
    }

    for (;;)
    {
        if ((connfd = accept(tcpfd, NULL, NULL)) < 0)
        {
            perror("Error");
            return EXIT_FAILURE;
        }

        signal(SIGCHLD, NULL);

        while ((i = read(connfd, content, sizeof(content) - 1)) > 0)
        {
            fp = fopen(LOG, "a+");
            content[i] = 0;
            fprintf(fp, "Date: %s", asctime(gmtime(&tm)));
            fprintf(fp, "%s", content);
            (void) fflush(fp);
            fputs(content, stdout);
            if ((start = strstr(content, get)))
            {
                start += strlen(get);
                if ((end = strstr(start, http)))
                {
                    target = (char *) malloc(end - start + 1);
                    memcpy(target, start, end - start);
                    target[end - start] = '\0';
                }
            }

            if (strlen(target) > 1)
            {
                fp = fopen(target, "r");

                if (!fp)
                {
                    perror("Error");
                    return 1;
                }
                free(target);
            }
            else 
            {
                fp = fopen(index, "r");
            }
            break;
        }

        if (!fork())
        {
            while ((fgets(content, (size_t) sizeof(content), fp)) != NULL)
            {
                strncat(storage, content, sizeof(storage) - 1);
            }
            send(connfd, "HTTP/1.1 200\r\n\r\n", 16, 0);
            send(connfd, storage, sizeof(storage) - 1, 0);
            memset(content, 0, sizeof(content));
            memset(storage, 0, sizeof(storage));
        }
        close(connfd);
    }
    shutdown(tcpfd, 0);
    close(tcpfd);
    return EXIT_SUCCESS;
}
