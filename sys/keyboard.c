#include<sys/idt.h>
#include<sys/sbunix.h>
#include<sys/klibc.h>
#include<sys/keyboard.h>
#include<sys/process.h>
extern __volatile__ char *video_mem_start;
int shift = 0;
int ctrl = 0;
int pressed = 0;
int alt = 0;
char ch = 0;
char kb_buf[KB_BUF_SIZE] = { 0 };

int kb_idx = 0;
int r_idx = 0;
int kb_free = KB_BUF_SIZE;
char kb_line[KB_BUF_SIZE] = { 0 };

static int kb_written = 0;

void handle_and_print(char ch, char s_ch);

void insert_key(char ch)
{
    if (kb_free > 0) {
        kb_written++;
        kb_free--;
        kb_buf[kb_idx++] = ch;
        if (kb_idx == KB_BUF_SIZE)
            kb_idx = 0;
    }
}

char read_key()
{
    char ch = -1;
    if (kb_free < KB_BUF_SIZE) {
        kb_free++;
        ch = kb_buf[r_idx];
        kb_buf[r_idx] = 0;
        r_idx++;
        if (r_idx == KB_BUF_SIZE)
            r_idx = 0;
    }
    return ch;
}

void handle_and_print(char ch, char s_ch)
{
    if (!pressed && ctrl && !shift) {
        if (ch == 'c') {
            printf("^c\n");
            insert_key('\0');
            resume_read_proc();
            return;
        } else if (ch == 'h') {
            if (kb_written > 0) {
                kb_written -= 2;
                insert_key(8);
                resume_read_proc();
            }
            return;
        } else {
            return;
        }
    }
    volatile char *glyph_mem =
        (char *) ((uint64_t) video_mem_start + 0xF8A);
    *(glyph_mem - 2) = '[';
    *(glyph_mem) = 0;
    *(glyph_mem + 2) = 0;
    *(glyph_mem + 4) = 0;
    if (!pressed) {
        if (ctrl) {
            if ('h' == ch) {
                //handling above. can delete this part. laters.
                insert_key(8);
                resume_read_proc();
                *glyph_mem++ = '^';
                *glyph_mem++ = screen_color;
                *glyph_mem++ = 'h';
                *glyph_mem++ = screen_color;
                *glyph_mem++ = ']';
                *glyph_mem++ = screen_color;
                return;
            }
        }
    }
    //!pressed ? (ctrl ? printf("^"):0) : 0;
    //!pressed ? (shift ? printf("%c",s_ch) : printf("%c",ch)): 0 ; 
    if (!pressed) {
        if (ctrl) {
            //printf("^");
            *glyph_mem++ = '^';
            *glyph_mem++ = screen_color;
        }
    }
    if (!pressed) {
        if (shift) {
            printf("%c", s_ch);
            insert_key(s_ch);
            *glyph_mem++ = s_ch;
            *glyph_mem++ = screen_color;
            resume_read_proc();
        } else {
            printf("%c", ch);
            insert_key(ch);
            *glyph_mem++ = ch;
            *glyph_mem++ = screen_color;
            resume_read_proc();
        }
    }
    *glyph_mem++ = ']';
    *glyph_mem++ = screen_color;
}

