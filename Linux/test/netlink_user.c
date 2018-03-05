#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include "geohash.h"
#include "geohash.c"

#define NETLINK_USER 22
#define USER_MSG    (NETLINK_USER + 1)
#define MSG_LEN 60

#define MAX_PLOAD 60

struct _my_msg
{
    struct nlmsghdr hdr;
    char  data[MSG_LEN];
};

typedef struct deliver
{
    short mode;
    union
    {
        struct 
        {
            char vertex[25];
            uint64_t geohash;
        }vertex;
        struct 
        {
            char from[25];
            char to[25];
            int road_id;
        }edge;
    }parameter;
}deliver;

//int send_message_to_kernel(deliver *d)
int message_send(deliver d)
{
    deliver *p;
    char *data = (char*)&d;
    struct sockaddr_nl  local, dest_addr;
    int skfd;
    struct nlmsghdr *nlh = NULL;
    struct _my_msg info;
    int ret;

    skfd = socket(AF_NETLINK, SOCK_RAW, USER_MSG);
    if(skfd == -1)
    {
        printf("create socket error...%s\n", strerror(errno));
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = 50; 
    local.nl_groups = 0;
    if(bind(skfd, (struct sockaddr *)&local, sizeof(local)) != 0)
    {
        printf("bind() error\n");
        close(skfd);
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // to kernel
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PLOAD));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = local.nl_pid; //self port

    memcpy(NLMSG_DATA(nlh), data, sizeof(deliver));
    ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_nl));

    if(!ret)
    {
        perror("sendto error1\n");
        close(skfd);
        exit(-1);
    }
    printf("wait kernel msg!\n");
    memset(&info, 0, sizeof(info));
    ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(!ret)
    {
        perror("recv form kernel error\n");
        close(skfd);
        exit(-1);
    }
    p=(deliver*)(&info.data);
    close(skfd);
    printf("%hd\n", p->mode);
    free((void *)nlh);
    return 0;

}

int main()
{
	deliver d;
    GeoHashBits hash;
    GeoHashRange lat_range={40.15929, 39.74732}, lon_range={ 116.73407, 116.16677};
    geohash_fast_encode(lat_range, lon_range, 40.00111, 116.32174, 12, &hash);
	d.mode=2;
    strcpy(d.parameter.vertex.vertex,"40.00111,116.32174");
    d.parameter.vertex.geohash = hash.bits;
    message_send(d);

    geohash_fast_encode(lat_range, lon_range, 39.99974, 116.32189, 12, &hash);
    d.mode=2;
    strcpy(d.parameter.vertex.vertex,"39.99974,116.32189");
    d.parameter.vertex.geohash = hash.bits;
    message_send(d);

    geohash_fast_encode(lat_range, lon_range, 39.99979, 116.32404, 12, &hash);
    d.mode=2;
    strcpy(d.parameter.vertex.vertex,"39.99979,116.32404");
    d.parameter.vertex.geohash = hash.bits;
    message_send(d);

    d.mode=3;
    strcpy(d.parameter.edge.from,"40.00111,116.32174");
    strcpy(d.parameter.edge.to,"39.99974,116.32189");
    d.parameter.edge.road_id=10000;
    message_send(d);

    d.mode=3;
    strcpy(d.parameter.edge.from,"39.99974,116.32189");
    strcpy(d.parameter.edge.to,"39.99979,116.32404");
    d.parameter.edge.road_id=20000;
    message_send(d);

    return 0;

}
