//#define DEBUG // Allow debugging
#include <os>
#include <net/class_udp.hpp>

using namespace net;

int UDP::bottom(std::shared_ptr<Packet> pckt){
  debug("<UDP handler> Got data \n");
  
  
  udp_header* hdr = &((full_header*)pckt->buffer())->udp_hdr;
  
  debug("\t Source port: %i, Dest. Port: %i Length: %i\n",
         __builtin_bswap16(hdr->sport),__builtin_bswap16(hdr->dport), 
         __builtin_bswap16(hdr->length));

  auto l = ports.find(__builtin_bswap16(hdr->dport));
  if (l != ports.end()){
    debug("<UDP> Someone's listening to this port. Let them hear it.\n");
    return l->second(pckt);
  }
  
  debug("<UDP> Nobody's listening to this port. Drop!\n");
    
  
  return -1;
}

void UDP::listen(uint16_t port, listener l){
  debug("<UDP> Listening to port %i \n",port);
  
  // any previous listeners will be evicted.
  ports[port] = l;
  
};

int UDP::transmit(IP4::addr sip,UDP::port sport,
                  IP4::addr dip,UDP::port dport,
                  uint8_t* data, int len){
  

  assert((uint32_t)len >= sizeof(UDP::full_header));
  
  udp_header* hdr = &((full_header*)data)->udp_hdr;
  
  // Populate all UDP header fields
  hdr->dport = dport;
  hdr->sport = sport; 

  hdr->length =  __builtin_bswap16((uint16_t)(len -sizeof(IP4::full_header)));
  hdr->checksum = 0; // This field is optional (must be 0 if not used)
  
  debug("<UDP> Transmitting %i bytes (big-endian 0x%x) to %s:%i \n",
        (uint16_t)(len -sizeof(full_header)),
        hdr->length,dip.str().c_str(),
        dport);
  
  debug("<UDP> sip: %s dip: %s, type: %i, len: %i  \n ",
        sip.str().c_str(),dip.str().c_str(),IP4::IP4_UDP,len
        );
  return _network_layer_out(sip,dip,IP4::IP4_UDP,data,len);

};

int ignore_udp(IP4::addr UNUSED(sip),IP4::addr UNUSED(dip),IP4::proto UNUSED(p),
               UDP::pbuf UNUSED(buf),uint32_t UNUSED(len)){
  debug("<UDP->Network> No handler - DROP!\n");
  return 0;
}

UDP::UDP() : _network_layer_out(UDP::network_out(ignore_udp)) {};
