void naginata_type(void);
void naginata_clear(void);
bool process_naginata(uint16_t, keyrecord_t *);
void naginata_mode(bool);
bool naginata_state(void);
void set_naginata(uint8_t, uint16_t);

#ifdef NAGINATA_EDIT_MODE
bool process_naginata_edit(uint16_t, keyrecord_t *);
void set_naginata_edit(uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
#endif