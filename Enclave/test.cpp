extern "C" {
#include "Enclave_t.h"
#include <stdarg.h>
#include "sgx_includes.h"
#include "sgx_funcs.h"
#include <string.h>
#include <stdio.h>



    size_t
    fwrite(void *buffer, size_t size, size_t cont, FILE *fd)
    {
        size_t res;
        ocall_fwrite(&res, buffer, size, cont, fd);
        return res;
    }

    int
    fprintf(FILE *file, const char* fmt, ...)
    {
        int res;
        res = 0;
        #define BUFSIZE 20000
        char buf[BUFSIZE] = {'\0'};
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, BUFSIZE, fmt, ap);
        va_end(ap);
        res+= (int)fwrite(buf, 1, strlen(buf), file);
        return res;   
    }


    int
    printf(const char* fmt, ...)
    {
        int res;
        res = 0;
        #define BUFSIZE 20000
        char buf[BUFSIZE] = {'\0'};
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, BUFSIZE, fmt, ap);
        va_end(ap);
        res+= (int)fwrite(buf, 1, strlen(buf), stdout);
        return res;   
    }

    int
    putchar(int a)
    {
        return printf("%c", a);
    }

    int fputc(int c, FILE *stream) {
        return fprintf(stream, "%c", c);

    }

    void ni(const char *s) {
        fprintf(stdout, "Not implemented %s\n", s);
        abort();
    }

    int sprintf(char *str, const char *format, ...) {
        ni(__FUNCTION__);
        return -1;
    }

    int 
    nanosleep(const struct timespec *req, struct timespec *rem)
    {
        ni(__FUNCTION__);
        return 1;
    }

    void *
    dlopen(const char *fname, int flags) 
    { 
        void *p;
        ocall_dlopen(&p, fname, flags);
        return p;
    }

    int 
    dlclose(void *handle) 
    {   
        int a;
        ocall_dlclose(&a, handle);
        return a;

    }
    char *
    fgets(char *str, int n, FILE *fd)
    {
        char *result;
        ocall_fgets(&result, str, n, fd);
        return result;
    }

    char *
    getenv(const char *name)
    {
        char *res;
        ocall_getenv(&res, name);
        return res;
    }
    struct lconv *
    localeconv(void)
    {
        lconv *res;
        ocall_localeconv(&res);
        return res;
    }

    int
    fputs(const char *str, FILE *stream)
    {
        int res;
        ocall_fputs(&res, str, stream); 
        return res;
    }

    FILE *
    fopen(const char *filename, const char *mode)
    {
        FILE *fp;
        char *l = malloc(strlen(filename) + 1);
        memset(l, 0, strlen(filename) + 1);
        memcpy(l, filename, strlen(filename));
        ocall_fopen(&fp, l, strlen(filename), mode);
        printf("PAW BA DIAVASW %p |%s| |%s|\n", fp, filename, mode);
        free(l);
        return fp;
    }

    void
    exit(int status_)
    {
        ocall_exit(status_);
    }

    char *
    setlocale(int category, const char *locale)
    {
        char *a;
        ocall_setlocale(&a, category, locale);
        return a;
    }

    int
    system(const char *str)
    {
        int a;
        ocall_system(&a, str);
        return a;
    }

    FILE *
    popen(const char *command, const char *type)
    {
        FILE *a = NULL;
        ocall_popen(&a, command, type);
        return a;
    }

    int 
    pclose(FILE *stream)
    {
        int a;
        return ocall_pclose(&a, stream);

    }

    long int
    mktime(void *timeptr)
    {
        long int a = 0;
        ni(__FUNCTION__);
        //ocall_mktime(&a, timeptr);
        return a;
    }

    int 
    remove(const char *filename)
    {
        int a;
        ocall_remove(&a, filename);
        return a;
    }

    struct tm *
    localtime(const long int *timer)
    {
        struct tm *a = NULL;
        ni(__FUNCTION__);
        //`ocall_localtime(&a, timer);
        return a;
    }

    struct tm *
    gmtime(const long int *timer)
    {
        struct tm *a = NULL;
        ni(__FUNCTION__);
        //ocall_gmtime(&a, timer);
        return a;
    }

    int
    rename(const char *old_filename, const char *new_filename)
    {
        int a;
        a = 0;
        ocall_rename(&a, old_filename, new_filename);
        return a;
    }

    char *
    tmpnam(char *str)
    {
        char *a;
        a = NULL;
        ocall_tmpnam(&a, str);
        return a;
    }

    int
    fclose(FILE *ptr)
    {
        int a;
        a = 0;
        ocall_fclose(&a, ptr);
        return a;
    }

    int
    setvbuf(FILE *stream, char *buffer, int mode, size_t size)
    {
        int a;
        a = 0;
        ocall_setvbuf(&a, stream, buffer, mode, size);
        return a;
    }

    long int
    ftell(FILE *stream)
    {
        long int a;
        a = 0;
        ocall_ftell(&a, stream);
        return a;
    }

    int
    fseek(FILE *stream, long int offset, int whence)
    {
        int a;
        a = 0;
        ocall_fseek(&a, stream, offset, whence);
        return a;
    }

    FILE *
    tmpfile()
    {
        FILE *fd;
        fd = NULL;
        ocall_tmpfile(&fd);
        return fd;
    }

    void
    clearerr(FILE *stream)
    {
        ocall_clearerr(stream);
    }

    size_t
    fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t a;
        a = 0;
        /* 
         * lua parses the first char on the file for some weird reason
         * and the the rest
         */
        ocall_fread(&a, ptr, size, nmemb, stream);
        ((char *)ptr)[a] = '\0';
        return a;
    }

    int
    ferror(FILE *stream)
    {
        int a;
        a = 0;
        ocall_ferror(&a, stream);
        return a;
    }

    char *rec_filename(FILE *f);
    int counter = -1;
    size_t len = 0;
    char *buffer = NULL;

    int
    getc(FILE *stream)
    {
        abort();
        int a;
        ocall_getc(&a, stream);
        return a;
    }

    int
    ungetc(int ch, FILE *stream)
    {
        int a;
        a = 0;
        ocall_ungetc(&a, ch, stream);
        return a;
    }

    FILE *
    freopen(const char *filename, const char *mode, FILE *stream)
    {
        FILE *fd;
        fd = NULL;
        ocall_freopen(&fd, filename, mode, stream);
        return fd;
    }

    int
    feof(FILE *stream)
    {
        int a;
        a = 0;
        ocall_feof(&a, stream);
        return a;
    }

    int
    rand(void)
    {
        int a;
        /* sgx_read_rand((unsigned char *)&a, sizeof(a));
        if (a < 0)
            a *= -1; 
        */
        ocall_rand(&a);
        //fprintf(stdout, "output = %u\n", a);
        return a;   
    }

    long int
    time(long int *src)
    {
        long int a;
        a = 0;
        ni(__FUNCTION__);
        //ocall_time(&a, src);
        return a;
    }

    void
    srand(unsigned int seed)
    {   
        ocall_srand(seed);
    }

    long int
    clock()
    {
        long int a;
        a = 0;
        ocall_clock(&a);
        return a;
    }

    void
    print_num(size_t s)
    {
        ocall_print_num(s);
    }

    int
    fflush(FILE *ptr)
    {
        int res;
        res = 0;
        return ocall_fflush(&res, ptr);
    }

    long int
    labs (long int i)
    {
        return i < 0 ? -i : i;
    }

    int
    puts(const char *s)
    {
        int a;
        a = fprintf(stdout, "%s", s);
        return a;
    }


    int
    putc(int a, FILE *f)
    {
        return fprintf(f, "%d", a);
    }

    void 
    perror(const char *p){
        fprintf(stdout, "%s", p);
    }

	int 
	strcmp(const char* s1, const char* s2)
	{
		while(*s1 && (*s1 == *s2))
		{
			s1++;
			s2++;
		}
    	return *(const unsigned char*)s1 - *(const unsigned char*)s2;
	}

    char *
    strcpy(char *strDest, const char *strSrc)
    {
        char *temp = strDest;
        while(*strDest++ = *strSrc++); // or while((*strDest++=*strSrc++) != '\0');
        return temp;
    }

    char *
    strcat(char *dest, const char *src)
    {
        char *rdest = dest;

        while (*dest)
          dest++;
        while (*dest++ = *src++)
          ;
        return rdest;
    }

    char *
    strdup(char *src)
    {
        char *str;
        size_t len = strlen(src);
        str = (char *)malloc(len + 1);
        if (str) {
            memcpy(str, src, len + 1);
        }
        return str;
    }

    int 
    memcmp(const void *s1, const void *s2, size_t n)
    {
		size_t i;  
		const unsigned char * cs = (const unsigned char*) s1;
		const unsigned char * ct = (const unsigned char*) s2;

		for (i = 0; i < n; i++, cs++, ct++) {
		if (*cs < *ct)
			return -1;
		else if (*cs > *ct)
			return 1;
		}
		return 0;
	}

    int 
    __isnan(double x) 
    {
        return x != x;
    }
    int isinf(double x) { return !__isnan(x) && __isnan(x - x); }


    int clock_gettime(clockid_t clk_id, struct timespec *tp) {
        fprintf(stdout,"Not implemented clock_gettime \n");
        return 0;
    }

    int gettimeofday(struct timeval *tv, struct timezone *tz) {
        return 0;
    };

    int waitpid(int pid, int *w, int opt){
        ni(__FUNCTION__);
        return -1;
    }
    int kill(int a, int s) {
        ni(__FUNCTION__);
        return -1;
    }

    int dup(int o){
        ni(__FUNCTION__);
        return -1;
    }


    int dup2(int o, int n){
        ni(__FUNCTION__);
        return -1;
    }

    int pipe(int p[2]) {
        ni(__FUNCTION__);
        return -1;

    }

    int _exit(int a) {
        ocall_exit(a);
    }

    int fork(void) {
        ni(__FUNCTION__);
        return -1;
    }

    int close(int fd) {
        int a;
        ocall_close(&a, fd);
        return a;

    }

    long sysconf(int n) {
        long a;
        ocall_sysconf(&a, n);
        return a;
    }

    int chdir(const char *path) {
        int a;
        ocall_chdir(&a, path);
        return a;
    }

    int execv(const char *path, char *const argv[]) {
        ni(__FUNCTION__);
        return -1;

    }

    int execvp(const char *path, char *const argv[]) {
        ni(__FUNCTION__);
        return -1;

    }

    int closedir(void *dir) {
        int a;
        ocall_closedir(&a, dir);
        return a;

    }

    void *opendir(const char *t) {
        void *p;
        ocall_opendir(&p, t);
        return p;

    }

    int symlink(const char *t, const char *l) {
        int a;
        ocall_symlink(&a, t, l);
        return a;
    }

    struct utimbuf {
        long a;
        long b;
    };

    int utime(const char *t, const struct utimbuf *l) {
        //int a;
        //ocall_symlink(&a, t, l);
        ni(__FUNCTION__);
        return -1;
    }


    void *
    dlsym(void *handle, const char *s) {
        abort();
        void *p;
        ocall_dlsym(&p, handle, s);
        return p;

    }

    int fgetc(FILE *stream) {
        unsigned char c;
        fread(&c, 1, sizeof(unsigned char), stream);
        return c;

    }
