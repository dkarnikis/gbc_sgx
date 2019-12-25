#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "sgx_urts.h"
#include "Enclave_u.h"
#include "sgx_tcrypto.h"
#define ENCLAVE_FILE "enclave.signed.so"
#define STDC_WANT_LIB_EXT1 1
#include <errno.h>
#include <locale.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <readline/readline.h>
#define RED   "\x1B[31m"   
#define GRN   "\x1B[32m"   
#define YELLOW   "\x1B[33m"   
#define BLUE   "\x1B[34m"   
#define MAGEnTA   "\x1B[35m"   
#define CYAN   "\x1B[36m"   
#define WHITE   "\x1B[37m"   
#define RESET "\x1B[0m"    

#include <SDL2/SDL.h>
/*****************************/
 struct emu_cpu_state {
    /* Lookup tables for the reg-index encoded in instructions to ptr to reg. */
    u8 *reg8_lut[9];
    u16 *reg16_lut[4];
    u16 *reg16s_lut[4];
};
  
struct player_input *input;
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <readline/history.h>

#include "../Enclave/gbc/types.h"
#include "../Enclave/gbc/hwdefs.h"
#include "../Enclave/gbc/emu.h"
#include "../Enclave/gbc/state.h"
#include "../Enclave/gbc/cpu.h"
#include "../Enclave/gbc/mmu.h"
#include "../Enclave/gbc/lcd.h"
#include "../Enclave/gbc/audio.h"
#include "../Enclave/gbc/disassembler.h"
#include "../Enclave/gbc/debugger.h"
#include "../Enclave/gbc/gui.h"
#include "../Enclave/gbc/fileio.h"

#define GUI_WINDOW_TITLE "KoenGB"
#define GUI_ZOOM      4


static SDL_Renderer *renderer;
static SDL_Texture *texture;
static SDL_AudioDeviceID audio_dev;

static int lcd_width, lcd_height;

/* Called by SDL when it needs more samples. */
void audio_callback(void *userdata, uint8_t *stream, int len) {
    uint8_t *sndbuf = userdata;
    memcpy(stream, sndbuf, len);
}

int gui_audio_init(int sample_rate, int channels, size_t sndbuf_size,
        uint8_t *sndbuf) {
    SDL_AudioSpec want, have;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        printf("SDL: failed to initialize sound: %s\n", SDL_GetError());
        return 1;
    }

    SDL_memset(&want, 0, sizeof(want));
    want.freq = sample_rate;
    want.format = AUDIO_U8;
    want.channels = channels;
    want.samples = sndbuf_size * channels;
    want.callback = audio_callback;
    want.userdata = sndbuf;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!audio_dev) {
        printf("SDL: failed to open sound device: %s\n", SDL_GetError());
        return 1;
    }
    SDL_PauseAudioDevice(audio_dev, 0);
    return 0;
}


