//**************************************************************************
//*	Name:			linuxerrors.c
//*
//*	Author:			Mark Sproul
//*					msproul@skychariot.com
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul msproul@skychariot.com
//*****************************************************************************
//*	Mar 17,	2021	<MLS> Created linuxerrors.c
//*****************************************************************************

#include	<errno.h>
#include	<string.h>
#include	<stdio.h>

#include	"linuxerrors.h"

//*****************************************************************************
void	GetLinuxErrorString(const int errNum, char *errorString)
{

//       ELOOP  Too many symbolic links were encountered in resolving pathname.
//       ENAMETOOLONG pathname was too long.


	switch(errNum)
	{
		case EPERM:		strcpy(errorString, "(1) Operation not permitted");		break;
		case ENOENT:	strcpy(errorString, "(2) No such file or directory");	break;
		case ESRCH:		strcpy(errorString, "(3) No such process");				break;
		case EINTR:		strcpy(errorString, "(4) Interrupted system call");		break;
		case EIO:		strcpy(errorString, "(5) I/O error");					break;
		case ENXIO:		strcpy(errorString, "(6) No such device or address");	break;
		case E2BIG:		strcpy(errorString, "(7) Argument list too long");		break;
		case ENOEXEC:	strcpy(errorString, "(8) Exec format error");			break;
		case EBADF:		strcpy(errorString, "(9) Bad file number");				break;
		case ECHILD:	strcpy(errorString, "(10) No child processes");			break;
		case EAGAIN:	strcpy(errorString, "(11) Try again");					break;
		case ENOMEM:	strcpy(errorString, "(12) Out of memory");				break;
		case EACCES:	strcpy(errorString, "(13) Permission denied");			break;
		case EFAULT:	strcpy(errorString, "(14) Bad address");				break;
		case ENOTBLK:	strcpy(errorString, "(15) Block device required");		break;
		case EBUSY:		strcpy(errorString, "(16) Device or resource busy");	break;
		case EEXIST:	strcpy(errorString, "(17) File exists");				break;
		case EXDEV:		strcpy(errorString, "(18) Cross-device link");			break;
		case ENODEV:	strcpy(errorString, "(19) No such device");				break;
		case ENOTDIR:	strcpy(errorString, "(20) Not a directory");			break;
		case EISDIR:	strcpy(errorString, "(21) Is a directory");				break;
		case EINVAL:	strcpy(errorString, "(22) Invalid argument");			break;
		case ENFILE:	strcpy(errorString, "(23) File table overflow");		break;
		case EMFILE:	strcpy(errorString, "(24) Too many open files");		break;
		case ENOTTY:	strcpy(errorString, "(25) Not a typewriter");			break;
		case ETXTBSY:	strcpy(errorString, "(26) Text file busy");				break;
		case EFBIG:		strcpy(errorString, "(27) File too large");				break;
		case ENOSPC:	strcpy(errorString, "(28) No space left on device");	break;
		case ESPIPE:	strcpy(errorString, "(29) Illegal seek");				break;
		case EROFS:		strcpy(errorString, "(30) Read-only file system");		break;
		case EMLINK:	strcpy(errorString, "(31) Too many links");				break;
		case EPIPE:		strcpy(errorString, "(32) Broken pipe");				break;
		case EDOM:		strcpy(errorString, "(33) Math argument out of domain of func");	break;
		case ERANGE:	strcpy(errorString, "(34) Math result not representable");break;

//define	ENOSYS		38	/* Invalid system call number */

//define	ENOTEMPTY	39	/* Directory not empty */
//define	ELOOP		40	/* Too many symbolic links encountered */
//define	EWOULDBLOCK	EAGAIN	/* Operation would block */
//define	ENOMSG		42	/* No message of desired type */
//define	EIDRM		43	/* Identifier removed */
//define	ECHRNG		44	/* Channel number out of range */
//define	EL2NSYNC	45	/* Level 2 not synchronized */
//define	EL3HLT		46	/* Level 3 halted */
//define	EL3RST		47	/* Level 3 reset */
//define	ELNRNG		48	/* Link number out of range */
//define	EUNATCH		49	/* Protocol driver not attached */
//define	ENOCSI		50	/* No CSI structure available */
//define	EL2HLT		51	/* Level 2 halted */
//define	EBADE		52	/* Invalid exchange */
//define	EBADR		53	/* Invalid request descriptor */
//define	EXFULL		54	/* Exchange full */
//define	ENOANO		55	/* No anode */
//define	EBADRQC		56	/* Invalid request code */
//define	EBADSLT		57	/* Invalid slot */

//define	EDEADLOCK	EDEADLK

//define	EBFONT		59	/* Bad font file format */
//define	ENOSTR		60	/* Device not a stream */
//define	ENODATA		61	/* No data available */
//define	ETIME		62	/* Timer expired */
//define	ENOSR		63	/* Out of streams resources */
//define	ENONET		64	/* Machine is not on the network */
//define	ENOPKG		65	/* Package not installed */
//define	EREMOTE		66	/* Object is remote */
//define	ENOLINK		67	/* Link has been severed */
//define	EADV		68	/* Advertise error */
//define	ESRMNT		69	/* Srmount error */
//define	ECOMM		70	/* Communication error on send */
//define	EPROTO		71	/* Protocol error */
//define	EMULTIHOP	72	/* Multihop attempted */
//define	EDOTDOT		73	/* RFS specific error */
//define	EBADMSG		74	/* Not a data message */
//define	EOVERFLOW	75	/* Value too large for defined data type */
//define	ENOTUNIQ	76	/* Name not unique on network */
//define	EBADFD		77	/* File descriptor in bad state */
//define	EREMCHG		78	/* Remote address changed */
//define	ELIBACC		79	/* Can not access a needed shared library */
//define	ELIBBAD		80	/* Accessing a corrupted shared library */
//define	ELIBSCN		81	/* .lib section in a.out corrupted */
//define	ELIBMAX		82	/* Attempting to link in too many shared libraries */
//define	ELIBEXEC	83	/* Cannot exec a shared library directly */
//define	EILSEQ		84	/* Illegal byte sequence */
//define	ERESTART	85	/* Interrupted system call should be restarted */
//define	ESTRPIPE	86	/* Streams pipe error */
//define	EUSERS		87	/* Too many users */
//define	ENOTSOCK	88	/* Socket operation on non-socket */
//define	EDESTADDRREQ	89	/* Destination address required */
//define	EMSGSIZE	90	/* Message too long */
//define	EPROTOTYPE	91	/* Protocol wrong type for socket */
//define	ENOPROTOOPT	92	/* Protocol not available */
//define	EPROTONOSUPPORT	93	/* Protocol not supported */
//define	ESOCKTNOSUPPORT	94	/* Socket type not supported */
//define	EOPNOTSUPP	95	/* Operation not supported on transport endpoint */
//define	EPFNOSUPPORT	96	/* Protocol family not supported */
//define	EAFNOSUPPORT	97	/* Address family not supported by protocol */
		case EADDRINUSE:	strcpy(errorString, "(98) Address already in use");				break;
		case EADDRNOTAVAIL:	strcpy(errorString, "(99) Cannot assign requested address");	break;
		case ENETDOWN:		strcpy(errorString, "(100) Network is down");					break;
		case ENETUNREACH:	strcpy(errorString, "(101) Network is unreachable");			break;
		case ENETRESET:		strcpy(errorString, "(102) Network dropped connection because of reset");	break;
		case ECONNABORTED:	strcpy(errorString, "(103) Software caused connection abort");	break;
		case ECONNRESET:	strcpy(errorString, "(104) Connection reset by peer");			break;
		case ENOBUFS:		strcpy(errorString, "(105) No buffer space available");			break;
//define	EISCONN		106	/* Transport endpoint is already connected */
//define	ENOTCONN	107	/* Transport endpoint is not connected */
//define	ESHUTDOWN	108	/* Cannot send after transport endpoint shutdown */
//define	ETOOMANYREFS	109	/* Too many references: cannot splice */
		case ETIMEDOUT:		strcpy(errorString, "(110) Connection timed out");	break;
		case ECONNREFUSED:	strcpy(errorString, "(111) Connection refused");	break;
		case EHOSTDOWN:		strcpy(errorString, "(112) Host is down");			break;
		case EHOSTUNREACH:	strcpy(errorString, "(113) No route to host");		break;
//define	EALREADY	114	/* Operation already in progress */
//define	EINPROGRESS	115	/* Operation now in progress */
//define	ESTALE		116	/* Stale file handle */
//define	EUCLEAN		117	/* Structure needs cleaning */
//define	ENOTNAM		118	/* Not a XENIX named type file */
//define	ENAVAIL		119	/* No XENIX semaphores available */
//define	EISNAM		120	/* Is a named type file */
//define	EREMOTEIO	121	/* Remote I/O error */
		case EDQUOT:		strcpy(errorString, "(122) Quota exceeded");		break;

//define	ENOMEDIUM	123	/* No medium found */
//define	EMEDIUMTYPE	124	/* Wrong medium type */
//define	ECANCELED	125	/* Operation Canceled */
//define	ENOKEY		126	/* Required key not available */
//define	EKEYEXPIRED	127	/* Key has expired */
//define	EKEYREVOKED	128	/* Key has been revoked */
//define	EKEYREJECTED	129	/* Key was rejected by service */

/* for robust mutexes */
//define	EOWNERDEAD	130	/* Owner died */
//define	ENOTRECOVERABLE	131	/* State not recoverable */

//define ERFKILL		132	/* Operation not possible due to RF-kill */

//define EHWPOISON	133	/* Memory page has hardware error */

		default:	sprintf(errorString, "Error #%d not in table", errNum);	break;

	}

}
