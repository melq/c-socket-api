#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PORT 50003
#define BSIZE 1024
#define STRSIZE 8191
#define NUMTHREAD 10

int lfd;

void svr_init() {
  int sockopt = 1;
  struct sockaddr_in  sa;

  printf("server start\n");

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  //ソケット作成
    perror("socket error"); exit(1);
  }

  setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);  //使用中の全アドレスで待ち受け
  sa.sin_port = htons(PORT);

  if (bind(lfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {  //ソケットの設定
    perror("bind error"); exit(1);
  }
  if (listen(lfd, 0) < 0) {  //待ち受け
    perror("listen error"); exit(1);
  }
}

void translate(char *input_text, char *output_text, int mode, int key) {
  int i = 0;

  if (mode == 1) {
    while(input_text[i] != '\0') {
      output_text[i] = 'a' + (input_text[i] - 'a' + key) % 26;
      i++;
    }
  } else if (mode == 2) {
    while(input_text[i] != '\0') {
      output_text[i] = 'z' - ('z' - input_text[i] + key) % 26;
      i++;
    }
  }
}

void process(int lfd) {
  int cfd, mode, key, i;
  char buf[BSIZE], input_text[STRSIZE], output_text[STRSIZE];
  char byte[4];
  int *str_len = (int *)byte;

  while (1) {
    cfd = accept(lfd, NULL, NULL);  //要求受付
    read(cfd, &mode, 1);
    read(cfd, &key, 1);
    read(cfd, (char *)str_len, sizeof(int));
    *str_len = ntohl(*str_len);
    read(cfd, input_text, *str_len);
    printf("input: mode: %d, key: %d, str_len: %d, text: %s\n", (int)mode, (int)key, *str_len, input_text);

    translate(input_text, output_text, mode, key);

    printf("output: %s\n\n", output_text);
    write(cfd, output_text, strlen(output_text) + 1);
    
    for (i = 0; i < *str_len; i++)
      output_text[i] = '\0';

    shutdown(cfd, SHUT_WR);  //もうこれ以上データの送信はしない
    while (read(cfd, buf, BSIZE) > 0);  //クライアントがclose()したら0が返る
    close(cfd);
  }
}

int main(void)
{
  int i;
  pthread_t tid[NUMTHREAD];

  svr_init();

  process(lfd);
  
  return 0;
}