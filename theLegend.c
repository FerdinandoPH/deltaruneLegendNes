

#define min(a,b)  a < b? a : b
#define max(a,v) a > b ? a : b


#define MAP_SUM(x,y) (x + y) % 256
#define ALLOK 0
#define ALLWRONG 8
#define DISASTER 12
#define DESERT 16
#define LIGHTINDESERT 20
#define THE3HEROES 24
#define SEALANGELHEAVEN 28
#define CASTLE_A 32
#define CASTLE_B 36
#define ENDCASTLE 40

#include "libs/neslib.h" 
#include "libs/nesdoug.h" 
#include "vrc6/vrc6_code.h"
#include "music/famistudio_cc65.h"
#include "graphics/AllOk.h"
#include "graphics/AllWrong.h"
#include "graphics/Disaster.h"
#include "graphics/Desert.h"
#include "graphics/LightInDesert.h"
#include "graphics/The3Heroes.h"
#include "graphics/SealAngelHeaven.h"
#include "graphics/Clear.h"
#include "graphics/NameTableLoader.h"
#include "graphics/Castle.h"
#include "graphics/EndCastleA.h"
#include "graphics/EndCastleB.h"
#include "graphics/DeltaruneLogo.h"


 #pragma bss-name(push, "ZEROPAGE")

u8 i, j, k, l, fi, fj, fk, scx, pad;
u8 castle_scanline;
u8 castle_scene_complete;
u16 i16, fi16, fj16;
u16 scy = 0, new_pos = 0;
u8 black = 0xFF;
u8 delay_time = 3;
char palette[16]={ 0x0f,0x07,0x17,0x27,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x07,0x17,0x30};
const char mensaje1[] = "For millenia, LIGHT and DARK\n\nhave lived in balance,";
const char mensaje2[] = "Bringing peace to the WORLD.";
const char mensaje3[] = "But if this harmony\n\nwere to shatter...";
const char mensaje4[] = "A terrible calamity would occur.";
const char mensaje5[] = "The sky will run\n\nblack with terror";
const char mensaje6[] = "And the land will\n\ncrack with fear.";
const char mensaje7[] = "Then, her heart pounding...";
const char mensaje8[] = "The EARTH will draw\n\nher final breath.";
const char mensaje9[] = "Only then, shining with hope...";
const char mensaje10[] = "Three HEROES appear\n\nat WORLDS\' edge.";
const char mensaje11[] = "A HUMAN,";
const char mensaje12[] = "A MONSTER,";
const char mensaje13[] = "And a PRINCE\n\nFROM THE DARK";
const char mensaje14[] = "Only they can seal the fountains";
const char mensaje15[] = "And banish the ANGEL'S HEAVEN.";
const char mensaje16[] = "Only then will balance\n\nbe restored,";
const char mensaje17[] = "And >the WORLD >saved\n\n>from destr>uction."; //> = lower pal 0's bright
const u16 castle_triggers[] = {1, 95, 170, 285, 0};
const char castle_messages_are_double[] = {0, 1, 1, 0};
const u16 castle_messages_coordinates[] = {NTADR_B(0,20),NTADR_B(0,20),NTADR_B(0,20), NTADR_B(5,20)};
const char* castle_messages[] = {"Once upon a time, a LEGEND\n\nwas whispered among shadows.", "IItt  wwaass\n\naa  LLEEGGEENNDD\n\nooff  HDORPEEA.M S .", "IItt  wwaass\n\naa  LLEEGGEENNDD\n\nooff  LDIAGRHKT.. ", "This is the legend of"};
const char castle_last_message[] = "DELTA RUNE";
const char end_castle_triggers[] = {1, 60, 0};
const u16 end_castle_messages_coordinates[] = {NTADR_B(0, 4), NTADR_B(6,4), NTADR_B(1,4)};
const char* end_castle_messages_inscroll[] = {"Today, the FOUNTAIN OF DARKNESS-", "The geyser that\n\ngives this land form-", "But recently, another fountain\n\nhas appeared on the horizon..."};
const char end_castle_message_fragment[] = "zon...";
const char end_castle_message_1[] = "Stands tall at the\n\ncenter of the kingdom.";
const char end_castle_message_2[] = "And with it, the balance of\n\nLIGHT and DARK begins to shift..";
unsigned char std_print_sender[] = {
MSB(NTADR_A(9, 20)) | NT_UPD_HORZ, LSB(NTADR_A(9,20)), 1, 'A', NT_UPD_EOF
};
unsigned char double_print_sender[] = {0,0,1,'A',0,0,1,'B',NT_UPD_EOF};
#pragma bss-name(pop)
char irq_buffer[] = {0xF7, 0, 0xF8, 1, 150, 0xF7, 4, 0xF8, 5, 0xFF}; // IRQ buffer
char irq_buffer_castle_1[] = {0xF7,CASTLE_A, 0xF8, CASTLE_A+1, 150, 0xFE, 4, 0xFD, 0xF2, 152, 0xF7, 4, 0xF8, 5, 0xFF};
char irq_buffer_castle_2[] = {0xF7,CASTLE_A, 0xF8, CASTLE_A+1, 0xF9, CASTLE_A+2, 0xFA, CASTLE_A+3, 145, 0xF7, CASTLE_B, 0xF8, CASTLE_B+1, 0xF9, CASTLE_B+2, 0xFA, CASTLE_B+3, 1, 0xF2, 152, 0xF7, 4, 0xF8, 5, 0xFF};
char irq_buffer_castle_3[] = {0xF7,CASTLE_B, 0xF8, CASTLE_B+1, 150, 0xFE, 4, 0xFD, 0xF2, 152, 0xF7, 4, 0xF8, 5, 0xFF};
char irq_buffer_end_castle[] = {0xF7,ENDCASTLE, 0xF8, ENDCASTLE+1, 130, 0xFE, 3, 0xFD, 0xF2, 0, 0xF7, 4, 0xF8, 5, 75, 0xFE, 8, 0xF2, 0, 0xFF};
char line_vram_sender[] = {0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,NT_UPD_EOF};