void irq_keyboard_handler(struct stack_frame *p_sf)
{
    unsigned char scancode = inb(0x60);
    pressed = !(scancode & 0x80);
    switch (scancode & 0x7F) {
        case 0x29:			//`
            handle_and_print('`', '~');
            break;
        case 0x01:			//escape
            break;
        case 0x02:			//1
            handle_and_print('1', '!');
            break;
        case 0x03:			//2
            handle_and_print('2', '@');
            break;
        case 0x04:			//3
            handle_and_print('3', '#');
            break;
        case 0x05:			//4
            handle_and_print('4', '$');
            break;
        case 0x06:			//5
            handle_and_print('5', '%');
            break;
        case 0x07:			//6
            handle_and_print('6', '^');
            break;
        case 0x08:			//7
            handle_and_print('7', '&');
            break;
        case 0x09:			//8
            handle_and_print('8', '*');
            break;
        case 0x0A:			//9
            handle_and_print('9', '(');
            break;
        case 0x0B:			//0
            handle_and_print('0', ')');
            break;
        case 0x0C:			//-
            handle_and_print('-', '_');
            break;
        case 0x0D:			//=
            handle_and_print('=', '+');
            break;
        case 0x10:			//q
            handle_and_print('q', 'Q');
            break;
        case 0x11:			//w
            handle_and_print('w', 'W');
            break;
        case 0x12:			//e
            handle_and_print('e', 'E');
            break;
        case 0x13:			//r
            handle_and_print('r', 'R');
            break;
        case 0x14:			//t
            handle_and_print('t', 'T');
            break;
        case 0x15:			//y
            handle_and_print('y', 'Y');
            break;
        case 0x16:			//u
            handle_and_print('u', 'U');
            break;
        case 0x17:			//i
            handle_and_print('i', 'I');
            break;
        case 0x18:			//o
            handle_and_print('o', 'O');
            break;
        case 0x19:			//p
            handle_and_print('p', 'P');
            break;
        case 0x1A:			//[
            handle_and_print('[', '{');
            break;
        case 0x1B:			//]
            handle_and_print(']', '}');
            break;
        case 0x2B:			//backslash
            handle_and_print('\\', '|');
            break;
        case 0x1E:			//a
            handle_and_print('a', 'A');
            break;
        case 0x1F:			//s
            handle_and_print('s', 'S');
            break;
        case 0x20:			//d
            handle_and_print('d', 'D');
            break;
        case 0x21:			//f
            handle_and_print('f', 'F');
            break;
        case 0x22:			//g
            handle_and_print('g', 'G');
            break;
        case 0x23:			//h
            handle_and_print('h', 'H');
            break;
        case 0x24:			//j
            handle_and_print('j', 'J');
            break;
        case 0x25:			//k
            handle_and_print('k', 'K');
            break;
        case 0x26:			//l
            handle_and_print('l', 'L');
            break;
        case 0x27:			//;
            handle_and_print(';', ':');
            break;
        case 0x28:			//'
            handle_and_print('\'', '\"');
            break;
        case 0x1C:			//enter
            if (!pressed) {
                printf("\n");
                insert_key('\n');
                kb_written = 0;
                resume_read_proc();
            }
            volatile char *glyph =
                (volatile char *) ((uint64_t) video_mem_start + 0xF8A);
            *(glyph - 2) = '[';
            *glyph = 0;
            *(glyph + 2) = 0;
            *(glyph + 4) = 0;
            *glyph++ = '^';
            *glyph++ = screen_color;
            *glyph++ = 'm';
            *glyph++ = screen_color;
            *glyph++ = ']';
            *glyph++ = screen_color;
            break;
        case 0x2A:			//shift
            shift = pressed ? 1 : 0;
            break;
        case 0x2C:			//z
            handle_and_print('z', 'Z');
            break;
        case 0x2D:			//x
            handle_and_print('x', 'X');
            break;
        case 0x2E:			//c
            handle_and_print('c', 'C');
            break;
        case 0x2F:			//v
            handle_and_print('v', 'V');
            break;
        case 0x30:			//b
            handle_and_print('b', 'B');
            break;
        case 0x31:			//n
            handle_and_print('n', 'N');
            break;
        case 0x32:			//m
            handle_and_print('m', 'M');
            break;
        case 0x33:			//,
            handle_and_print(',', '<');
            break;
        case 0x34:			//.
            handle_and_print('.', '>');
            break;
        case 0x35:			///
            handle_and_print('/', '?');
            break;
        case 0x1D:			//ctrl
            ctrl = pressed ? 1 : 0;
            break;
        case 0x38:			//alt
            alt = pressed ? 1 : 0;
            break;
        case 0x39:			//space
            !pressed ? printf(" ") : 0;
            !pressed ? insert_key(' ') : 0;
            glyph = (volatile char *) ((uint64_t) video_mem_start + 0xF8A);
            *(glyph - 2) = '[';
            *glyph = 0;
            *(glyph + 2) = 0;
            *(glyph + 4) = 0;
            *glyph++ = ' ';
            *glyph++ = screen_color;
            *glyph++ = ']';
            *glyph++ = screen_color;
            resume_read_proc();
            break;
        default:
            break;
    }
}
