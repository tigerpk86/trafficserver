/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

/****************************************************************************

  P_UDPPacket.h
  Implementation of UDPPacket
  
  
 ****************************************************************************/


#ifndef __P_UDPPPACKET_H_
#define __P_UDPPPACKET_H_

#include "I_UDPNet.h"
#include "P_UDPConnection.h"

//#define PACKETQUEUE_IMPL_AS_PQLIST
#define PACKETQUEUE_IMPL_AS_RING

class UDPPacketInternal:public UDPPacket
{

public:
  UDPPacketInternal();
  virtual ~ UDPPacketInternal();

  void append_bytes(char *buf, int len);
  void append_block_internal(IOBufferBlock * block);

  virtual void free();

    SLink<UDPPacketInternal> alink;  // atomic link
  // packet scheduling stuff: keep it a doubly linked list
    Link<UDPPacketInternal> slink;
  // From the packet scheduler point of view...
  inku64 m_pktSendStartTime;
  inku64 m_pktSendFinishTime;
  inku32 m_pktLength;

  bool m_isReliabilityPkt;

  int m_reqGenerationNum;
  // Associate a TS seq. # with each packet...  We need this for WMT---WMT
  // maintains its own sequence numbers that need to increment by 1 on each
  // packet send.  Since packets can be cancelled during a seek, WMT needs to
  // know the next "WMT seq. #" that it can tag to a packet.  To determine the
  // "WMT seq. #", WMT code maintains a mapping betweeen WMT seq. # and TS
  // seq. #.  If m_pktTSSeqNum is set to -1, then this value is ignored by the
  // UDP code.
  ink64 m_pktTSSeqNum;

  ink_hrtime m_delivery_time;   // when to deliver packet
  ink_hrtime m_arrival_time;    // when packet arrived

    Ptr<IOBufferBlock> m_chain;
  Continuation *m_cont;         // callback on error
  UDPConnectionInternal *m_conn;        // connection where packet should be sent to.

#if defined(PACKETQUEUE_IMPL_AS_PQLIST) || defined(PACKETQUEUE_IMPL_AS_RING)
  int in_the_priority_queue;
  int in_heap;
#endif

  virtual void UDPPacket_is_abstract()
  {
  };
};

inkcoreapi extern ClassAllocator<UDPPacketInternal> udpPacketAllocator;

inline
UDPPacketInternal::UDPPacketInternal()
  :
m_pktSendStartTime(0)
  ,
m_pktSendFinishTime(0)
  ,
m_pktLength(0)
  ,
m_isReliabilityPkt(false)
  ,
m_reqGenerationNum(0)
  ,
m_pktTSSeqNum(-1)
  ,
m_delivery_time(0)
  ,
m_arrival_time(0)
  ,
m_cont(NULL)
  ,
m_conn(NULL)
#if defined(PACKETQUEUE_IMPL_AS_PQLIST) || defined(PACKETQUEUE_IMPL_AS_RING)
,
in_the_priority_queue(0)
  ,
in_heap(0)
#endif
{
  memset(&m_from, '\0', sizeof(m_from));
  memset(&m_to, '\0', sizeof(m_to));
}

inline
UDPPacketInternal::~
UDPPacketInternal()
{
  m_chain = NULL;
}

inline void
UDPPacketInternal::free()
{
  m_chain = NULL;
  if (m_conn)
    m_conn->Release();
  m_conn = NULL;
  udpPacketAllocator.free(this);
}

inline void
UDPPacketInternal::append_bytes(char *buf, int len)
{
  IOBufferData *d = NULL;
  if (buf) {
    d = new_xmalloc_IOBufferData(buf, len);
    append_block(new_IOBufferBlock(d, len));
  }
}

INK_INLINE void
UDPPacket::setReliabilityPkt()
{
  UDPPacketInternal *p = (UDPPacketInternal *) this;

  p->m_isReliabilityPkt = true;
}

INK_INLINE void
UDPPacket::setPktTSSeq(ink64 seqno)
{
  UDPPacketInternal *p = (UDPPacketInternal *) this;
  p->m_pktTSSeqNum = seqno;
}

INK_INLINE void
UDPPacket::append_block(IOBufferBlock * block)
{
  UDPPacketInternal *p = (UDPPacketInternal *) this;

  if (block) {
    if (p->m_chain) {           // append to end
      IOBufferBlock *last = p->m_chain;
      while (last->next != NULL) {
        last = last->next;
      }
      last->next = block;
    } else {
      p->m_chain = block;
    }
  }
}

INK_INLINE char *
UDPPacket::asBuf(int *len)
{
  UDPPacketInternal *p = (UDPPacketInternal *) this;
  if (p->m_chain) {
    if (len)
      *len = p->m_chain->size();
    return p->m_chain->start();
  } else {
    return NULL;
  }
}

