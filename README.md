# ICMP_Implement
a sample implementation for a ping program. this uses row sockets in order to implement ICMP 
The core bases for implementing the icmp is to:
1- define a row socket which uses PF_INET as A PROTOCOL family and IPPROTO_ICMP as A protocol
2- to implement the icmp packet header with the structure which is found in the "ip_icmp.h"
struct icmphdr
{
  uint8_t type;		/* message type */
  uint8_t code;		/* type sub-code */
  uint16_t checksum;
  union
  {
    struct
    {
      uint16_t	id;
      uint16_t	sequence;
    } echo;			/* echo datagram */
    uint32_t	gateway;	/* gateway address */
    struct
    {
      uint16_t	__glibc_reserved;
      uint16_t	mtu;
    } frag;			/* path mtu discovery */
  } un;
};

the important part for this sample is to fill the type, checksum, un.echo.sequence and un.echo.id