int gui_lcd_init(int width, int height, int zoom, char *wintitle) {
    SDL_Window *window;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL failed to initialize video: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(wintitle,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width * zoom, height * zoom, 0);
    if (!window){
        printf("SDL could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!window){
        printf("SDL could not create renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture){
        printf("SDL could not create screen texture: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    lcd_width = width;
    lcd_height = height;


    return 0;
}


void ocall_gui_lcd_render_frame(char use_colors, uint16_t *pixbuf) {
    uint32_t *pixels = NULL;
    int pitch;
    if (SDL_LockTexture(texture, NULL, (void*)&pixels, &pitch)) {
        printf("SDL could not lock screen texture: %s\n", SDL_GetError());
        exit(1);
    }

    if (use_colors) {
        /* The colors stored in pixbuf are two byte each, 5 bits per rgb
         * component: -bbbbbgg gggrrrrr. We need to extract these, scale these
         * values from 0-1f to 0-ff and put them in RGBA format. For the scaling
         * we'd have to multiply by 0xff/0x1f, which is 8.23, approx 8, which is
         * a shift by 3. */
        for (int y = 0; y < lcd_height; y++)
            for (int x = 0; x < lcd_width; x++) {
                int idx = x + y * lcd_width;
                uint16_t rawcol = pixbuf[idx];
                uint32_t r = ((rawcol >>  0) & 0x1f) << 3;
                uint32_t g = ((rawcol >>  5) & 0x1f) << 3;
                uint32_t b = ((rawcol >> 10) & 0x1f) << 3;
                uint32_t col = (r << 24) | (g << 16) | (b << 8) | 0xff;
                pixels[idx] = col;
            }
    } else {
        /* The colors stored in pixbuf already went through the palette
         * translation, but are still 2 bit monochrome. */
        uint32_t palette[] = { 0xffffffff, 0xaaaaaaaa, 0x66666666, 0x11111111 };
        for (int y = 0; y < lcd_height; y++)
            for (int x = 0; x < lcd_width; x++) {
                int idx = x + y * lcd_width;
                pixels[idx] = palette[pixbuf[idx]];
            }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(1000/120);
}

void
ocall_gui_input_poll(struct player_input *input) {

    input->special_quit = 0;
    input->special_savestate = 0;
    input->special_dbgbreak = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
            case SDLK_q:
                input->special_quit = 1;
                break;

            case SDLK_b:         input->special_dbgbreak = 1; break;
            case SDLK_s:         input->special_savestate = 1; break;

            case SDLK_RETURN:    input->button_start = 1; break;
            case SDLK_BACKSPACE: input->button_select = 1; break;
            case SDLK_x:         input->button_b = 1; break;
            case SDLK_z:         input->button_a = 1; break;
            case SDLK_DOWN:      input->button_down = 1; break;
            case SDLK_UP:        input->button_up = 1; break;
            case SDLK_LEFT:      input->button_left = 1; break;
            case SDLK_RIGHT:     input->button_right = 1; break;
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_RETURN:    input->button_start = 0; break;
            case SDLK_BACKSPACE: input->button_select = 0; break;
            case SDLK_x:         input->button_b = 0; break;
            case SDLK_z:         input->button_a = 0; break;
            case SDLK_DOWN:      input->button_down = 0; break;
            case SDLK_UP:        input->button_up = 0; break;
            case SDLK_LEFT:      input->button_left = 0; break;
            case SDLK_RIGHT:     input->button_right = 0; break;
            }
            break;

        case SDL_QUIT:
            input->special_quit = 1;
        }
    }
}



void print_usage(char *progname) {
    printf("Usage: %s [option]... rom\n\n", progname);
    printf("GameBoy emulator by koenk.\n\n");
    printf("Options:\n");
    printf(" -S, --break-start      Break into debugger before executing first "
            "instruction.\n");
    printf(" -a, --audio            Enable (WIP) audio\n");
    printf(" -d, --print-disas      Print every instruction before executing "
            "it.\n");
    printf(" -m, --print-mmu        Print every memory access\n");
    printf(" -b, --bios=FILE        Use the specified bios (default is no "
            "bios).\n");
    printf(" -l, --load-state=FILE  Load the gamestate from a file (makes ROM "
            "optional).\n");
    printf(" -e, --load-save=FILE   Load the battery-backed (cartridge) RAM "
            "from a file, the \n");
    printf("                        normal way of saving games. (Optional: the "
            "emulator will \n");
    printf("                        automatically search for this file).\n");
}

int parse_args(int argc, char **argv, struct emu_args *emu_args) {
    memset(emu_args, 0, sizeof(struct emu_args));

    if (argc == 1) {
        print_usage(argv[0]);
        return 1;
    }

    while (1) {
        static struct option long_options[] = {
            {"break-start",  no_argument,        0,  'S'},
            {"audio",        no_argument,        0,  'a'},
            {"print-disas",  no_argument,        0,  'd'},
            {"print-mmu",    no_argument,        0,  'm'},
            {"bios",         required_argument,  0,  'b'},
            {"load-state",   required_argument,  0,  'l'},
            {"load-save",    required_argument,  0,  'e'},
            {0, 0, 0, 0}
        };

        char c = getopt_long(argc, argv, "Sadmb:l:e:", long_options, NULL);

        if (c == -1)
            break;

        switch (c) {
            case 'S':
                emu_args->break_at_start = 1;
                break;

            case 'a':
                emu_args->audio_enable = 1;
                break;

            case 'd':
                emu_args->print_disas = 1;
                break;

            case 'm':
                emu_args->print_mmu = 1;
                break;

            case 'b':
                emu_args->bios_filename = optarg;
                break;

            case 'l':
                emu_args->state_filename = optarg;
                break;

            case 'e':
                emu_args->save_filename = optarg;
                break;

            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (optind != argc - 1) {
        /* The remainder are non-option arguments (ROM) */
        print_usage(argv[0]);
        return 1;
    }

    emu_args->rom_filename = argv[optind];

    return 0;
}


void
print_data(unsigned int got){
	printf("The secret is: %u\n", got);
}

void
ocall_print(uint8_t* value){
    int i;
    for(i = 0; i < 32; i++){
	printf("%2.2x", value[i]);
    }
}

int
main(int argc, char *argv[])
{
    unsigned int secret;
	sgx_enclave_id_t eid = 0 ;
	sgx_status_t ret = SGX_SUCCESS;
	sgx_launch_token_t token = {0};
	int updated = 0;
    input = malloc(sizeof(struct player_input));
	/* create the enclave */
	printf("Creating Enclave\n");
	ret = sgx_create_enclave("enclave.signed.so", SGX_DEBUG_FLAG, &token,
	    &updated, &eid, NULL);
	if (ret != SGX_SUCCESS){
		printf("\nERROR: failed to create enclave, code: %#x\n", ret);
        abort();
	}

    input = malloc(sizeof(struct player_input));
    ret = ecall_init(eid, argc, stdin, stdout, stdout);
    struct gb_state gb_state, *g2_state;
    struct emu_args emu_args;



    if (parse_args(argc, argv, &emu_args))
        return 1;
    /* Initialize frontend-specific GUI */
    if (gui_lcd_init(GB_LCD_WIDTH, GB_LCD_HEIGHT, GUI_ZOOM, GUI_WINDOW_TITLE)) {
        fprintf(stderr, "Couldn't initialize GUI LCD\n");
        return 1;
    }

    printf("%s\n",emu_args.rom_filename);
    printf("%s\n",emu_args.bios_filename);
    printf("%s\n",emu_args.state_filename);
    printf("%s\n",emu_args.save_filename);
    printf("%d\n",emu_args.break_at_start);
    printf("%d\n",emu_args.print_disas);
    printf("%d\n",emu_args.print_mmu);
    printf("%d\n",emu_args.audio_enable);

    struct emu_state es, es2;
    struct emu_cpu_state ecs;
    ecall_emu_init(eid, &gb_state, &emu_args, &es, &ecs);
    gb_state.emu_cpu_state = &ecs;
    gb_state.emu_state = &es;
    printf("%d\n", gb_state.emu_state->quit);
    struct player_input input_state, *ap;
    ap = malloc(sizeof(struct player_input));
    uint16_t *b;
    b = malloc(23040 * 2);
    memset(&input_state, 0, sizeof(struct player_input));
    while (!gb_state.emu_state->quit) {
        ecall_emu_step_frame(eid, &gb_state, &es, &ecs, b);
        gb_state.emu_cpu_state = &ecs;
        gb_state.emu_state = &es;
       
        ocall_gui_input_poll(&input_state);
        ecall_emu_process_inputs(eid, &gb_state, &input_state, &es2);
        gb_state.emu_state = &es2;
        ocall_gui_lcd_render_frame(gb_state.gb_type == GB_TYPE_CGB,
            b);
        //if (gb_state.emu_state->audio_enable) /* TODO */
        //    audio_update(&gb_state);
        //
    }








    //ecall_start(eid, emu_args);   

    printf("==========================\n");
    printf("=== Starting execution ===\n");
    printf("==========================\n\n");

    struct timeval starttime, endtime;
    gettimeofday(&starttime, NULL);

#if 0
    if (gb_state.emu_state->extram_dirty)
        ecall_emu_save(eid, &gb_state, 1, gb_state.emu_state->save_filename_out);
#endif
    gettimeofday(&endtime, NULL);

    printf("\nEmulation ended at instr: ");
    //ecall_disassemble(eid, &gb_state);
    //dbg_print_regs(&gb_state);

    int t_usec = endtime.tv_usec - starttime.tv_usec;
    int t_sec = endtime.tv_sec - starttime.tv_sec;
    double exectime = t_sec + (t_usec / 1000000.);

    double emulated_secs = gb_state.emu_state->time_seconds +
        gb_state.emu_state->time_cycles / 4194304.;

    printf("\nEmulated %f sec in %f sec WCT, %.0f%%.\n", emulated_secs, exectime,
            emulated_secs / exectime * 100);



    printf("Secret = %u\n", secret);

}	




char *
ocall_setlocale(int category, const char *locale)
{
    return setlocale(category, locale);
}

void
ocall_exit(int status)
{
    exit(status);
}

long int
ocall_clock()
{
    return clock();
}

long int
ocall_time(long int *src)
{
    return time(src);
}

char *
ocall_getenv(const char *name)
{
    return getenv(name);
}

int
ocall_write(int file, const void *buf, unsigned int size)
{
   return (int)write(file, buf, size);
}

int
ocall_fputs(const char *str, FILE *fd)
{
    return fputs(str, fd);
}

size_t
ocall_fwrite(void *buffer, size_t size, size_t count, FILE *fd)
{
    return fwrite(buffer, size, count, fd);
}

char *recover_filename(FILE *fd);

char *
ocall_fgets(char *str, int n, FILE *fd)
{
#if defined(DEBUG)
    printf(RED "FGETS: stream  |%s|\n" RESET, recover_filename(fd));
#endif
   
    return fgets(str, n, fd);
}

int
ocall_fflush(FILE *ptr)
{
    return fflush(ptr);
}

struct lconv *
ocall_localeconv()
{
    return localeconv();
}

FILE *
ocall_fopen(char *filename, int l, const char *mode)
{
    return fopen(filename, mode); 
}	

FILE *
ocall_fdopen(const char *filename, const char *mode)
{
    return fopen(filename, mode); 
}	



FILE *
ocall_popen(const char *cmd, const char *type)
{
    return popen(cmd, type);
}


int
ocall_pclose(FILE *stream)
{
    return pclose(stream);

}

int
ocall_system(const char *str)
{
    int a;
    a = system(str);
    (void)a;
    return 0;
}

long int
ocall_mktime(struct tm *timeptr)
{
    return mktime(timeptr);
}

int
ocall_remove(const char *filename)
{
    return remove(filename);
}

struct tm *
ocall_localtime(const long int *timer)
{
    return localtime(timer);
}

struct tm *
ocall_gmtime(const long int *timer)
{
    return gmtime(NULL);
}

int
ocall_rename(const char *old_filename, const char *new_filename)
{
    return rename(old_filename, new_filename);
}

char *
ocall_tmpnam(char *str)
{
    return tmpnam(str);
}

int
ocall_fclose(FILE *ptr)
{
    return fclose(ptr);
}

int
ocall_setvbuf(FILE *stream, char *buffer, int mode, size_t size)
{
    return setvbuf(stream, buffer, mode, size);
}

long int
ocall_ftell(FILE *stream)
{
    return ftell(stream);
}

int
ocall_fseek(FILE *stream, long int offset, int whence)
{
    return fseek(stream, offset, whence);
}

FILE *
ocall_tmpfile()
{
    return tmpfile();
}

void
ocall_clearerr(FILE *stream)
{
    clearerr(stream);
}



char *
recover_filename(FILE *f)
{
  int fd;
  char fd_path[255];
  char * filename = (char *)malloc(255);
  size_t n;

  fd = fileno(f);
  sprintf(fd_path, "/proc/self/fd/%d", fd);
  n = readlink(fd_path, filename, 255);
  if (n < 0)
      return NULL;
  filename[n] = '\0';
  return filename;
}

size_t
ocall_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t res;
#if defined(DEBUG)
    printf(RED "FREAD: opening %s %zd %zd\n" RESET, recover_filename(stream), size, nmemb);
    //printf(RED "FREAD: character:|%s|\n" RESET, ((char *)ptr));
#endif
    res = fread(ptr, size, nmemb, stream);

    return res;
}

int
ocall_ferror(FILE *stream)
{
    return ferror(stream);
}


int
ocall_getc(FILE *stream)
{
    int res;
    res = getc(stream);
    //if(run_locally == 1)
    //    fseek(stream, 0, SEEK_SET);
#if defined(DEBUG)
    printf(GRN "GETC: stream  |%s|\n" RESET, recover_filename(stream));
    printf(GRN "GETC: opening |%d|->|%c|\n" RESET, res, (char)res);
#endif
    return res;
}

int
ocall_ungetc(int ch, FILE *stream)
{
    int res;
    res = ungetc(ch, stream);
#if defined(DEBUG)
    printf(CYAN "UNGETC: stream  |%s|\n" RESET, recover_filename(stream));
    printf(CYAN "UNGETC: opening |%d|->|%c|\n" RESET, res, (char)res);
#endif
    return res;
}

FILE *
ocall_freopen(const char *filename, const char *mode, FILE *stream)
{
#if defined(DEBUG)
    printf(GRN "FREOPEN: stream  |%s|\n" RESET, filename);
#endif
    return freopen(filename, mode, stream);
}

int
ocall_feof(FILE *stream)
{
#if defined(DEBUG)
    printf(CYAN "FEOF: stream  |%s|\n" RESET, recover_filename(stream));
#endif

    return feof(stream);
}

int
ocall_rand()
{   
    return rand();
}

/* 
 * random ocall function
 */
void
ocall_srand(unsigned int seed)
{
    srand(seed);

}

/*
 * ocall from lua function that prints numbers on stdout
 */
void
ocall_print_num(size_t s)
{
    fprintf(stdout, "%zd", s);
}


void *
ocall_dlopen(const char *fmt, int flags)
{
    return dlopen(fmt, flags);
}

int
ocall_dlclose(void *handle)
{
    return dlclose(handle);
}

int
ocall_close(int a) {
    return close(a);
}

long ocall_sysconf(int n) {
    return sysconf(n);
}

int ocall_chdir(const char *path) {
    return chdir(path);

}

int ocall_closedir(void *dir) {
    return closedir((DIR *)dir);
}

void *ocall_opendir(const char *t) {
    return opendir(t);
}

int ocall_symlink(const char *t, const char *l) {
    return symlink(t, l);
}

void *
ocall_dlsym(void *h, const char *s)
{
    return dlsym(h, s);

}

int
ocall_mkdir(const char *l, int m) {
    return mkdir(l, m);
}

int 
ocall_fseeko(FILE *s, long o, int w) {
    return fseeko(s, o, w);
}

int ocall_fileno(FILE *s) {
    return fileno(s);
}

long ocall_ftello(FILE *s) {
    return ftello(s);
}

size_t 
ocall_read(int fd, void *b, size_t c) {
    return read(fd, b, c);
}

size_t 
ocall_write(int fd, void *b, size_t c) {
    return write(fd, b, c);
}


size_t 
ocall_open(int fd, void *b, size_t c) {
    return read(fd, b, c);
}

long ocall_lseek(int f, long o, int w) {
    return lseek(f, o, w);
}

int 
ocall_isatty(int f){
    return isatty(f);
}

char *ocall_getcwd(char *b, size_t s) {
    return ocall_getcwd(b, s);
}

int ocall_signal(int a, void *b){
    return -1;//signal(a, b);
}

int ocall_ioctl(int fd, unsigned long req, struct winsize *ws) {
    return ioctl(fd, req, ws);
}

int ocall_tcgetattr(int f, void *p) {
    return 0;//tcgetattr(f, (struct termios *)p);
}

int ocall_tcsetattr(int f, int oa, const void *p) {
    return 0;//tcsetattr(f, oa, (struct termios*)p);
}

void
ocall_FD_ZERO(void *p) {
    FD_ZERO((fd_set *)p);
}


void
ocall_FD_SET(int fd, void *p) {
    FD_SET(fd, (fd_set *)p);
}


void
ocall_rewind(FILE *f)
{
    rewind(f);
}

void
ocall_FD_ISSET(int fd, void *p) {
    FD_SET(fd, (fd_set *)p);
}

int ocall_select(int a, void *r, void *w, void *e, void *t) 
{
    (void)a;
    (void)r;
    (void)e;
    (void)t;

    return 0;//select(a, r, w, e, t); 
}

char *
ocall_readline(char *s)
{
    return readline(s);
}

void
ocall_add_history(char *s)
{
    add_history(s);

}
