#define NULL 0

typedef char *va_list;
#define va_arg(ap, T) (*(T *)(((ap) += sizeof(T)) - sizeof(T)))
#define va_end(ap) ((ap) = (void *)NULL)
#define va_start(ap, parmN) ((ap) = (char *)(&(parmN)) + sizeof(parmN))
#define va_copy(dest, src) ((dest) = (src))