void custom_fade_in(u8 palNo, u8 time){
    for (fi = 1; fi <= 3; ++fi){
        for (fj = 0; fj < fi; ++fj){
            fk = palNo*4+3-fj;
            if(palette[fk] == 0x0f){
                palette[fk] = 0x07;
            }else{
                palette[fk] += 0x10;
            }
        }
        delay(time);
        pal_bg(palette);
    }
}
void load_nametable_a_during_vblank(const unsigned char* table, u16 length){
    fi16 = 0;
    while(fi16 < length){ //NT_A+FULL 1024 B without changing palette
        set_vram_update(NULL);
        nameTableLoader[0] = MSB(fi16 + NAMETABLE_A) | NT_UPD_HORZ;
        nameTableLoader[1] = LSB(fi16 + NAMETABLE_A);
        memcpy(&nameTableLoader[3], &table[fi16], 64);
        fi16 += 64;
        set_vram_update(nameTableLoader);
        ppu_wait_nmi();
    }
    set_vram_update(NULL);
}
void fade_in(u8 time){ // From black to normal
    for (fi = 0; fi<4; ++fi){
        delay(time);
        pal_fade_to(fi, fi+1);
    }
}
void fade_out(u8 time){ //From normal to black
    for (fi = 4; fi > 0; --fi){
        delay(time);
        pal_fade_to(fi, fi-1);
    }
}
void fade_out_white(u8 time){ //From normal to white
    for (fi = 4; fi < 8; ++fi){
        delay(time);
        pal_fade_to(fi, fi+1);
    }
}
void fade_in_white(u8 time){ //From white to normal
    for (fi = 8; fi > 4; --fi){
        delay(time);
        pal_fade_to(fi, fi-1);
    }
}
void send_line_after_vscroll_a(){
    line_vram_sender[0] = MSB(NAMETABLE_A+new_pos)| NT_UPD_HORZ;
    line_vram_sender[1] = LSB(NAMETABLE_A+new_pos);
    memcpy(&line_vram_sender[3], &castle[1024+new_pos], 32);
    set_vram_update(line_vram_sender);
    new_pos = new_pos + 32;
}
void map_to(u8 image){ // Changes the tileset of BG 0 and updates the irq
    disable_irq();
    set_chr_mode_0(image);
    set_chr_mode_1(image+1);
    set_chr_mode_2(image+2);
    set_chr_mode_3(image+3);
    irq_buffer[1] = image;
    irq_buffer[3] = image+1;
    set_irq_ptr(irq_buffer);
}
void fix_text_color(u16 nametable){ //Fixes the nametable attr to show the text in white
    fi = 0;
    vram_adr(nametable+0x3E8);
    for (fi = 0; fi < 16; ++fi){
        vram_put(0xFF);
    }
}

