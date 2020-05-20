#ifndef _ERRNO_H
#define _ERRNO_H

extern volatile int errno;

/**
 * This file contains all the linux error codes 
 */
#define ERROR 			   0 	 // General error
#define EPERM              1     // Operation not permitted
#define ENOENT             2     // No such file or directory
#define ESRCH              3     // No such process
#define EINTR              4     // Interrupted system call
#define EIO                5     // I/O error
#define ENXIO              6     // No such device or address
#define E2BIG              7     // Argument list too long
#define ENOEXEC            8     // Exec format error
#define EBADF              9     // Bad file number
#define ECHILD            10     // No child processes
#define EAGAIN            11     // Try again
#define ENOMEM            12     // Out of memory
#define EACCES            13     // Permission denied
#define EFAULT            14     // Bad address
#define ENOTBLK           15     // Block device required
#define EBUSY             16     // Device or resource busy
#define EEXIST            17     // File exists
#define EXDEV             18     // Cross-device link
#define ENODEV            19     // No such device
#define ENOTDIR           20     // Not a directory
#define EISDIR            21     // Is a directory
#define EINVAL            22     // Invalid argument
#define ENFILE            23     // File table overflow
#define EMFILE            24     // Too many open files
#define ENOTTY            25     // Not a typewriter
#define ETXTBSY           26     // Text file busy
#define EFBIG             27     // File too large
#define ENOSPC            28     // No space left on device
#define ESPIPE            29     // Illegal seek
#define EROFS             30     // Read-only file system
#define EMLINK            31     // Too many links
#define EPIPE             32     // Broken pipe
#define EDOM              33     // Math argument out of domain of func
#define ERANGE            34     // Math result not representable
#define EDEADLK           35     // Resource deadlock would occur
#define ENAMETOOLONG      36     // File name too long
#define ENOLCK            37     // No record locks available
#define ENOSYS            38     // Function not implemented
#define ENOTEMPTY         39     // Directory not empty
#define ELOOP             40     // Too many symbolic links encountered
#define ENOMSG            42     // No message of desired type
#define EIDRM             43     // Identifier removed
#define ECHRNG            44     // Channel number out of range
#define EL2NSYNC          45     // Level 2 not synchronized
#define EL3HLT            46     // Level 3 halted
#define EL3RST            47     // Level 3 reset
#define ELNRNG            48     // Link number out of range
#define EUNATCH           49     // Protocol driver not attached
#define ENOCSI            50     // No CSI structure available
#define EL2HLT            51     // Level 2 halted
#define EBADE             52     // Invalid exchange
#define EBADR             53     // Invalid request descriptor
#define EXFULL            54     // Exchange full
#define ENOANO            55     // No anode
#define EBADRQC           56     // Invalid request code
#define EBADSLT           57     // Invalid slot
#define EBFONT            59     // Bad font file format
#define ENOSTR            60     // Device not a stream
#define ENODATA           61     // No data available
#define ETIME             62     // Timer expired
#define ENOSR             63     // Out of streams resources
#define ENONET            64     // Machine is not on the network
#define ENOPKG            65     // Package not installed
#define EREMOTE           66     // Object is remote
#define ENOLINK           67     // Link has been severed
#define EADV              68     // Advertise error
#define ESRMNT            69     // Srmount error
#define ECOMM             70     // Communication error on send
#define EPROTO            71     // Protocol error
#define EMULTIHOP         72     // Multihop attempted
#define EDOTDOT           73     // RFS specific error
#define EBADMSG           74     // Not a data message
#define EOVERFLOW         75     // Value too large for defined data type
#define ENOTUNIQ          76     // Name not unique on network
#define EBADFD            77     // File descriptor in bad state
#define EREMCHG           78     // Remote address changed
#define ELIBACC           79     // Can not access a needed shared library
#define ELIBBAD           80     // Accessing a corrupted shared library
#define ELIBSCN           81     // .lib section in a.out corrupted
#define ELIBMAX           82     // Attempting to link in too many shared libraries
#define ELIBEXEC          83     // Cannot exec a shared library directly
#define EILSEQ            84     // Illegal byte sequence
#define ERESTART          85     // Interrupted system call should be restarted
#define ESTRPIPE          86     // Streams pipe error
#define EUSERS            87     // Too many users
#define ENOTSOCK          88     // Socket operation on non-socket
#define EDESTADDRREQ      89     // Destination address required
#define EMSGSIZE          90     // Message too long
#define EPROTOTYPE        91     // Protocol wrong type for socket
#define ENOPROTOOPT       92     // Protocol not available
#define EPROTONOSUPPORT   93     // Protocol not supported
#define ESOCKTNOSUPPORT   94     // Socket type not supported
#define EOPNOTSUPP        95     // Operation not supported on transport endpoint
#define EPFNOSUPPORT      96     // Protocol family not supported
#define EAFNOSUPPORT      97     // Address family not supported by protocol
#define EADDRINUSE        98     // Address already in use
#define EADDRNOTAVAIL     99     // Cannot assign requested address
#define ENETDOWN         100     // Network is down
#define ENETUNREACH      101     // Network is unreachable
#define ENETRESET        102     // Network dropped connection because of reset
#define ECONNABORTED     103     // Software caused connection abort
#define ECONNRESET       104     // Connection reset by peer
#define ENOBUFS          105     // No buffer space available
#define EISCONN          106     // Transport endpoint is already connected
#define ENOTCONN         107     // Transport endpoint is not connected
#define ESHUTDOWN        108     // Cannot send after transport endpoint shutdown
#define ETOOMANYREFS     109     // Too many references: cannot splice
#define ETIMEDOUT        110     // Connection timed out
#define ECONNREFUSED     111     // Connection refused
#define EHOSTDOWN        112     // Host is down
#define EHOSTUNREACH     113     // No route to host
#define EALREADY         114     // Operation already in progress
#define EINPROGRESS      115     // Operation now in progress
#define ESTALE           116     // Stale NFS file handle
#define EUCLEAN          117     // Structure needs cleaning
#define ENOTNAM          118     // Not a XENIX named type file
#define ENAVAIL          119     // No XENIX semaphores available
#define EISNAM           120     // Is a named type file
#define EREMOTEIO        121     // Remote I/O error
#define EDQUOT           122     // Quota exceeded
#define ENOMEDIUM        123     // No medium found
#define EMEDIUMTYPE      124     // Wrong medium type
#define ECANCELED        125     // Operation Canceled
#define ENOKEY           126     // Required key not available
#define EKEYEXPIRED      127     // Key has expired
#define EKEYREVOKED      128     // Key has been revoked
#define EKEYREJECTED     129     // Key was rejected by service
#define EOWNERDEAD       130     // Owner died
#define ENOTRECOVERABLE  131     // State not recoverable

