#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 50003
#define BSIZE 1024
#define STRSIZE 8191

void client_init(int *lfd) {
  int svr_sel = 0;
  char host[STRSIZE];
  struct hostent *he;
  struct sockaddr_in sa;

  if (svr_sel == 0) strcpy(host, "localhost");
  else strcpy(host, "knsk.freeddns.org");
  if ((he = gethostbyname(host)) == NULL) {
    perror("server name error"); exit(1);
  }

  if ((*lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error"); exit(1);
  }

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = *(unsigned int *)he->h_addr_list[0];
  sa.sin_port = htons(PORT);
  if (connect(*lfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    perror("connection error"); exit(1);
  }
}

void user_input(char *mode, char *key, char *input_text, int **str_len) {
  int mode_value, key_value;
  char byte[4];
  *str_len = (int *)byte;

  printf("1: 暗号化, 2: 復号: "); scanf("%d", &mode_value);
  *mode = mode_value;
  printf("KEY(1-100): "); scanf("%d", &key_value);
  *key = key_value;
  printf("文字列を入力(1-8191文字): "); scanf("%s", input_text);
  **str_len = strlen(input_text) + 1;
  **str_len = htonl(**str_len);
}

int main(void) {
  int lfd, i = 0;
  char mode, key;
  char buf[BSIZE], input_text[STRSIZE], output_text[STRSIZE];
  int *str_len;

  client_init(&lfd);
  user_input(&mode, &key, input_text, &str_len);

  write(lfd, &mode, 1);
  write(lfd, &key, 1);
  write(lfd, (char *)str_len, sizeof(int));
  write(lfd, input_text, ntohl(*str_len));

  read(lfd, buf, BSIZE);
  printf("変換結果: %s\n", buf);

  close(lfd);

  return 0;
}