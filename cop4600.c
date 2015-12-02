/*	$OpenBSD: cop4600.c,v 1.00 2003/07/12 01:33:27 dts Exp $	*/

#include <sys/param.h>
#include <sys/acct.h>
#include <sys/systm.h>
#include <sys/ucred.h>
#include <sys/proc.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/malloc.h>
#include <sys/filedesc.h>
#include <sys/pool.h>

#include <sys/mount.h>
#include <sys/syscallargs.h>

/*========================================================================**
**  Dave's example system calls                                           **
**========================================================================*/

/*
** hello() prints to the tty a hello message and returns the process id
*/

int
sys_hello( struct proc *p, void *v, register_t *retval )
{
  uprintf( "\nHello, process %d!\n", p->p_pid );
  *retval = p->p_pid;

  return (0);
}

/*
** showargs() demonstrates passing arguments to the kernel
*/

#define MAX_STR_LENGTH  1024

int
sys_showargs( struct proc *p, void *v, register_t *retval )
{
  /* The arguments are passed in a structure defined as:
  **
  **  struct sys_showargs_args
  **  {
  **      syscallarg(char *) str;
  **      syscallarg(int)    val;
  **  }
  */

  struct sys_showargs_args *uap = v;

  char kstr[MAX_STR_LENGTH+1]; /* will hold kernal-space copy of uap->str */
  int err = 0;
  int size = 0;

  /* copy the user-space arg string to kernal-space */

  err = copyinstr( SCARG(uap, str), &kstr, MAX_STR_LENGTH, &size );
  if (err == EFAULT)
    return( err );

  uprintf( "The argument string is \"%s\"\n", kstr );
  uprintf( "The argument integer is %d\n", SCARG(uap, val) );
  *retval = 0;

  return (0);
}

/*========================================================================**
**  <your name here!>'s COP4600 2004C system calls                        **
**========================================================================*/
#define CIPHER_TEXT_MAX 1025

int sys_cipher (struct proc* p, void* v, register_t* retval) {   
    /*
     * struct sys_cipher_args {
     *  syscallarg(char*) text;
     *  syscallarg(int) lkey;
     *  syscallarg(int) nkey;
     * }
     *
     */


    /* kernel var containers  */
    int err;
    char text[CIPHER_TEXT_MAX];
    int lkey, nkey;


    /*************************/
    /* cipher function vars */ 
    /***********************/
    int textLength;

    /* substitution alg vars */
    int x, offset, lkeyHash, nkeyHash, decryptFlag;

    /* transposition alg vars */
    int lastQuad;

    /* temp vars */
    int i; char temp;

    /*******************/
    /* init user vars */ 
    /*****************/
    struct sys_cipher_args* uap = v;

    lkey = SCARG(uap, lkey);
    nkey = SCARG(uap, nkey);
    err = copyinstr( SCARG(uap, text), &text, CIPHER_TEXT_MAX, &textLength );
    if( err == EFAULT)
        return(err);


    /**********************/
    /* process input text */ 
    /**********************/

    textLength = 0;

    /* calculate text length */
    while(text[textLength] != '\0') {
        ++textLength;
    }
    /* account for ending null terminator */
    textLength += 1;

    /* truncate string */
    if(textLength > CIPHER_TEXT_MAX) {
        textLength = CIPHER_TEXT_MAX;
        text[textLength] = '\0';
    }

    /* textLength is now accurate and text is sanitized */
    /* run encryption algorithms */

    /*****************************/
    /* run substitution on text */
    /*****************************/
    lkeyHash = (lkey % 26) + 26;
    nkeyHash = (nkey % 10) + 10;

    decryptFlag = ( (lkey < 0) && (lkey & 0x1) );

    /* main substitution loop */
    for (i = 0; i < textLength; i++) {
        /* text[i] is upper case letter */
        if(text[i] >= 'A' && text[i] <= 'Z') {
            x = (text[i] - 'A' + lkeyHash) % 26;

            if (decryptFlag)
                offset = ((x-'A') & 0x1) ? 'A':'a';
            else
                offset = ((x-'A') & 0x1) ? 'a':'A';

            text[i] = x + offset;
        }
        /* text[i] is lower case letter */
        else if(text[i] >= 'a' && text[i] <= 'z') {
            x = (text[i] - 'a' + lkeyHash) % 26;
            offset = (decryptFlag) ? 
                ( ( (x - 'A') & 0x1 ) ? 'a' : 'A' ) :
                ( ( (x - 'A') & 0x1 ) ? 'A' : 'a') ;
            text[i] = x + offset;
        }
        /* text[i] is digit */
        else if(text[i] >= '0' && text[i] <= '9') {
            text[i] = ((text[i] - '0' + nkeyHash) % 10) + '0';
        }
        else
            continue;
    } /***** END Substitution ******/
    
    /*****************************/
    /* run transposition on text */
    /*****************************/
    lastQuad = textLength % 4;
    /* process the text up to the last quad */
    for (i = 0; i < (textLength - lastQuad); i+=4) {
        temp = text[i];
        text[i] = text[i+2];
        text[i+2] = temp;

        temp = text[i+1];
        text[i+1] = text[i+3];
        text[i+3] = temp;
    } /* i holds the start of the last quad */

    /* process the last quad if exists */
    if (lastQuad !=0) 
        switch(lastQuad) {
            case 1:
                break;

            case 2:
                temp = text[i];
                text[i] = text[i+1];
                text[i+1] = temp;
                break;
            case 3:
                temp = text[i];
                text[i] = text[i+2];
                text[i+2] = temp;
                break;
        }/***** END Transposition ******/

    /* return textLength to user  */
    *retval = textLength;

    return (0);
}