#if 0
    int signal(int a, void*b){
        abort();
        int c;
        ocall_signal(&c, a, b);
        return c;
    }
#endif
    FILE *
    fdopen(int fd, const char *mode) {
        FILE *l;
        ocall_fdopen(&l, fd, mode);
        return l;

    }

    int mkdir(const char *path, int l) {
        int a;
        ocall_mkdir(&a, path, l);
        return a;
    }

    void *readdir(DIR *l) {
        ni(__FUNCTION__);
        return NULL;
    }

    int fseeko(FILE *s, long offset, int w) {
        int a;
        ocall_fseeko(&a, s, offset, w);
        return a;
    }

    int fileno(FILE *s) {
        int a;
        ocall_fileno(&a, s);
        return a;
    }

    long ftello(FILE *s) {
        long a;
        ocall_ftello(&a, s);
        return a;
    }

    struct tm *localtime_r(void *a, void *r){
        ni(__FUNCTION__);                    
        return NULL;                         
    }                                        

    size_t
    read(int fd, void *b, size_t c)
    {
        size_t l;
        ocall_read(&l, fd, b, c);
        return l;
    }

    size_t
    write(int fd, void *b, size_t c)
    {
        size_t l;
        ocall_write(&l, fd, b, c);
        return l;
    }

#if 0
    int ioctl(int fd, unsigned long req, struct winsize *ws) {
        return 0;
        int a;
        ocall_ioctl(&a, fd, req, ws);
        return -1;
    }
