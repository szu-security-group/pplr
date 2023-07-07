/*
 *   Elliptic Curve Digital Signature Algorithm (ECDSA)
 *
 *
 *   This program verifies the signature given to a <file> in
 *   <file>.ecs generated by program ecsign
 * 
 *   The curve is y^2=x^3+Ax+B mod p
 *
 *   The file common.ecs is presumed to exist, and to contain the domain
 *   information {p,A,B,q,x,y}, where A and B are curve parameters, (x,y) are
 *   a point of order q, p is the prime modulus, and q is the order of the 
 *   point (x,y). In fact normally q is the prime number of points counted
 *   on the curve. 
 *
 */

#include <stdio.h>
#include "miracl.h"
#include <stdlib.h>
#include <string.h>

#ifdef MR_COUNT_OPS
int fpm2,fpi2,fpc,fpa,fpx;
#endif

void strip(char *name)
{ /* strip off filename extension */
    int i;
    for (i=0;name[i]!='\0';i++)
    {
        if (name[i]!='.') continue;
        name[i]='\0';
        break;
    }
}

static void hashing(FILE *fp,big hash)
{ /* compute hash function */
    char h[20];
    sha sh;
    int ch;
    shs_init(&sh);
    while ((ch=fgetc(fp))!=EOF) shs_process(&sh,ch);
    shs_hash(&sh,h);
    bytes_to_big(20,h,hash);
}

int main()
{
    FILE *fp;
    int bits,ep;
    epoint *g,*publc;
    char ifname[50],ofname[50];
    big a,b,p,q,x,y,v,u1,u2,r,s,hash;
    miracl *mip;

/* get public data */
#ifndef MR_EDWARDS	
    fp=fopen("common.ecs","rt");
    if (fp==NULL)
    {
        printf("file common.ecs does not exist\n");
        return 0;
    }
    fscanf(fp,"%d\n",&bits); 
#else
    fp=fopen("edwards.ecs","rt");
    if (fp==NULL)
    {
        printf("file edwards.ecs does not exist\n");
        return 0;
    }
    fscanf(fp,"%d\n",&bits); 
#endif

    mip=mirsys(bits/4,16);   /* Use Hex Internally */
    a=mirvar(0);
    b=mirvar(0);
    p=mirvar(0);
    q=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);
    v=mirvar(0);
    u1=mirvar(0);
    u2=mirvar(0);
    s=mirvar(0);
    r=mirvar(0);
    hash=mirvar(0);

    innum(p,fp);
    innum(a,fp);
    innum(b,fp);
    innum(q,fp);
    innum(x,fp);
    innum(y,fp);
    
    fclose(fp);

    ecurve_init(a,b,p,MR_PROJECTIVE);  /* initialise curve */
    g=epoint_init();
    epoint_set(x,y,0,g); 
    if (!epoint_set(x,y,0,g)) /* initialise point of order q */
    {
        printf("1. Problem - point (x,y) is not on the curve\n");
        exit(0);
    }

/* get public key of signer */
    fp=fopen("public.ecs","rt");
    if (fp==NULL)
    {
        printf("file public.ecs does not exist\n");
        return 0;
    }
    fscanf(fp,"%d",&ep);
    innum(x,fp);
    fclose(fp);

    publc=epoint_init();
    if (!epoint_set(x,x,ep,publc))  /* decompress */
    {
        printf("1. Not a point on the curve\n");
        return 0;
    }

/* get message */
    printf("signed file = ");
    gets(ifname);
    strcpy(ofname,ifname);
    strip(ofname);
    strcat(ofname,".ecs");
    if ((fp=fopen(ifname,"rb"))==NULL)
    { /* no message */
        printf("Unable to open file %s\n",ifname);
        return 0;
    }
    hashing(fp,hash);
    fclose(fp);
    fp=fopen(ofname,"rt");
    if (fp==NULL)
    { /* no signature */
        printf("signature file %s does not exist\n",ofname);
        return 0;
    }
    innum(r,fp);
    innum(s,fp);
    fclose(fp);
    if (compare(r,q)>=0 || compare(s,q)>=0)
    {
        printf("Signature is NOT verified\n");
        return 0;
    }
    xgcd(s,q,s,s,s);
    mad(hash,s,s,q,q,u1);
    mad(r,s,s,q,q,u2);
#ifdef MR_COUNT_OPS
fpm2=fpi2=fpc=fpa=fpx=0;
#endif
    ecurve_mult2(u2,publc,u1,g,g);
#ifdef MR_COUNT_OPS
printf("Number of modmuls= %d, inverses= %d\n",fpc,fpx);
#endif
    epoint_get(g,v,v);
    divide(v,q,q);
    if (compare(v,r)==0) printf("Signature is verified\n");
    else                 printf("Signature is NOT verified\n");
    return 0;
}

