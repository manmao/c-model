#ifndef __ERROR_FUNCTIONS_H_
#define __ERROR_FUNCTIONS_H_



#define min(m,n) ((m) < (n)?(m):(n))
#define max(m,n) ((m) > (n)?(m):(n))



//
void errMsg(const char *format,...);

#ifdef __GNUC__
#define  NORETURN __attribute__((__noreturn__))
#else
#define  NORETURN
#endif

void errExit(const char *format, ...) NORETURN;
void err_exit(const char *format, ...) NORETURN;
void errExitEN(int errnum,const char *format,...) NORETURN;
void fatal(const char *format,...) NORETURN;
void usageErr(const char *format,...) NORETURN;
void cmdLineErr(const char *format,...) NORETURN;

#endif