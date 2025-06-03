// Contains functions to help with working with multiple PRG/CHR banks
// For VRC6 code.
// These are sort of unsafe for VRC6, and not tested (they were imported from MMC3)


// Switch to the given bank (at $8000-9fff). Your prior bank is not saved.
// Can be used for reading data with a function in the fixed bank.
// bank_id: The bank to switch to.
void __fastcall__ set_prg_8000(unsigned char bank_id);


// Get the current PRG bank at $8000-9fff.
// returns: The current bank.
unsigned char __fastcall__ get_prg_8000(void);


// WARNING, DON'T USE THIS IN THE CURRENT CFG, unless you
// really know what you're doing. El Crasho.
// Switch to the given bank (at $a000-bfff). Your prior bank is not saved.
// bank_id: The bank to switch to.
void __fastcall__ set_prg_a000(unsigned char bank_id);


// Changes a portion of the tilsets
// The plan was to NOT use these. Use the irq system instead
// You can, but make sure the IRQ system isn't changing CHR.
// See notes in README
void __fastcall__ set_chr_mode_0(unsigned char chr_id);
void __fastcall__ set_chr_mode_1(unsigned char chr_id);
void __fastcall__ set_chr_mode_2(unsigned char chr_id);
void __fastcall__ set_chr_mode_3(unsigned char chr_id);
void __fastcall__ set_chr_mode_4(unsigned char chr_id);
void __fastcall__ set_chr_mode_5(unsigned char chr_id);


// Turns off VRC6 irqs, and changes the array pointer
// to point to a default 0xff
void disable_irq(void);


// This points an array to the IRQ system 
// Also turns ON the system
void set_irq_ptr(char * address);


// Check if it's safe to write to the irq array
// returns 0xff if done, zero if not done
// if the irq pointer is pointing to 0xff it is
// safe to edit.
unsigned char is_irq_done(void);

// Functions for changing the PRG ROM when calling famistudio (thought for my game)

void famistudio_init_wrapper(unsigned char platform, void* music_data);

void famistudio_music_play_wrapper(unsigned char song_index);