static const char *errno_name[] =
{
	"EPERM",
	"ENOENT",
	"ESRCH",
	"EINTR",
	"EIO",
	"ENXIO",
	"E2BIG",
	"ENOEXEC",
	"EBADF",
	"ECHILD",
	"EAGAIN",
	"ENOMEM",
	"EACCES",
	"EFAULT",
	"ENOTBLK",
	"EBUSY",
	"EEXIST",
	"EXDEV",
	"ENODEV",
	"ENOTDIR",
	"EISDIR",
	"EINVAL",
	"ENFILE",
	"EMFILE",
	"ENOTTY",
	"ETXTBSY",
	"EFBIG",
	"ENOSPC",
	"ESPIPE",
	"EROFS",
	"EMLINK",
	"EPIPE",
	"EDOM",
	"ERANGE",
	"EDEADLK",
	"ENAMETOOLONG",
	"ENOLCK",
	"ENOSYS",
	"ENOTEMPTY",
	"ELOOP",
	"ENOMSG",
	"EIDRM",
	"ECHRNG",
	"EL2NSYNC",
	"EL3HLT",
	"EL3RST",
	"ELNRNG",
	"EUNATCH",
	"ENOCSI",
	"EL2HLT",
	"EBADE",
	"EBADR",
	"EXFULL",
	"ENOANO",
	"EBADRQC",
	"EBADSLT",
	"EBFONT",
	"ENOSTR",
	"ENODATA",
	"ETIME",
	"ENOSR",
	"ENONET",
	"ENOPKG",
	"EREMOTE",
	"ENOLINK",
	"EADV",
	"ESRMNT",
	"ECOMM",
	"EPROTO",
	"EMULTIHOP",
	"EDOTDOT",
	"EBADMSG",
	"EOVERFLOW",
	"ENOTUNIQ",
	"EBADFD",
	"EREMCHG",
	"ELIBACC",
	"ELIBBAD",
	"ELIBSCN",
	"ELIBMAX",
	"ELIBEXEC",
	"EILSEQ",
	"ERESTART",
	"ESTRPIPE",
	"EUSERS",
	"ENOTSOCK",
	"EDESTADDRREQ",
	"EMSGSIZE",
	"EPROTOTYPE",
	"ENOPROTOOPT",
	"EPROTONOSUPPORT",
	"ESOCKTNOSUPPORT",
	"EOPNOTSUPP",
	"EPFNOSUPPORT",
	"EAFNOSUPPORT",
	"EADDRINUSE",
	"EADDRNOTAVAIL",
	"ENETDOWN",
	"ENETUNREACH",
	"ENETRESET",
	"ECONNABORTED",
	"ECONNRESET",
	"ENOBUFS",
	"EISCONN",
	"ENOTCONN",
	"ESHUTDOWN",
	"ETOOMANYREFS",
	"ETIMEDOUT",
	"ECONNREFUSED",
	"EHOSTDOWN",
	"EHOSTUNREACH",
	"EALREADY",
	"EINPROGRESS",
	"ESTALE",
	"EUCLEAN",
	"ENOTNAM",
	"ENAVAIL",
	"EISNAM",
	"EREMOTEIO",
	"EDQUOT",
	"ENOMEDIUM",
	"EMEDIUMTYPE",
	"ECANCELED",
	"ENOKEY",
	"EKEYEXPIRED",
	"EKEYREVOKED",
	"EKEYREJECTED",
	"EOWNERDEAD",
	"ENOTRECOVERABLE"
};

