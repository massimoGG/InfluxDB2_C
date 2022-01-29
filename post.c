#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>


#define IP_ADDRESS ""
#define PORT        8086
#define ORG         ""
#define BUCKET      ""
#define TOKEN       ""

#define MEASUREMENT "noise"

#define HOSTNAME "blue"

#define USECONDS 1
#define LOOPS   120000

#define BUFSIZE 8196

void pexit(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char argv[])
{
    int sockfd;
    char header[BUFSIZE];
    char body[BUFSIZE];
    char result[BUFSIZE];

    static struct sockaddr_in serv_addr;

    printf("Connecting socket to %s and port %d\n", IP_ADDRESS, PORT);
    if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0)
        pexit("socket() failed");

    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = inet_addr(IP_ADDRESS);
    serv_addr.sin_port          = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
        pexit("connect() failed");

    float f;
    for (int i=0; i<LOOPS; i++) {

        // BODY
        sprintf(body, "%s,host=%s random=%.3f \nnoise,host=red random=%.3f\n",
                MEASUREMENT,HOSTNAME, ((double)(random())/1000.0), sin(f));
        f+=0.5;
        if (f > 2*3.14)
            f=0;

        // HEADER
        sprintf(header, "POST /api/v2/write?org=%s&bucket=%s&precision=ns HTTP/1.1\r\nHost: influx:8086\r\nAuthorization: Token %s\r\nContent-Length: %ld\r\n\r\n",
                ORG, BUCKET, TOKEN, strlen(body));

        if (write(sockfd, header, strlen(header)) < 0 )
            pexit("Header Write socket failed");
        printf("Sent header to InfluxDB: %s\n",header);

        if (write(sockfd, body, strlen(body)) < 0)
            pexit("Body Write socket failed");
        printf("Sent body to InfluxDB: %s\n",body);

        int ret = read(sockfd, result, sizeof(result));

        if (ret < 0)
            pexit("Reading the result from InfluxDB failed");

        result[ret] = 0; /* terminate string */
        printf("Result returned from InfluxDB. Note:204 is Success\n->|%s|<-\n",result);
        sleep(USECONDS);
    }
    close(sockfd);
}