#endif

    long lseek(int f, long o, int w){
        long a;
        ocall_lseek(&a, f, o, w);
        return a;
    }

    int isatty(int f) {
        return 0;
        int a;
        ocall_isatty(&a, f);
        return 0;
    }

    int tcgetattr(int f, void *p) {
        return 0;
        int a;
        ocall_tcgetattr(&a, f, p);
        return a;
    }

    int tcsetattr(int f, int oa, void *p) {
        return 0;
        int a;
        ocall_tcsetattr(&a, f, oa, p);
        return a;

    }

    char *getcwd(char *b, size_t s) {
        char *p;
        ocall_getcwd(&p, b, s);
        return p;
    }
    int utimes(const char *f, const struct timeval[2]) {
        ni(__FUNCTION__);
        return -1;
    }

    int FD_ISSET(int fd, void *set) {
        ocall_FD_ISSET(fd, set);
        return -1;
    }

    int FD_SET(int fd, void *set) {
        ocall_FD_SET(fd, set);
        return -1;
    }

    void FD_ZERO(void *p) {
        ocall_FD_ZERO(p);
        return ;

    }
    int lstat(const char *p, void *s) {
        ni(__FUNCTION__);
        return -1;
    }

    int stat(const char *p, void *s) {
        ni(__FUNCTION__);
        return -1;
    }

    int select(int a, void *r, void *w, void *e, void *t) {
        int b;
        ocall_select(&b, a, r, w, e, t);
        return b;

    }

    int open(const char *m, int a, int b) {
        ni(__FUNCTION__);
        return -1;
    }

    char *
    realpath(const char *rn, const char *r)
    {
        ni(__FUNCTION__);
        return NULL;
    }

    size_t 
    readlink(const char *r, char *b, size_t l) 
    {
        ni(__FUNCTION__);
        return -1;
    }

    void
    atomic_fetch_sub(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_fetch_xor(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_compare_exchange_strong(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_store(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_compare_exchage_strong(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_exchange(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_fetch_and(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_fetch_or(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_load(int a) {
        ni(__FUNCTION__);
    }

    void
    atomic_fetch_add(int a) {
        ni(__FUNCTION__);
    }
    
    void
    rewind(FILE *fp){
        ocall_rewind(fp);
    }

    char *
    readline(char *s)
    {
        char *p;
        ocall_readline(&p, s);
        return p;
    }

    void
    add_history(char *s)
    {
        ocall_add_history(s);
    }
}
