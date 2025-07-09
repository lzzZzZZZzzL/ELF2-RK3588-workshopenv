#ifndef __CONFI_H__
#define __CONFI_H__

//Used for wolfssl

#ifdef WOLFSSL_NO_SOCK
  typedef enum {
    EWOULDBLOCK,
    EAGAIN,
    ECONNRESET,
    EINTR,
    EPIPE,
    ECONNREFUSED,
    ECONNABORTED
  }SOCK_ERR_CODE;

  extern int errno;
#endif

#endif //__CONFI_H__