void print(const char* message, u16 addr){
    fi = 0; //fi is the index for the message
    fj = 0; //fj is the line count
    fi16 = addr;
    --fi16;
    std_print_sender[0] = MSB(fi16) | NT_UPD_HORZ;
    std_print_sender[1] = LSB(fi16);
    while(message[fi]){
        ppu_wait_nmi();
        set_vram_update(NULL);
        if(message[fi] == '\n'){
            set_vram_update(NULL);
            std_print_sender[1] = LSB(fi16) + 32 * ++fj;
            if(fj >= 8){
                fj = 0;
                ++std_print_sender[0];
            }
        }else if (message[fi] == '>'){
            for (fk = 0; fk < 4; ++fk){
                if(palette[fk] <= 0x0f){
                    palette[fk] = 0x0f;
                }else{
                    palette[fk] -= 0x10;
                }
            }
            pal_bg(palette);
        }else{
            std_print_sender[3] = message[fi];
            ++std_print_sender[1];
            set_vram_update(std_print_sender);
            delay(delay_time);
        }
        ++fi;
    }
    ppu_wait_nmi();
    set_vram_update(NULL);
}
void print_a(const char* message, u8 x, u8 y){
    print(message, NTADR_A(x,y));
}
void clear_4_lines(u16 addr){
    set_vram_update(NULL);
    fi16 = addr;
    clear[0] = MSB(fi16) | NT_UPD_HORZ;
    clear[1] = LSB(fi16);
    set_vram_update(clear);
    ppu_wait_nmi();
    fi16 += 64;
    clear[0] = MSB(fi16) | NT_UPD_HORZ;
    clear[1] = LSB(fi16);
    ppu_wait_nmi();
    set_vram_update(NULL);
}
void clear_4_lines_a(u8 line){
    clear_4_lines(NTADR_A(0, line));
}
void change_std_scanline(u8 line){
    while(!is_irq_done()){}
    irq_buffer[4] = line;
}
void main (void) {
    ppu_off(); // screen off
    bank_spr(1); //WARNING: bg map must be 0 and spr bank must be 1 for IRQ to work in real HW
    //set_mirroring(MIRROR_HORIZONTAL); // set the mirroring modes
    pal_bg(palette); //	load the palette
    disable_irq();
    famistudio_init_wrapper((u8)FAMISTUDIO_PLATFORM_NTSC, 0x8000);

    while (1){ 
        //Castle (prep)
        map_to(CASTLE_A);
        vram_adr(NAMETABLE_A);
        vram_write(castle, 1024);
        vram_adr(NTADR_B(11,20));
        //vram_write("HOLA MUNDO",10);
        fix_text_color(NAMETABLE_B);
        scy = 0;
        new_pos = 0;
        castle_scanline = 1;
        castle_scene_complete = 0;
        irq_buffer_castle_2[8] = 146;
        irq_buffer_castle_2[17] = 1;
        i = 0xFF;
        j = 0;
        set_irq_ptr(irq_buffer_castle_1);
        pal_bright(0);
        ppu_on_all();
        famistudio_music_play_wrapper(0);
        fade_in(4);
        //Castle
        while(!castle_scene_complete){
            //i is for the current message, j is for the current character, k is a delay counter to async text and scroll
            //i16 is the address for the new char
            //l is the amount of newlines
            ppu_wait_nmi();
            set_vram_update(NULL);
            if(scy < 0x14A){
                ++k;
                if (k==3){
                    k=0;
                    scy = add_scroll_y(1, scy);
                    set_scroll_y(scy);
                    if (scy == 91){
                        while(!is_irq_done()){}
                        set_irq_ptr(irq_buffer_castle_2);
                    }else if (scy>=91 && castle_scanline){
                        while(!is_irq_done()){}
                        --irq_buffer_castle_2[8];
                        ++irq_buffer_castle_2[17];
                        if (irq_buffer_castle_2[8] == 0){
                            while(!is_irq_done()){}
                            set_irq_ptr(irq_buffer_castle_3);
                            set_chr_mode_2(CASTLE_B+2);
                            set_chr_mode_3(CASTLE_B+3);
                            castle_scanline = 0;
                        }
                    }
                    if (scy<0x100 && scy%8==0){
                        send_line_after_vscroll_a();
                    }
                }else{
                    if(castle_triggers[(u8)(i+1)] && scy>= castle_triggers[(u8)(i+1)]){
                        ++i;
                        j=0;
                        l=0;
                        clear_4_lines(NTADR_B(0,20));
                        clear_4_lines(NTADR_B(0,21));
                        i16 = castle_messages_coordinates[i];
                    }
                    if (i!=0xFF && castle_messages[i][j]&& k%2==0){
                        if(castle_messages[i][j]=='\n'){
                            ++l;
                            i16 = castle_messages_coordinates[i];
                            i16+=32*l;
                            ++j;
                        }else{
                            if(castle_messages_are_double[i]){
                                double_print_sender[0] = MSB(i16) | NT_UPD_HORZ;
                                double_print_sender[1] = LSB(i16);
                                double_print_sender[3] = castle_messages[i][j];
                                i16+=22;
                                double_print_sender[4] = MSB(i16) | NT_UPD_HORZ;
                                double_print_sender[5] = LSB(i16);
                                ++j;
                                double_print_sender[7] = castle_messages[i][j];
                                set_vram_update(double_print_sender);
                                i16-=22;
                            }else{
                                std_print_sender[0] = MSB(i16) | NT_UPD_HORZ;
                                std_print_sender[1] = LSB(i16);
                                std_print_sender[3] = castle_messages[i][j];
                                set_vram_update(std_print_sender);
                            }

                            ++j;
                            ++i16;
                        }
                        
                    }
                }
            }else{
                delay(50);
                print(castle_last_message, NTADR_B(10,22));
                delay(200);
                fade_out(4);
                ppu_off();
                castle_scene_complete = 1;
                delay(20);
            }
        }
        //AllOk
        set_scroll_y(0);
        map_to(ALLOK);
        vram_adr(NAMETABLE_A);
        vram_unrle(allOk);

        fix_text_color(NAMETABLE_A);
        pal_bright(0);
        set_irq_ptr(irq_buffer);
        ppu_on_all();
        famistudio_music_play_wrapper(1);

        fade_in(5);

        print_a(mensaje1, 2, 20);
        delay(45);

        clear_4_lines_a(20);
        print_a(mensaje2, 2, 20);
        delay(100);

        //AllWrong
        clear_4_lines_a(20);
        pal_fade_to(4, 8);
        map_to(ALLWRONG);
        load_nametable_a_during_vblank(allWrong, 0x3C0);
        pal_fade_to(8, 4);

        print_a(mensaje3, 5, 20);
        delay(75);
        fade_out(1);
        ppu_off();
        palette[3] = 0x30;
        
        pal_bg(palette);
        vram_adr(NAMETABLE_A);
        vram_fill(0xFF, 0x3C0);
        ppu_on_all();
        pal_bright(4);
        change_std_scanline(20);

        print_a(mensaje4, 0, 10);
        delay(50);
        fade_out_white(15);

        //Disaster (prep)
        palette[1] = 0x07;
        palette[2] = 0x17;
        palette[3] = 0x27;
        pal_bg(palette);
        map_to(DISASTER);
        load_nametable_a_during_vblank(disaster, 0x3C0);
        ppu_wait_nmi();
        change_std_scanline(150);
        fade_in_white(5);
        //Disaster
        print_a(mensaje5, 7, 20);
        delay(100);
        clear_4_lines_a(20);
        print_a(mensaje6, 7, 20);
        delay(125);
        clear_4_lines_a(20);
        print_a(mensaje7, 3, 20);
        delay(175);
        clear_4_lines_a(20);
        print_a(mensaje8, 6, 20);
        delay(50);
        fade_out(3);
        //Desert (prep)
        ppu_off();
        map_to(DESERT);
        vram_adr(NAMETABLE_A);
        vram_unrle(desert);
        fix_text_color(NAMETABLE_A);
        ppu_on_all();
        fade_in(4);
        //Desert
        print_a(mensaje9, 1, 20);
        delay(100);
        pal_fade_to(4,8);
        //Light in Desert (prep)
        map_to(LIGHTINDESERT);
        load_nametable_a_during_vblank(lightInDesert, 0x3C0);
        pal_fade_to(8,4);
        //Light in Desert
        print_a(mensaje10, 6, 20);
        delay(50);
        fade_out(4);
        //The3Heroes (prep)
        ppu_off();
        map_to(THE3HEROES);
        vram_adr(NAMETABLE_A);
        vram_write(the3Heroes, sizeof(the3Heroes) - 64);
        vram_write(the3Heroes_attr, sizeof(the3Heroes_attr));
        fix_text_color(NAMETABLE_A);
        ppu_on_all();
        fade_in(4);
        print_a(mensaje11, 0, 20);
        delay(100);
        custom_fade_in(1,6);
        print_a(mensaje12, 9, 20);
        delay(75);
        custom_fade_in(2,6);
        print_a(mensaje13, 19, 20);
        delay(100);
        fade_out(4);

        // SealAngelHeaven (prep)
        ppu_off();
        for (i = 4; i<12; ++i){
            palette[i] = 0x0f;
        }
        pal_bg(palette);
        map_to(SEALANGELHEAVEN);
        vram_adr(NAMETABLE_A);
        vram_unrle(sealAngelHeaven);
        fix_text_color(NAMETABLE_A);
        ppu_on_all();
        fade_in(4);
        //SealAngelHeaven
        print_a(mensaje14, 0, 20);
        delay(75);
        clear_4_lines_a(20);
        print_a(mensaje15, 1, 20);
        delay(175);
        clear_4_lines_a(20);
        print_a(mensaje16, 6, 20);
        delay(100);
        clear_4_lines_a(20);
        print_a(mensaje17, 6, 20);
        delay(50);
        fade_out(4);
        ppu_off();
        palette[1] = 0x07;
        palette[2] = 0x17;
        palette[3] = 0x27;
        pal_bg(palette);
        pal_bright(0);
        ppu_off();
        delay(50);
        //EndCastle (prep)
        disable_irq();
        map_to(ENDCASTLE);
        vram_adr(NAMETABLE_A);
        vram_write(endCastleA, 1024);
        vram_adr(NAMETABLE_B);
        vram_write(endCastleB, 1024);
        // vram_adr(NTADR_B(11,5));
        // vram_write("HOLA MUNDO",10);
        vram_adr(NAMETABLE_B+0x3C0);
        for (i = 0; i<16; ++i){
            vram_put(0xFF);
        }
        set_irq_ptr(irq_buffer_end_castle);
        pal_bright(0);
        // Reset vars
        scy = 0;
        scx = 0;
        k=0;
        i=0xFF;
        ppu_on_all();
        fade_in(4);
        while(scy < 105){
            ppu_wait_nmi();
            set_vram_update(NULL);
            ++k;
            if(k==4){
                k=0;
                scy = add_scroll_y(1, scy);
                set_scroll_y(scy);
            }else if (k%2==0){
                    if(end_castle_triggers[(u8)(i+1)] && scy>= end_castle_triggers[(u8)(i+1)]){
                        ++i;
                        j=0;
                        l=0;
                        clear_4_lines(NTADR_B(0,2));
                        i16 = end_castle_messages_coordinates[i];
                    }
                    if (i!=0xFF && end_castle_messages_inscroll[i][j]){
                        if(end_castle_messages_inscroll[i][j]=='\n'){
                            ++l;
                            i16 = end_castle_messages_coordinates[i];
                            i16+=32*l;
                            ++j;
                        }else{
                            std_print_sender[0] = MSB(i16) | NT_UPD_HORZ;
                            std_print_sender[1] = LSB(i16);
                            std_print_sender[3] = end_castle_messages_inscroll[i][j];
                            set_vram_update(std_print_sender);
                            ++j;
                            ++i16;
                        }
                        
                    }
                }
        }
        delay(128);
        set_vram_update(NULL);
        clear_4_lines(NTADR_B(0,4));
        print(end_castle_message_1, NTADR_B(5,4));
        delay(128);
        ++i;
        j=0;
        l=0;
        clear_4_lines(NTADR_B(0,4));
        i16 = end_castle_messages_coordinates[i];
        while (scx < 112){
            ppu_wait_nmi();
            ++k;
            if(k==4){
                k=0;
                scx+=2;
                set_scroll_x(scx);
            }else if(k<2){
                    if (i!=0xFF && end_castle_messages_inscroll[i][j]){
                        if(end_castle_messages_inscroll[i][j]=='\n'){
                            ++l;
                            i16 = end_castle_messages_coordinates[i];
                            i16+=32*l;
                            ++j;
                        }else{
                            std_print_sender[0] = MSB(i16) | NT_UPD_HORZ;
                            std_print_sender[1] = LSB(i16);
                            std_print_sender[3] = end_castle_messages_inscroll[i][j];
                            set_vram_update(std_print_sender);
                            ++j;
                            ++i16;
                        }
                    }
            }
        }
        set_vram_update(NULL);
        print(end_castle_message_fragment, NTADR_B(25,6));
        delay(128);
        clear_4_lines(NTADR_B(0,4));
        print(end_castle_message_2, NTADR_B(0,4));
        delay(128);
        fade_out(4);
        ppu_off();
        scx = 0;
        scy = 0;
        set_scroll_x(scx);
        set_scroll_y(scy);
        vram_adr(NAMETABLE_B);
        vram_fill(0xFF, 0x3C0); // Clear nametable B
        
        //Logo (prep)
        vram_adr(NAMETABLE_A);
        vram_unrle(deltaruneLogo);
        palette[1] = 0x16;
        palette[2] = 0x10;
        palette[3] = 0x30;
        pal_bg(palette);
        disable_irq();
        set_chr_mode_0(4);
        set_chr_mode_1(5);
        set_chr_mode_2(6);
        set_chr_mode_3(7);
        delay(100);
        ppu_on_all();
        fade_in(4);
        j=1;
        pad = 0;
        while(j){
            ppu_wait_nmi();
            pad = pad_poll(0);
            if(pad & (PAD_A | PAD_START)){
                j=0;
            }
        }
        fade_out(4);
        ppu_off();
        palette[1] = 0x07;
        palette[2] = 0x17;
        palette[3] = 0x27;
        pal_bg(palette);
    }
 }