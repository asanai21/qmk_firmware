#include QMK_KEYBOARD_H
#include "naginata.h"

#define NGBUFFER 5 // バッファのサイズ

static uint8_t ng_chrcount = 0; // 文字キー入力のカウンタ (シフトキーを除く)
static bool is_naginata = false; // 薙刀式がオンかオフか
static uint8_t naginata_layer = 0; // レイヤー番号
static uint16_t ng_shiftkey = 0; // 薙刀式のシフトキー
static bool ng_shift = false; // シフトキーの状態
static bool ng_space = false; // シフトキーかスペース入力か
static uint8_t n_modifier = 0; // 押しているmodifierキーの数

#ifdef NAGINATA_EDIT_MODE
static uint8_t naginata_elayerl = 0; // レイヤー番号
static uint8_t naginata_elayerr = 0; // レイヤー番号
static uint8_t n_editl = 0; // 押しているmodifierキーの数
static uint8_t n_editr = 0; // 押しているmodifierキーの数

// 編集モードキー
static uint16_t kchr10;
static uint16_t kchr20;
static uint16_t kchr30;
static uint16_t kup5;
static uint16_t kdown5;
#endif

// 31キーを32bitの各ビットに割り当てる
#define B_Q    (1UL<<0)
#define B_W    (1UL<<1)
#define B_E    (1UL<<2)
#define B_R    (1UL<<3)
#define B_T    (1UL<<4)

#define B_Y    (1UL<<5)
#define B_U    (1UL<<6)
#define B_I    (1UL<<7)
#define B_O    (1UL<<8)
#define B_P    (1UL<<9)

#define B_A    (1UL<<10)
#define B_S    (1UL<<11)
#define B_D    (1UL<<12)
#define B_F    (1UL<<13)
#define B_G    (1UL<<14)

#define B_H    (1UL<<15)
#define B_J    (1UL<<16)
#define B_K    (1UL<<17)
#define B_L    (1UL<<18)
#define B_SCLN (1UL<<19)

#define B_Z    (1UL<<20)
#define B_X    (1UL<<21)
#define B_C    (1UL<<22)
#define B_V    (1UL<<23)
#define B_B    (1UL<<24)

#define B_N    (1UL<<25)
#define B_M    (1UL<<26)
#define B_COMM (1UL<<27)
#define B_DOT  (1UL<<28)
#define B_SLSH (1UL<<29)

#define B_SHFT (1UL<<30)

// 文字入力バッファ
static uint16_t ninputs[NGBUFFER];

// キーコードとキービットの対応
const uint32_t ng_key[] = {
  [KC_Q]    = B_Q,
  [KC_W]    = B_W, 
  [KC_E]    = B_E, 
  [KC_R]    = B_R,
  [KC_T]    = B_T, 

  [KC_Y]    = B_Y, 
  [KC_U]    = B_U, 
  [KC_I]    = B_I, 
  [KC_O]    = B_O, 
  [KC_P]    = B_P, 

  [KC_A]    = B_A,
  [KC_S]    = B_S,
  [KC_D]    = B_D, 
  [KC_F]    = B_F, 
  [KC_G]    = B_G,

  [KC_H]    = B_H, 
  [KC_J]    = B_J,  
  [KC_K]    = B_K, 
  [KC_L]    = B_L,
  [KC_SCLN] = B_SCLN,

  [KC_Z]    = B_Z,   
  [KC_X]    = B_X, 
  [KC_C]    = B_C, 
  [KC_V]    = B_V, 
  [KC_B]    = B_B, 

  [KC_N]    = B_N, 
  [KC_M]    = B_M, 
  [KC_COMM] = B_COMM, 
  [KC_DOT]  = B_DOT, 
  [KC_SLSH] = B_SLSH, 
};

