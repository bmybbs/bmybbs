/* -------------add by hightman for 跳蚤市场------------------ */
/* filenames, put them in bbs.h                                */
/* ----------------------------------------------------------- */
#define FN_GRP   ".GRP"
#define FN_ITEM   ".ITEM"
#define FN_2ND_DECL  "2ndhand_decl"
#define FN_2ND_HELP  "2ndhand_help"

/*----------------------------------------------------*/
/* 跳蚤市场的有关定义                                 */
/*----------------------------------------------------*/

// SLOT propeties, put them in struct.h
#define PROP_EMPTY  0x0000 // slot is empty, available
#define PROP_G_GROUP  0x0001
#define PROP_G_CANCEL  0x0002
#define PROP_G_OPEN  0x0004
#define PROP_I_SELL  0x0010
#define PROP_I_WANT  0x0020
#define PROP_I_CANCEL  0x0040 // if set, another prog will expire
     // this item.
#define PROP_IS_GROUP  (PROP_G_GROUP | PROP_G_CANCEL)
#define PROP_IS_ITEM  (PROP_I_SELL | PROP_I_WANT | PROP_I_CANCEL)
#define PROP_IS_CANCEL  (PROP_G_CANCEL | PROP_I_CANCEL)
typedef struct SLOT  // if is group, only prop/reply/title/fn
{                    // will be used
  time_t chrono;     // time stamp
  int prop;          // propety of this slot
  int reply;         // number of replied mail of this item
  char title[30];
  char userid[IDLEN + 1]; // userid for mail reply and owner del check
  char price[10];
  char contact[20];
  char date[6];   // only 6 bytes, not 9
  char fn[15];        // dirname or filename of item desc, max9 bytes
} SLOT;    // 80 bytes totally
typedef struct SLOT_DOUBLE_LINKLIST
{
  struct SLOT_DOUBLE_LINKLIST *prev;
  SLOT slot;
  struct SLOT_DOUBLE_LINKLIST *next;
} SDL;
 
/* ----------add by hightman 2000 ------------------------*/
