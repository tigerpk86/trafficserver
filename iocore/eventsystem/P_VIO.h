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


#if !defined ( P_VIO_h)
#define  P_VIO_h
#include "I_VIO.h"

INK_INLINE
VIO::VIO(int aop)
  :
_cont(NULL),
nbytes(0),
ndone(0),
op(aop),
data(0),
buffer(),
vc_server(0),
mutex(0)
{
  return;
}

/////////////////////////////////////////////////////////////
//
//  VIO::VIO()
//
/////////////////////////////////////////////////////////////
INK_INLINE
VIO::VIO()
  :
_cont(0),
nbytes(0),
ndone(0),
op(VIO::NONE),
data(0),
buffer(),
vc_server(0),
mutex(0)
{
  return;
}

INK_INLINE void
VIO::set_nbytes(int anbytes)
{
  nbytes = anbytes;
}
INK_INLINE void
VIO::add_nbytes(int anbytes)
{
  nbytes += anbytes;
}
INK_INLINE void
VIO::set_nbytes_internal(int anbytes)
{
  nbytes = anbytes;
}
INK_INLINE void
VIO::add_nbytes_internal(int anbytes)
{
  nbytes += anbytes;
}
INK_INLINE void
VIO::set_ndone(int andone)
{
  ndone = andone;
}
INK_INLINE void
VIO::add_ndone(int andone)
{
  ndone += andone;
}
INK_INLINE void
VIO::set_data(int adata)
{
  data = adata;
}
INK_INLINE void
VIO::set_vc_server(VConnection * avc_server)
{
  vc_server = avc_server;
}
INK_INLINE Continuation *
VIO::get_continuation()
{
  return _cont;
}
INK_INLINE int
VIO::get_nbytes()
{
  return nbytes;
}
INK_INLINE int
VIO::get_ndone()
{
  return ndone;
}
INK_INLINE int
VIO::get_data()
{
  return data;
}
INK_INLINE void
VIO::set_op(int aop)
{
  op = aop;
}
INK_INLINE int
VIO::get_op()
{
  return op;
}
INK_INLINE ProxyMutex *
VIO::get_mutex()
{
  return (mutex);
}
INK_INLINE void
VIO::set_writer(MIOBuffer * writer)
{
  buffer.writer_for(writer);
}
INK_INLINE void
VIO::set_reader(IOBufferReader * reader)
{
  buffer.reader_for(reader);
}
INK_INLINE MIOBuffer *
VIO::get_writer()
{
  return buffer.writer();
}
INK_INLINE IOBufferReader *
VIO::get_reader()
{
  return (buffer.reader());
}
INK_INLINE int
VIO::ntodo()
{
  return nbytes - ndone;
}
INK_INLINE void
VIO::done()
{
  if (buffer.reader())
    set_nbytes(ndone + buffer.reader()->read_avail());
  else
    set_nbytes(ndone);
}
INK_INLINE int
VIO::get_ntodo()
{
  return (nbytes - ndone);
}
INK_INLINE VConnection *
VIO::get_vc_server()
{
  return (vc_server);
}

/////////////////////////////////////////////////////////////
//
//  VIO::set_continuation()
//
/////////////////////////////////////////////////////////////
INK_INLINE void
VIO::set_continuation(Continuation * acont)
{
  if (vc_server)
    vc_server->set_continuation(this, acont);
  if (acont) {
    mutex = acont->mutex;
    _cont = acont;
  } else {
    mutex = NULL;
    _cont = NULL;
  }
  return;
}

/////////////////////////////////////////////////////////////
//
//  VIO::reenable()
//
/////////////////////////////////////////////////////////////
INK_INLINE void
VIO::reenable()
{
  if (vc_server)
    vc_server->reenable(this);
}

/////////////////////////////////////////////////////////////
//
//  VIO::reenable_re()
//
/////////////////////////////////////////////////////////////
INK_INLINE void
VIO::reenable_re()
{
  if (vc_server)
    vc_server->reenable_re(this);
}

#endif /* #if !defined ( P_VIO_h) */
