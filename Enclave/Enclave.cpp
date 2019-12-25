#include "Enclave_t.h"
#include "sgx_tcrypto.h"
#include <string.h>
#include "sgx_includes.h"
#include "sgx_funcs.h"
#include "sgx_structs.h"
#include <stdarg.h>
#include <unistd.h>
#include "sgx_defs.h"
#include <stdio.h>
struct emu_cpu_state {
    /* Lookup tables for the reg-index encoded in instructions to ptr to reg. */
    u8 *reg8_lut[9];
    u16 *reg16_lut[4];
    u16 *reg16s_lut[4];
};


#include "gbc/types.h"
#include "gbc/hwdefs.h"
#include "gbc/emu.h"
#include "gbc/state.h"
#include "gbc/cpu.h"
#include "gbc/mmu.h"
#include "gbc/lcd.h"
#include "gbc/audio.h"
#include "gbc/disassembler.h"
#include "gbc/debugger.h"
#include "gbc/gui.h"
#include "gbc/fileio.h"

#include "../Enclave/gbc/types.h"
unsigned int secret = 0;
/* original std* variables from the untrusted world */
int argc;
int count = 0;
char **argv;

void
print_secret(){
	print_data(secret);
}

int main(int, char **);
unsigned int
get_secret(){
    secret = 10;
    return secret;
}

void
ecall_init(int arg, FILE *stdi, FILE *stdo, FILE *stde) {
    argc = arg;
    stdin = stdi;
    stdout = stdo;
    stderr = stde;
    argv = (char **) calloc(argc + 5, sizeof(char **));
}

void
ecall_push_arg(char *arg)
{
    argv[count] = (char *)malloc(strlen(arg));
    memcpy(argv[count], arg, strlen(arg));
    count++;
}

void
ecall_emu_step_frame(struct gb_state *s)
{
    emu_step_frame(s);
}


void
ecall_emu_init(struct gb_state *s, struct emu_args *p)
{
    emu_init(s, p);
}

void
ecall_start(struct emu_args emu_args)
{


    struct gb_state gb_state;    
    ecall_emu_init(&gb_state, &emu_args);
    printf("%d\n", gb_state.emu_state->quit);
    struct player_input input_state, *ap;
    ap = malloc(sizeof(struct player_input));
    uint16_t *b;
    b = malloc(23040 * 2);
    memset(&input_state, 0, sizeof(struct player_input));
    while (!gb_state.emu_state->quit) {
        ecall_emu_step_frame(&gb_state);//, &emu_state, &emu_cpu_state, b);
        //ocall_gui_input_poll(&ap, &input_state);

        emu_process_inputs(&gb_state, ap);
        memcpy(b, gb_state.emu_state->lcd_pixbuf, 23040 * 2);
        ocall_gui_lcd_render_frame(gb_state.gb_type == GB_TYPE_CGB,
            b);
        //if (gb_state.emu_state->audio_enable) /* TODO */
        //    audio_update(&gb_state);
        //
    }
}


void
set_secret(unsigned int a){
	secret = a;
	return;
}


void
addition(unsigned int a, unsigned int b){
	secret =  a + b;
	return;
}

size_t mspace_usable_size(const void *mem) {
  return 0;
}


size_t malloc_usable_size(void *p) {
    return 0;
}
void
ecall_emu_init(struct gb_state *s, struct emu_args *p, struct emu_state *es, struct emu_cpu_state *ecs)
{
    emu_init(s, p);
    memcpy(es, s->emu_state, sizeof(struct emu_state));
    memcpy(ecs, s->emu_cpu_state, sizeof(struct emu_cpu_state));
    s->lala =20;
}

void ecall_emu_process_inputs(struct gb_state *s, struct player_input *p, struct emu_state *es) {
    emu_process_inputs(s, p);
    memcpy(es, s->emu_state, sizeof(struct emu_state));
}

void
ecall_emu_step_frame(struct gb_state *s, struct emu_state *es, struct emu_cpu_state *ecs, u16 *lcd_pixbuf)
{
    emu_step_frame(s);
    memcpy(es, s->emu_state, sizeof(struct emu_state));
    memcpy(ecs, s->emu_cpu_state, sizeof(struct emu_cpu_state));
    memcpy(lcd_pixbuf, es->lcd_pixbuf, 23040 * 2);
}

void ecall_emu_save(struct gb_state *s, char p, char *o)
{
    emu_save(s, p, o);
}

void
ecall_disassemble(void *s)
{
    disassemble(s);
}

void
ecall_audio_update(void *s)
{
    audio_update(s);
}
