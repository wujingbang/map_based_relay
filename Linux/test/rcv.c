 
#include <stdio.h>  

#include <sys/socket.h> 

#include <unistd.h>  

#include <sys/types.h>

#include <netdb.h>  

#include <netinet/in.h> 

#include <arpa/inet.h> 

#include <string.h>  

#include <stdint.h>

#include <sys/mman.h>
   
#include <sys/stat.h>  

#include <fcntl.h>

#include <stdlib.h>

#include <linux/netlink.h>

#include <errno.h>

#define NEIGH_STATUS_OFFSET 0
#define NEIGH_COUNT_OFFSET  1
#define NEIGH_DATA_OFFSET   5

struct neighbor{

uint32_t ip;

uint8_t mac[6];

uint64_t geoHash;

}*temp;

typedef struct neighbor_table_ {

    uint32_t ip;

    uint8_t mac[6];

    uint64_t geoHash;
    
    int     isvalid;

}neighbor_table;

neighbor_table  *neigh_data;

uint8_t      *neigh_status;

uint32_t     *neigh_count;

void *g_pBuffer = NULL;

int g_fd = -1;

void get_buffer_entry()
{

    if (NULL != g_pBuffer)
    {

        return ;
    }

    if (g_fd < 0)
    {
        g_fd = open("/dev/mbr_neighbor_mem", O_RDWR);

        if (0 > g_fd)
        {
            printf("Error : getMemMsgBuf() open /dev/mbr_neighbor_mem failed \n");

            return ;
        }
    }

    g_pBuffer = mmap(NULL, 4096 * 4,PROT_READ|PROT_WRITE,MAP_SHARED, g_fd,0 );

    if( g_pBuffer == MAP_FAILED ) 
    {
        printf("Error : getMemMsgBuf() fail mmap!\n");

    }
    return ;
}

void add_neighbor(struct neighbor* temp)
{
    int i,k;
    
    neighbor_table  *entry = neigh_data;

    *neigh_status = 1;

    for(k = 0; k < *neigh_count; ++k)
    {
        if(temp->ip == entry->ip)
        {
            for(i = 0; i < 6; ++i)

                entry->mac[i] = temp->mac[i];

            entry->geoHash = temp->geoHash;

            break;
        }
        else
        {
            ++entry;
        }
    }

    if(k == *neigh_count)
    {
        entry->ip = temp->ip;

        for(i = 0; i < 6; ++i)

            entry->mac[i] = temp->mac[i];

        entry->geoHash = temp->geoHash;

        *neigh_count += 1;
    }

    *neigh_status=0;

    return ;
}
        
int main() 

{  
    int i;

    setvbuf(stdout, NULL, _IONBF, 0);

    fflush(stdout);   

    // 绑定地址  
    struct sockaddr_in addrto; 

    bzero(&addrto, sizeof(struct sockaddr_in));

    addrto.sin_family = AF_INET;

    addrto.sin_addr.s_addr = htonl(INADDR_ANY);

    addrto.sin_port = htons(6000);  

    // 广播地址  
    struct sockaddr_in from; 

    bzero(&from, sizeof(struct sockaddr_in)); 

    from.sin_family = AF_INET;  

    from.sin_addr.s_addr = htonl(INADDR_ANY);  

    from.sin_port = htons(6000);  

    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
    {     
        printf("%s\n","socket error!");
  
        return 0;  
    }     

    const int opt = 1; 

    //设置该套接字为广播类型，

    int nb = 0;  

    nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));  
    if(nb == -1)  
    {  
        printf("%s\n","set socket error...");
  
        return 0;  
    }  

    if(bind(sock,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1)   
    {     
        printf("%s\n","bind error...");
         
        return 0;  
    }  

    int len = sizeof(struct sockaddr_in); 

    char smsg[20] = {0};  

    get_buffer_entry();

    neigh_status = (uint8_t*)((uint8_t*)g_pBuffer + NEIGH_STATUS_OFFSET);

    neigh_count = (uint32_t*)((uint8_t*)g_pBuffer + NEIGH_COUNT_OFFSET);

    *neigh_count = 0;

    neigh_data = (neighbor_table*)((uint8_t*)g_pBuffer + NEIGH_DATA_OFFSET);


    while(1)  
    {  
        //从广播地址接受消息  

        int ret=recvfrom(sock, smsg, 20, 0, (struct sockaddr*)&from,(socklen_t*)&len); 

        if(ret<=0)  
        {  
            printf("%s\n","read error....");

        }  
        else  
        {     
            temp = (struct neighbor*)smsg;

            add_neighbor(temp);
  
            //printf("%x ", temp->ip);  

            //for(i=0; i<6; ++i)

            //printf("%x\n",temp->mac[i]);
  
        }  
  
    }  

    return 0;  
}
