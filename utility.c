#include <iconv.h>
#include <stdio.h>
#include <string.h>

#include "utility.h"

int code_convert(char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open("GB2312", "UTF-8");
    if (cd==0)
        return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,&inlen,pout,&outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}

int code_convert_utf8_gbk(char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open("GB2312", "UTF-8");
    if (cd==0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);

    return 0;
}

char* utf8_to_gbk(char *buf)
{
    static char outbuf[512];
    char inbuf[512];
    int inlen, outlen;
    int rc;

    inlen = sizeof(inbuf);
    outlen = sizeof(outbuf);
    memcpy(inbuf, buf, inlen);
    memset(outbuf, 0, outlen);

    rc = code_convert_utf8_gbk(inbuf, inlen, outbuf, outlen);
    if (rc == -1)
        return NULL;

    return outbuf;
}
