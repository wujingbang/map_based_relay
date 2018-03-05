 
#include <stdio.h>  
#include <sys/socket.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <string.h>  
  
struct neighbor {

uint32_t ip;

uint8_t mac[6];

uint64_t geoHash;

}*temp;
  
int main() 

{  

  setvbuf(stdout, NULL, _IONBF, 0);  

  fflush(stdout);   

  int sock = -1;  

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
  {     
      printf("%s\n","socket error");

      return ;  
  }     

  const int opt = 1;  

  //设置该套接字为广播类型， 

  int nb = 0; 

  nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));  

  if(nb == -1)  
  {  
    printf("%s\n","set socket error...");
  
    return ;  
  }  

  struct sockaddr_in addrto; 

  bzero(&addrto, sizeof(struct sockaddr_in)); 

  addrto.sin_family=AF_INET; 

  addrto.sin_addr.s_addr=inet_addr("192.168.1.255");

  //addrto.sin_addr.s_addr=htonl(INADDR_BROADCAST);

  addrto.sin_port=htons(6000);

  int nlen=sizeof(addrto);  

  //从广播地址发送消息  

  char smsg[20];

  temp = (struct neighbor*)smsg;

  temp->ip = inet_addr("192.168.1.161");

  temp->mac[0] = 0x54; temp->mac[1] = 0x27; temp->mac[2] = 0x1E;

  temp->mac[3] = 0x1A; temp->mac[4] = 0x77; temp->mac[5] = 0x99;

  temp->geoHash = 6386669;

  while(1)

  {

    int ret=sendto(sock, smsg, 20, 0, (struct sockaddr*)&addrto, nlen);  

    if(ret<0)  
    {  
      printf("%s\n","send error...."); 
    }  
    else  
    {         
      //printf("%s\n","ok !!!");    
    }  

    sleep(1); 
  }

  return 0;  
}
