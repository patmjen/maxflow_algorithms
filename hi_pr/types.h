#ifndef hi_pr_types_h
#define hi_pr_types_h
/* defs.h */

#ifdef EXCESS_TYPE_LONG
typedef unsigned long excessType;
#else
typedef long long int excessType; /* change to double if not supported */
#endif

typedef unsigned long cType;

typedef  /* arc */
   struct arcSt
{
   cType           resCap;          /* residual capasity */
   struct nodeSt   *head;           /* arc head */
   struct arcSt    *rev;            /* reverse arc */
}
  arc;

typedef  /* node */
   struct nodeSt
{
   arc             *first;           /* first outgoing arc */
   arc             *current;         /* current outgoing arc */
   excessType      excess;           /* excess at the node 
				        change to double if needed */
   long            d;                /* distance label */
   struct nodeSt   *bNext;           /* next node in bucket */
   struct nodeSt   *bPrev;           /* previous node in bucket */
} node;


typedef /* bucket */
   struct bucketSt
{
  node             *firstActive;      /* first node with positive excess */
  node             *firstInactive;    /* first node with zero excess */
} bucket;

#endif