// 仮名のマップ。ng_combと対応する。
const char PROGMEM ng_kana[][5] = {
  // 単独
  "vu", "ha", "te", "si"             , "BS", "ru", "su", "he",
  "ro", "ki", "to", "ka", "ltu", "ku", "a" , "i" , "u" , "-" ,
  "ho", "hi", "ke", "ko", "so" , "ta", "na", "nn", "ra", "re",

  // シフト
        "mi", "ri", "me"            , "sa", "yo", "yu", "e" ,
  "se", "ne", "ni", "ma", "ti", "wa", "no", "mo", "tu", "ya",
              "wo", "," , "nu", "o" , "." , "mu", "hu",

  // 濁音
        "ba", "de", "ji"            , "za",       "zu", "be",
  "ze", "gi", "do", "ga", "du", "gu",             "du",
  "bo", "bi", "ge", "go", "zo", "da",             "bu",

  // 半濁音
  "pa", "pi", "pu", "pe", "po",

  // 小書き
  "la", "li", "lu" , "le", "lo",
  // 拗音
  "lya", "lyu", "lyo",

  // 拗音同時
  "kya", "kyu", "kyo", 
  "sya", "syu", "syo", 
  "tya", "tyu", "tyo", 
  "nya", "nyu", "nyo", 
  "hya", "hyu", "hyo", 
  "mya", "myu", "myo", 
  "rya", "ryu", "ryo",

  // 濁音拗音同時
  "gya", "gyu", "gyo",
  "jya", "jyu", "jyo",
  "dya", "dyu", "dyo",
  "bya", "byu", "byo",

  // 半濁音拗音同時
  "pya", "pyu", "pyo",

  // 外来音
  "va", "vi", "vyu", "ve", "vo",
  "teli", "telu",
  "sye",
  "tolu",
  "tile",
  "uli", "ule", "ulo",
  "fa", "fi", "fyu", "fe", "fo" ,

  // シフト外来音
  "tula", "tuli", "tule", "tulo",

  // 濁音外来音
  "deli", "delu",
  "je",
  "dolu",
  "dile",
};

// 同時キー組み合わせのマップ。ng_kanaと対応する。
const uint32_t ng_comb[] = {
  // 単独
  B_Q, B_W, B_E, B_R          , B_U, B_I   , B_O  , B_P,
  B_A, B_S, B_D, B_F, B_G, B_H, B_J, B_K   , B_L  , B_SCLN,
  B_Z, B_X, B_C, B_V, B_B, B_N, B_M, B_COMM, B_DOT, B_SLSH,

  // シフト
              B_SHFT|B_W, B_SHFT|B_E, B_SHFT|B_R                        , B_SHFT|B_U, B_SHFT|B_I   , B_SHFT|B_O  , B_SHFT|B_P,
  B_SHFT|B_A, B_SHFT|B_S, B_SHFT|B_D, B_SHFT|B_F, B_SHFT|B_G, B_SHFT|B_H, B_SHFT|B_J, B_SHFT|B_K   , B_SHFT|B_L  , B_SHFT|B_SCLN,
                          B_SHFT|B_C, B_SHFT|B_V, B_SHFT|B_B, B_SHFT|B_N, B_SHFT|B_M, B_SHFT|B_COMM, B_SHFT|B_DOT,

  // 濁音
           B_J|B_W, B_J|B_E, B_J|B_R                  , B_F|B_U,             B_F|B_O  , B_F|B_P,
  B_J|B_A, B_J|B_S, B_J|B_D, B_J|B_F, B_J|B_G, B_F|B_H                     , B_F|B_L  ,
  B_J|B_Z, B_J|B_X, B_J|B_C, B_J|B_V, B_J|B_B, B_F|B_N                     , B_F|B_DOT,

  // 半濁音
  B_M|B_W, B_M|B_X, B_V|B_DOT , B_V|B_P , B_M|B_Z,

  // 小書き
  B_SHFT|B_V|B_J, B_SHFT|B_V|B_K, B_SHFT|B_V|B_L, B_SHFT|B_V|B_P, B_SHFT|B_V|B_N,
  // 拗音
  B_SHFT|B_V|B_SCLN, B_SHFT|B_V|B_O, B_SHFT|B_V|B_I,

  // 拗音同時
  B_S|B_SCLN, B_S|B_O, B_S|B_I,
  B_R|B_SCLN, B_R|B_O, B_R|B_I,
  B_G|B_SCLN, B_G|B_O, B_G|B_I,
  B_D|B_SCLN, B_D|B_O, B_D|B_I,
  B_X|B_SCLN, B_X|B_O, B_X|B_I,
  B_W|B_SCLN, B_W|B_O, B_W|B_I,
  B_E|B_SCLN, B_E|B_O, B_E|B_I,

  // 濁音拗音同時
  B_J|B_S|B_SCLN, B_J|B_S|B_O, B_J|B_S|B_I,
  B_J|B_R|B_SCLN, B_J|B_R|B_O, B_J|B_R|B_I,
  B_J|B_G|B_SCLN, B_J|B_G|B_O, B_J|B_G|B_I,
  B_J|B_X|B_SCLN, B_J|B_X|B_O, B_J|B_X|B_I,

  // 半濁音拗音同時
  B_M|B_X|B_SCLN, B_M|B_X|B_O, B_M|B_X|B_I,

  // 外来音
  B_Q|B_J, B_Q|B_K, B_Q|B_L, B_Q|B_P, B_Q|B_N,
  B_E|B_K, B_E|B_L,
  B_R|B_P,
  B_D|B_L,
  B_G|B_P,
  B_L|B_K, B_L|B_P, B_L|B_N,
  B_DOT|B_J, B_DOT|B_K, B_DOT|B_L, B_DOT|B_P, B_DOT|B_N,

  // シフト外来音
  B_SHFT|B_L|B_J, B_SHFT|B_L|B_K, B_SHFT|B_L|B_P, B_SHFT|B_L|B_N,

  // 濁音外来音
  B_E|B_J|B_K, B_E|B_J|B_L,
  B_R|B_J|B_P,
  B_D|B_J|B_L,
  B_G|B_J|B_P,
};