static const char *errno_explanation[] = 
{
	"Operation not permitted",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Argument list too long",
	"Exec format error",
	"Bad file number",
	"No child processes",
	"Try again",
	"Out of memory",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device or resource busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Math argument out of domain of func",
	"Math result not representable",
	"Resource deadlock would occur",
	"File name too long",
	"No record locks available",
	"Function not implemented",
	"Directory not empty",
	"Too many symbolic links encountered",
	"No message of desired type",
	"Identifier removed",
	"Channel number out of range",
	"Level 2 not synchronized",
	"Level 3 halted",
	"Level 3 reset",
	"Link number out of range",
	"Protocol driver not attached",
	"No CSI structure available",
	"Level 2 halted",
	"Invalid exchange",
	"Invalid request descriptor",
	"Exchange full",
	"No anode",
	"Invalid request code",
	"Invalid slot",
	"Bad font file format",
	"Device not a stream",
	"No data available",
	"Timer expired",
	"Out of streams resources",
	"Machine is not on the network",
	"Package not installed",
	"Object is remote",
	"Link has been severed",
	"Advertise error",
	"Srmount error",
	"Communication error on send",
	"Protocol error",
	"Multihop attempted",
	"RFS specific error",
	"Not a data message",
	"Value too large for defined data type",
	"Name not unique on network",
	"File descriptor in bad state",
	"Remote address changed",
	"Can not access a needed shared library",
	"Accessing a corrupted shared library",
	".lib section in a.out corrupted",
	"Attempting to link in too many shared libraries",
	"Cannot exec a shared library directly",
	"Illegal byte sequence",
	"Interrupted system call should be restarted",
	"Streams pipe error",
	"Too many users",
	"Socket operation on non-socket",
	"Destination address required",
	"Message too long",
	"Protocol wrong type for socket",
	"Protocol not available",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported on transport endpoint",
	"Protocol family not supported",
	"Address family not supported by protocol",
	"Address already in use",
	"Cannot assign requested address",
	"Network is down",
	"Network is unreachable",
	"Network dropped connection because of reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"No buffer space available",
	"Transport endpoint is already connected",
	"Transport endpoint is not connected",
	"Cannot send after transport endpoint shutdown",
	"Too many references: cannot splice",
	"Connection timed out",
	"Connection refused",
	"Host is down",
	"No route to host",
	"Operation already in progress",
	"Operation now in progress",
	"Stale NFS file handle",
	"Structure needs cleaning",
	"Not a XENIX named type file",
	"No XENIX semaphores available",
	"Is a named type file",
	"Remote I/O error",
	"Quota exceeded",
	"No medium found",
	"Wrong medium type",
	"Operation Canceled",
	"Required key not available",
	"Key has expired",
	"Key has been revoked",
	"Key was rejected by service",
	"Owner died",
	"State not recoverable"
	
};

#endif