INK_INLINE int
UDPPacket::getPktLength()
{
  UDPPacketInternal *p = (UDPPacketInternal *) this;
  IOBufferBlock *b;

  p->m_pktLength = 0;
  b = p->m_chain;
  while (b) {
    p->m_pktLength += b->read_avail();
    b = b->next;
  }
  return p->m_pktLength;
}

INK_INLINE void
UDPPacket::free()
{
  ((UDPPacketInternal *) this)->free();
}

INK_INLINE void
UDPPacket::setContinuation(Continuation * c)
{
  ((UDPPacketInternal *) this)->m_cont = c;
}

INK_INLINE void
UDPPacket::setConnection(UDPConnection * c)
{
  /*Code reviewed by Case Larsen.  Previously, we just had
     ink_assert(!m_conn).  This prevents tunneling of packets
     correctly---that is, you get packets from a server on a udp
     conn. and want to send it to a player on another connection, the
     assert will prevent that.  The "if" clause enables correct
     handling of the connection ref. counts in such a scenario. */

  UDPConnectionInternal *&conn = ((UDPPacketInternal *) this)->m_conn;

  if (conn) {
    if (conn == c)
      return;
    conn->Release();
    conn = NULL;
  }
  conn = (UDPConnectionInternal *) c;
  conn->AddRef();
}

INK_INLINE IOBufferBlock *
UDPPacket::getIOBlockChain(void)
{
  return ((UDPPacketInternal *) this)->m_chain;
}

INK_INLINE UDPConnection *
UDPPacket::getConnection(void)
{
  return ((UDPPacketInternal *) this)->m_conn;
}

INK_INLINE void
UDPPacket::setArrivalTime(ink_hrtime t)
{
  ((UDPPacketInternal *) this)->m_arrival_time = t;
}

INK_INLINE UDPPacket *
new_UDPPacket(struct sockaddr_in *to, ink_hrtime when, char *buf, int len)
{
  UDPPacketInternal *p = udpPacketAllocator.alloc();

#if defined(PACKETQUEUE_IMPL_AS_PQLIST) || defined(PACKETQUEUE_IMPL_AS_RING)
  p->in_the_priority_queue = 0;
  p->in_heap = 0;
#endif
  p->m_delivery_time = when;
  memcpy(&p->m_to, to, sizeof(p->m_to));

  if (buf) {
    IOBufferBlock *body = new_IOBufferBlock();
    body->alloc(iobuffer_size_to_index(len));
    memcpy(body->end(), buf, len);
    body->fill(len);
    p->append_block(body);
  }

  return p;
}

INK_INLINE UDPPacket *
new_UDPPacket(struct sockaddr_in * to, ink_hrtime when, IOBufferBlock * buf, int len)
{
  (void) len;
  UDPPacketInternal *p = udpPacketAllocator.alloc();
  IOBufferBlock *body;

#if defined(PACKETQUEUE_IMPL_AS_PQLIST) || defined(PACKETQUEUE_IMPL_AS_RING)
  p->in_the_priority_queue = 0;
  p->in_heap = 0;
#endif
  p->m_delivery_time = when;
  memcpy(&p->m_to, to, sizeof(p->m_to));

  while (buf) {
    body = buf->clone();
    p->append_block(body);
    buf = buf->next;
  }
  return p;
}

INK_INLINE UDPPacket *
new_UDPPacket(struct sockaddr_in * to, ink_hrtime when, Ptr<IOBufferBlock> buf)
{
  UDPPacketInternal *p = udpPacketAllocator.alloc();

#if defined(PACKETQUEUE_IMPL_AS_PQLIST) || defined(PACKETQUEUE_IMPL_AS_RING)
  p->in_the_priority_queue = 0;
  p->in_heap = 0;
#endif
  p->m_delivery_time = when;
  if (to)
    memcpy(&p->m_to, to, sizeof(p->m_to));
  p->m_chain = buf;
  return p;
}

INK_INLINE UDPPacket *
new_UDPPacket(ink_hrtime when, Ptr<IOBufferBlock> buf)
{
  return new_UDPPacket(NULL, when, buf);
}

INK_INLINE UDPPacket *
new_incoming_UDPPacket(struct sockaddr_in * from, char *buf, int len)
{
  UDPPacketInternal *p = udpPacketAllocator.alloc();

#if defined(PACKETQUEUE_IMPL_AS_PQLIST) || defined(PACKETQUEUE_IMPL_AS_RING)
  p->in_the_priority_queue = 0;
  p->in_heap = 0;
#endif
  p->m_delivery_time = 0;
  memcpy(&p->m_from, from, sizeof(p->m_from));

  IOBufferBlock *body = new_IOBufferBlock();
  body->alloc(iobuffer_size_to_index(len));
  memcpy(body->end(), buf, len);
  body->fill(len);
  p->append_block(body);

  return p;
}

INK_INLINE UDPPacket *
new_UDPPacket()
{
  UDPPacketInternal *p = udpPacketAllocator.alloc();
  return p;
}

#endif //__P_UDPPPACKET_H_