// 薙刀式のレイヤー、シフトキーを設定
void set_naginata(uint8_t layer, uint16_t shiftkey) {
  naginata_layer = layer;
  ng_shiftkey = shiftkey;
}

#ifdef NAGINATA_EDIT_MODE
void set_naginata_edit(uint8_t layer1, uint8_t layer2, uint16_t ke1, uint16_t ke2, uint16_t ke3, uint16_t ke4, uint16_t ke5) {
  naginata_elayerl = layer1;
  naginata_elayerr = layer2;
  kchr10 = ke1;
  kchr20 = ke2;
  kchr30 = ke3;
  kup5   = ke4;
  kdown5 = ke5;
}
#endif

// 薙刀式をオンオフ
void naginata_on(void) {
  is_naginata = true;
  layer_on(naginata_layer);
}

void naginata_off(void) {
  is_naginata = false;
  layer_off(naginata_layer);
}

// 薙刀式の状態
bool naginata_state(void) {
  return is_naginata;
}

// キー入力を文字に変換して出力する
void naginata_type(void) {
  char kana[5];
  bool douji = false;
  uint32_t keycomb = 0UL; // 同時押しの状態を示す。32bitの各ビットがキーに対応する。

  if (ng_shift) keycomb |= B_SHFT; // シフトキー状態を反映

  for (int i = 0; i < ng_chrcount; i++) {
    keycomb |= ng_key[ninputs[i]]; // バッファにあるキー状態を合成する
  }

  switch (keycomb) {
    case B_U: // send_stringできないキーはここで定義
      register_code(KC_BSPC);
      unregister_code(KC_BSPC);
      break;
    case B_V|B_M:
      register_code(KC_ENT);
      unregister_code(KC_ENT);
      break;
    default:
      // キーから仮名に変換して出力する。
      // 同時押しの場合
      for (int i = 0; i < sizeof ng_comb / sizeof ng_comb[0]; i++) {
        if (keycomb == ng_comb[i]) {
          douji = true;
          memcpy_P(&kana, &ng_kana[i], sizeof(kana)); 
          send_string(kana);
          break;
        }
      }
      // 連続押しの場合
      if (!douji) {
        for (int j = 0; j < ng_chrcount; j++) {
          for (int i = 0; i < sizeof ng_comb / sizeof ng_comb[0]; i++) {
            if (ng_key[ninputs[j]] == ng_comb[i]) {
              memcpy_P(&kana, &ng_kana[i], sizeof(kana)); 
              send_string(kana);
              break;
            }
          }
        }
      }
  }

  naginata_clear(); // バッファを空にする
}

// バッファをクリアする
void naginata_clear(void) {
  for (int i = 0; i < NGBUFFER; i++) {
    ninputs[i] = 0;
  }
  ng_chrcount = 0;
  ng_space = false;
}

void repeatkey(uint16_t k, uint8_t n) {
  for (int i = 0; i < n; i++) {
    register_code(k);
    unregister_code(k);
  }
}

// 薙刀式の処理
bool process_naginata(uint16_t keycode, keyrecord_t *record) {
  if (!is_naginata) return true;

  // modifierが押されたらレイヤーをオフ
  switch (keycode) {
    case KC_LCTRL:
    case KC_LSHIFT:
    case KC_LALT:
    case KC_LGUI:
    case KC_RCTRL:
    case KC_RSHIFT:
    case KC_RALT:
    case KC_RGUI:
      if (record->event.pressed) {
        n_modifier++;
        layer_off(naginata_layer);
      } else {
        n_modifier--;
        if (n_modifier == 0) {
          layer_on(naginata_layer);
        }
      }
      break;
  }

  if (n_modifier == 0) {
    #ifdef NAGINATA_EDIT_MODE
    if (record->event.pressed) {
      switch (keycode) {
        case KC_D:
        case KC_F:
          n_editr++;
          if (n_editr == 2) {
            layer_on(naginata_elayerr);
          }
          break;
        case KC_J:
        case KC_K:
          n_editl++;
          if (n_editl == 2) {
            layer_on(naginata_elayerl);
          }
          break;
      }
    } else {
      switch (keycode) {
        case KC_D:
        case KC_F:
          n_editr--;
          layer_off(naginata_elayerr);
          break;
        case KC_J:
        case KC_K:
          n_editl--;
          layer_off(naginata_elayerl);
          break;
      }
    }
    #endif

    if (record->event.pressed) {
      switch (keycode) {
        case KC_A ... KC_Z:
        case KC_SLSH:
        case KC_DOT:
        case KC_COMM:
        case KC_SCLN:
          ninputs[ng_chrcount] = keycode; // キー入力をバッファに貯める
          ng_chrcount++;
          if (ng_chrcount > 2) naginata_type(); // 3文字押したら処理を開始
          return false;
          break;
        default: // 薙刀式入力に関係ないキーを押したらバッファを空にして処理を中断
          naginata_clear();
          break;
      }
      if (keycode == ng_shiftkey) {
        ng_shift = true;
        ng_space = true;
        return false;
      }

    } else { // key release
      switch (keycode) {
        case KC_A ... KC_Z:
        case KC_SLSH:
        case KC_DOT:
        case KC_COMM:
        case KC_SCLN:
          // 3文字入力していなくても、どれかキーを離したら処理を開始する
          if (ng_chrcount > 0) naginata_type();
          return false;
          break;
      }
      if (keycode == ng_shiftkey) {
        ng_shift = false;
        if (ng_space) {
          register_code(KC_SPC);
          unregister_code(KC_SPC);
        }
        return false;
      }
    }
  }
  return true;
}

#ifdef NAGINATA_EDIT_MODE
// 編集モード
bool process_naginata_edit(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    if (keycode == kchr10) {
      register_code(KC_LCMD);
      register_code(KC_LEFT);
      unregister_code(KC_LEFT);
      unregister_code(KC_LCMD);      
      repeatkey(KC_RGHT, 10);
      return false;
    } else if (keycode == kchr20) {
      register_code(KC_LCMD);
      register_code(KC_LEFT);
      unregister_code(KC_LEFT);
      unregister_code(KC_LCMD);      
      repeatkey(KC_RGHT, 20);
      return false;
    } else if (keycode == kchr30) {
      register_code(KC_LCMD);
      register_code(KC_LEFT);
      unregister_code(KC_LEFT);
      unregister_code(KC_LCMD);      
      repeatkey(KC_RGHT, 30);
      return false;
    } else if (keycode == kup5) {
      repeatkey(KC_UP, 5);
      return false;
    } else if (keycode == kdown5) {
      repeatkey(KC_DOWN, 5);
      return false;
    }
  }
  return true;
}
#endif