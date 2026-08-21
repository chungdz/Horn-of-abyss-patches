#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define HERO_ID_NYX 140
#define HERO_ID_VEHR 212
#define CONFLUX_HERO_FIRST_ID 128
#define CONFLUX_HERO_LAST_ID 143
#define LEGACY_SPECIALTY_FRAME_COUNT 156
#define LEGACY_SECONDARY_SKILL_FRAME_COUNT 87
#define HOTA_SECONDARY_SKILL_FRAME_COUNT 93
#define SPIRITISM_FRAME_FIRST 39
#define SPIRITISM_FRAME_COUNT 3
#define CREATURE_ID_WALKING_DEAD 58
#define CREATURE_ID_WIGHT 60
#define CREATURE_ID_LICH 64
#define CREATURE_ID_EARTH_ELEMENTAL 113
#define CREATURE_ID_FIRE_ELEMENTAL 114
#define CREATURE_ID_PIXIE 118
#define CREATURE_ID_SPRITE 119
#define CREATURE_ID_PSYCHIC_ELEMENTAL 120
#define SECONDARY_SKILL_NECROMANCY 12
#define SECONDARY_SKILL_TEXT_ADDRESS 0x0067DCF0
#define SECONDARY_SKILL_DEFINITION_ADDRESS 0x006601D0
#define SECONDARY_SKILL_DEFINITION_CAPACITY 16
#define SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS 0x006600F8
#define SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY 12
#define SPECIALTY_DEFINITION_ADDRESS 0x00679D90
#define SPECIALTY_DEFINITION_CAPACITY 12
#define GET_NECROMANCY_CREATURE_ADDRESS 0x004E3ED0
#define GET_NECROMANCY_POWER_ADDRESS 0x004E3F40
#define GET_NECROMANCY_POWER_PROLOGUE_SIZE 6
#define SPIRITISM_RATE_BONUS_PER_LEVEL 0.05f
#define SHOW_HERO_DIALOG_ADDRESS 0x004E1A70
#define LEVEL_UP_ADDRESS 0x004DA990
#define SECONDARY_SKILL_CONTROL_CONSTRUCTOR_ADDRESS 0x004EA800
#define SECONDARY_SKILL_CONTROL_CONSTRUCTOR_PROLOGUE_SIZE 5
#define GET_SECONDARY_SKILL_ID_ADDRESS 0x004E2610
#define HERO_SECONDARY_SKILL_COUNT_OFFSET 0x101
#define SWAP_DIALOG_BUILDER_ADDRESS 0x005AAD90
#ifndef CHINESE_HOTA_R10
#define SWAP_SKILL_ID_CALL_ADDRESS 0x005B0342
#define SWAP_SKILL_POPUP_CALL_ADDRESS 0x005B0863
#define SHOW_POPUP_ADDRESS 0x004F6C00
#define SHOW_POPUP_PROLOGUE_SIZE 5
#define HOTA_HERMIT_SHACK_RVA 0x0015F480
#define HOTA_HERMIT_SHACK_PROLOGUE_SIZE 6
#define HOTA_HERMIT_POPUP_MOV_RVA 0x0015F6C9
#endif
#define SWAP_SKILL_CONTROL_COUNT 16
#define SWAP_SKILL_CONTROLS_PER_HERO 8
#define HD_HOTA_SWAP_LAYOUT_COUNT 2
#ifdef CHINESE_HOTA_R10
#define HD_HOTA_HERO_DIALOG_RVA 0x002346F0
#define HD_HOTA_HERO_DIALOG_CALL_1_RVA 0x0023669A
#define HD_HOTA_HERO_DIALOG_CALL_2_RVA 0x002372AD
#define HD_HOTA_SPECIALTY_DEFINITION_RVA 0x0029F3A4
#define HD_HOTA_SECONDARY_SKILL_DEFINITION_RVA 0x0029F3B8
#define HD_HOTA_SWAP_SPECIALTY_DEFINITION_RVA 0x002965F0
#define HD_HOTA_SWAP_SKILL_DEFINITION_RVA 0x00296650
#define HOTA_INSPECTION_GUARD_1_SITE_RVA 0x00002E6D
#define HOTA_INSPECTION_GUARD_1_CONTINUE_RVA 0x00002E76
#define HOTA_INSPECTION_GUARD_1_RETURN_RVA 0x00002EFD
#define HOTA_INSPECTION_GUARD_2_SITE_RVA 0x00003F3D
#define HOTA_INSPECTION_GUARD_2_CONTINUE_RVA 0x00003F46
#define HOTA_INSPECTION_GUARD_2_RETURN_RVA 0x00003FCD
#else
#define HD_HOTA_HERO_DIALOG_RVA 0x002350E0
#define HD_HOTA_HERO_DIALOG_CALL_1_RVA 0x0023708A
#define HD_HOTA_HERO_DIALOG_CALL_2_RVA 0x00237C9D
#define HD_HOTA_SPECIALTY_DEFINITION_RVA 0x002A043C
#define HD_HOTA_SECONDARY_SKILL_DEFINITION_RVA 0x002A0450
#define HD_HOTA_SWAP_SPECIALTY_DEFINITION_RVA 0x002975F0
#define HD_HOTA_SWAP_SKILL_DEFINITION_RVA 0x00297650
#define HOTA_INSPECTION_GUARD_1_SITE_RVA 0x00002E5D
#define HOTA_INSPECTION_GUARD_1_CONTINUE_RVA 0x00002E66
#define HOTA_INSPECTION_GUARD_1_RETURN_RVA 0x00002EED
#define HOTA_INSPECTION_GUARD_2_SITE_RVA 0x00003F2D
#define HOTA_INSPECTION_GUARD_2_CONTINUE_RVA 0x00003F36
#define HOTA_INSPECTION_GUARD_2_RETURN_RVA 0x00003FBD
#endif
#define HD_HOTA_SPECIALTY_DEFINITION_CAPACITY 12
#define HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY 16
#define HD_HOTA_SWAP_DEFINITION_CAPACITY 12
#define HOTA_INSPECTED_HERO_POINTER_RVA 0x00250E74
#define GENERAL_TEXT_ADDRESS 0x006A5DC4
#define ADVENTURE_GENERAL_TEXT_ADDRESS 0x00696A68
#define GENERAL_TEXT_TABLE_OFFSET 0x20
#define NECROMANCY_MESSAGE_PLURAL_OFFSET 0x2AC
#define NECROMANCY_MESSAGE_SINGULAR_OFFSET 0x2B0
#define GENERAL_TEXT_TABLE_COUNT 2
#define LOAD_DEF_ADDRESS 0x0055C9C0
#define LOG_CAPACITY 32768
#define CODE_PATCH_CAPACITY 9
#ifdef CHINESE_HOTA_R10
#define HOOK_PATCH_COUNT 9
#else
#define HOOK_PATCH_COUNT 15
#endif

struct ResourceItem {
  void *vtable;
  char name[12];
  DWORD name_end;
  LONG type;
  LONG references;
};

struct DefGroup {
  LONG count;
  DWORD sprites_size;
  void **frames;
};

struct LoadedDef {
  ResourceItem resource;
  DefGroup **groups;
  void *palette565;
  void *palette888;
  LONG groups_count;
  LONG *active_groups;
  LONG width;
  LONG height;
};

struct DefFrame {
  ResourceItem resource;
  LONG raw_data_size;
  LONG data_size;
  LONG compression_type;
  LONG width;
  LONG height;
  LONG frame_width;
  LONG frame_height;
  LONG margin_left;
  LONG margin_top;
  LONG width2;
  BYTE *raw_data;
};

struct SecondarySkillText {
  const char *name;
  const char *description[3];
};

struct SkillResourcePair {
  void *native_frames[SPIRITISM_FRAME_COUNT];
  void *spiritism_frames[SPIRITISM_FRAME_COUNT];
  BOOL ready;
};

static_assert(sizeof(ResourceItem) == 0x1C, "Unexpected ResourceItem layout");
static_assert(sizeof(DefGroup) == 0x0C, "Unexpected DefGroup layout");
static_assert(sizeof(LoadedDef) == 0x38, "Unexpected LoadedDef layout");
static_assert(sizeof(DefFrame) == 0x48, "Unexpected DefFrame layout");
static_assert(sizeof(SecondarySkillText) == 0x10, "Unexpected skill text layout");

typedef LoadedDef *(__thiscall *LoadDef)(const char *name);
typedef int (__thiscall *GetNecromancyCreature)(void *hero);
typedef float (__thiscall *GetNecromancyPower)(void *hero, int capped);
typedef void (__fastcall *ShowHeroDialog)(
  int hero_id,
  int dismissable,
  int not_in_town,
  int right_click);
typedef void (__thiscall *LevelUp)(void *hero);
typedef void *(__stdcall *HdShowHeroDialog)(int hero_id);
typedef void *(__thiscall *SecondarySkillControlConstructor)(
  void *control,
  int x,
  int y,
  int width,
  int height,
  int id,
  const char *definition,
  int frame,
  int state,
  int hotkey,
  int flags,
  int hint);
typedef int (__thiscall *GetSecondarySkillId)(void *hero, int slot);
typedef void *(__thiscall *SwapDialogBuilder)(
  void *dialog,
  void **heroes);
#ifndef CHINESE_HOTA_R10
typedef void (__fastcall *ShowSwapSkillPopup)(
  const char *text,
  int mode,
  int first,
  int second,
  int type,
  int frame,
  int fifth,
  int sixth,
  int seventh,
  int eighth,
  int ninth,
  int tenth);
typedef int (__thiscall *HotaHermitShack)(
  void *shack,
  void *hero,
  int second,
  int third,
  int fourth,
  int fifth);
#endif

struct CodePatch {
  uintptr_t address;
  BYTE original[CODE_PATCH_CAPACITY];
  BYTE replacement[CODE_PATCH_CAPACITY];
  DWORD size;
};

static HMODULE self_module;
static char log_buffer[LOG_CAPACITY];
static DWORD log_length;
static BOOL spiritism_message_applied;
static volatile LONG spiritism_ui_depth;
static SecondarySkillText saved_necromancy_text;
static BOOL skill_text_applied;
static char saved_specialty_definition[SPECIALTY_DEFINITION_CAPACITY];
static BOOL specialty_definition_applied;
static uintptr_t hd_specialty_definition_address;
static char saved_hd_specialty_definition[
  HD_HOTA_SPECIALTY_DEFINITION_CAPACITY];
static BOOL hd_specialty_definition_applied;
static uintptr_t hd_secondary_skill_definition_address;
static uintptr_t hd_swap_specialty_definition_address;
static char saved_hd_swap_specialty_definition[
  HD_HOTA_SWAP_DEFINITION_CAPACITY];
static BOOL hd_swap_specialty_definition_applied;
static uintptr_t hd_swap_skill_definition_address;
static uintptr_t hd_hota_base;
static const uintptr_t general_text_addresses[GENERAL_TEXT_TABLE_COUNT] = {
  GENERAL_TEXT_ADDRESS,
  ADVENTURE_GENERAL_TEXT_ADDRESS,
};
static uintptr_t plural_message_entries[GENERAL_TEXT_TABLE_COUNT];
static uintptr_t singular_message_entries[GENERAL_TEXT_TABLE_COUNT];
static const char *saved_necromancy_message_plural[GENERAL_TEXT_TABLE_COUNT];
static const char *saved_necromancy_message_singular[GENERAL_TEXT_TABLE_COUNT];
static BOOL spiritism_message_table_applied[GENERAL_TEXT_TABLE_COUNT];
static GetNecromancyCreature chained_get_necromancy_creature;
static GetNecromancyPower chained_get_necromancy_power;
static ShowHeroDialog chained_show_hero_dialog;
static LevelUp chained_level_up;
static HdShowHeroDialog chained_hd_show_hero_dialog;
static SecondarySkillControlConstructor
  chained_secondary_skill_control_constructor;
static SwapDialogBuilder chained_swap_dialog_builder;
#ifndef CHINESE_HOTA_R10
static GetSecondarySkillId chained_swap_get_secondary_skill_id;
static ShowSwapSkillPopup chained_show_swap_skill_popup;
static ShowSwapSkillPopup chained_show_spiritism_popup;
static HotaHermitShack chained_hota_hermit_shack;
static volatile LONG swap_right_click_spiritism_level;
static volatile LONG hermit_spiritism_depth;
static const BYTE hota_hermit_shack_prologue[
  HOTA_HERMIT_SHACK_PROLOGUE_SIZE] = {
    0x51, 0x53, 0x8B, 0x5C, 0x24, 0x0C,
  };
static const BYTE secondary_skill_control_constructor_prologue[
  SECONDARY_SKILL_CONTROL_CONSTRUCTOR_PROLOGUE_SIZE] = {
    0x55, 0x8B, 0xEC, 0x6A, 0xFF,
  };
static const BYTE show_popup_prologue[SHOW_POPUP_PROLOGUE_SIZE] = {
  0x55, 0x8B, 0xEC, 0x6A, 0xFF,
};
#endif
static SkillResourcePair small_skill_resources;
static SkillResourcePair large_skill_resources;
static SkillResourcePair swap_skill_resources;

static void write_log(void);

#ifdef CHINESE_HOTA_R10
static const char spiritism_name[] =
  "\273\275\301\351\312\365";
static const char basic_spiritism_description[] =
  "{\263\365\274\266\273\275\301\351\312\365}\n\n"
  "\325\275\266\267\275\341\312\370\272\363\243\254"
  "\325\363\315\366\265\304\273\356\314\345\311\372"
  "\316\357\327\334\311\372\303\374\326\265\265\304"
  "10%\275\253\261\273\327\252\273\257\316\252\273\250"
  "\321\375\241\243\275\250\326\376\272\315\261\246"
  "\316\357\273\341\325\325\263\243\314\341\271\251"
  "\274\323\263\311\241\243";
static const char advanced_spiritism_description[] =
  "{\270\337\274\266\273\275\301\351\312\365}\n\n"
  "\325\275\266\267\275\341\312\370\272\363\243\254"
  "\325\363\315\366\265\304\273\356\314\345\311\372"
  "\316\357\327\334\311\372\303\374\326\265\265\304"
  "20%\275\253\261\273\327\252\273\257\316\252\273\250"
  "\321\375\241\243\275\250\326\376\272\315\261\246"
  "\316\357\273\341\325\325\263\243\314\341\271\251"
  "\274\323\263\311\241\243";
static const char expert_spiritism_description[] =
  "{\327\250\274\322\274\266\273\275\301\351\312\365}\n\n"
  "\325\275\266\267\275\341\312\370\272\363\243\254"
  "\325\363\315\366\265\304\273\356\314\345\311\372"
  "\316\357\327\334\311\372\303\374\326\265\265\304"
  "30%\275\253\261\273\327\252\273\257\316\252\273\250"
  "\321\375\241\243\275\250\326\376\272\315\261\246"
  "\316\357\273\341\325\325\263\243\314\341\271\251"
  "\274\323\263\311\241\243";
#else
static const char spiritism_name[] = "Spiritism";
static const char basic_spiritism_description[] =
  "{Basic Spiritism}\n\n"
  "After combat, 10% of the health of slain living creatures is summoned "
  "as Pixies, or Sprites for Nyx. With the Cloak of the Undead King, Fire "
  "Elementals are summoned instead. Buildings and artifacts add their "
  "normal bonuses.";
static const char advanced_spiritism_description[] =
  "{Advanced Spiritism}\n\n"
  "After combat, 20% of the health of slain living creatures is summoned "
  "as Pixies, or Sprites for Nyx. With the Cloak of the Undead King, Earth "
  "Elementals are summoned instead. Buildings and artifacts add their "
  "normal bonuses.";
static const char expert_spiritism_description[] =
  "{Expert Spiritism}\n\n"
  "After combat, 30% of the health of slain living creatures is summoned "
  "as Pixies, or Sprites for Nyx. With the Cloak of the Undead King, Psychic "
  "Elementals are summoned instead. Buildings and artifacts add their "
  "normal bonuses.";
#endif
static const SecondarySkillText spiritism_text = {
  spiritism_name,
  {
    basic_spiritism_description,
    advanced_spiritism_description,
    expert_spiritism_description,
  },
};
static const char native_definition_name[
  SECONDARY_SKILL_DEFINITION_CAPACITY] = "secskill.def";
static const char spiritism_definition_name[
  SECONDARY_SKILL_DEFINITION_CAPACITY] = "SPIRIT.def";
static const char native_large_definition_name[
  SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY] = "secsk82.def";
static const char spiritism_large_definition_name[
  SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY] = "SPIR82.def";
static const char native_specialty_definition_name[
  SPECIALTY_DEFINITION_CAPACITY] = "un44.def";
static const char nyx_specialty_definition_name[
  SPECIALTY_DEFINITION_CAPACITY] = "IX44.def";
static const char native_hd_specialty_definition_name[
  HD_HOTA_SPECIALTY_DEFINITION_CAPACITY] = "UN44.def";
static const char nyx_hd_specialty_definition_name[
  HD_HOTA_SPECIALTY_DEFINITION_CAPACITY] = "IX44.def";
static const char native_hd_definition_name[
  HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY] = "Secskill.def";
static const char spiritism_hd_definition_name[
  HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY] = "SPIRIT.def";
static const char native_hd_swap_specialty_definition_name[
  HD_HOTA_SWAP_DEFINITION_CAPACITY] = "un32.def";
static const char nyx_hd_swap_specialty_definition_name[
  HD_HOTA_SWAP_DEFINITION_CAPACITY] = "IX32.def";
static const char native_hd_swap_skill_definition_name[
  HD_HOTA_SWAP_DEFINITION_CAPACITY] = "secsk32.def";
static const char spiritism_hd_swap_skill_definition_name[
  HD_HOTA_SWAP_DEFINITION_CAPACITY] = "SPIR32.def";
#ifndef CHINESE_HOTA_R10
static const DWORD hd_swap_skill_control_push_rvas[
  HD_HOTA_SWAP_LAYOUT_COUNT][SWAP_SKILL_CONTROL_COUNT] = {
  {
    0x000354C0, 0x000354FA, 0x00035534, 0x0003556E,
    0x000355AB, 0x000355E8, 0x00035625, 0x00035662,
    0x0003569F, 0x000356DC, 0x00035719, 0x00035756,
    0x00035793, 0x000357D0, 0x0003580D, 0x0003584A,
  },
  {
    0x00039321, 0x0003935B, 0x00039395, 0x000393CF,
    0x0003940C, 0x00039449, 0x00039486, 0x000394C3,
    0x00039500, 0x0003953D, 0x0003957A, 0x000395B7,
    0x000395F4, 0x00039631, 0x0003966E, 0x000396AB,
  },
};
#endif
#ifdef CHINESE_HOTA_R10
static const char spiritism_message_plural[] =
  "\312\251\325\271\273\275\301\351\312\365\272\363\243\254"
  "\304\343\265\304\323\242\320\333\275\253\265\320\276\374"
  "\313\300\325\337\326\320\265\304%d\270\366\270\264\273\356"
  "\316\252%s\262\242\274\323\310\353\367\342\317\302\241\243";
static const char spiritism_message_singular[] =
  "\312\251\325\271\273\275\301\351\312\365\272\363\243\254"
  "\304\343\265\304\323\242\320\333\275\253\265\320\276\374"
  "\313\300\325\337\326\320\265\3041\270\366\270\264\273\356"
  "\316\252%s\262\242\274\323\310\353\367\342\317\302\241\243";
#else
static const char spiritism_message_plural[] =
  "Practicing the art of Spiritism, your hero is able to raise %d of the "
  "enemy's dead to return under their service as %s.";
static const char spiritism_message_singular[] =
  "Practicing the art of Spiritism, your hero is able to raise one of the "
  "enemy's dead to return under their service as a %s.";
#endif

static BOOL safe_read(uintptr_t address, void *destination, SIZE_T size) {
  SIZE_T bytes_read = 0;
  return address != 0 &&
    ReadProcessMemory(
      GetCurrentProcess(),
      (LPCVOID)address,
      destination,
      size,
      &bytes_read) &&
    bytes_read == size;
}

static BOOL safe_write(uintptr_t address, const void *source, SIZE_T size) {
  SIZE_T bytes_written = 0;
  DWORD old_protection = 0;
  DWORD ignored_protection = 0;
  BOOL written;

  if (address == 0) {
    return FALSE;
  }
  written = WriteProcessMemory(
      GetCurrentProcess(),
      (LPVOID)address,
      source,
      size,
      &bytes_written);
  if (written && bytes_written == size) {
    FlushInstructionCache(
      GetCurrentProcess(),
      (LPCVOID)address,
      size);
    return TRUE;
  }
  if (!VirtualProtect(
    (LPVOID)address,
    size,
    PAGE_EXECUTE_READWRITE,
    &old_protection)) {
    return FALSE;
  }
  bytes_written = 0;
  written = WriteProcessMemory(
    GetCurrentProcess(),
    (LPVOID)address,
    source,
    size,
    &bytes_written);
  VirtualProtect(
    (LPVOID)address,
    size,
    old_protection,
    &ignored_protection);
  if (written && bytes_written == size) {
    FlushInstructionCache(
      GetCurrentProcess(),
      (LPCVOID)address,
      size);
  }
  return written && bytes_written == size;
}

static void append_char(char value) {
  if (log_length + 1 < LOG_CAPACITY) {
    log_buffer[log_length++] = value;
  }
}

static void append_text(const char *text) {
  while (*text != '\0') {
    append_char(*text++);
  }
}

static void append_hex_digit(BYTE value) {
  append_char((char)(value < 10 ? '0' + value : 'A' + value - 10));
}

static void append_hex32(DWORD value) {
  int shift;
  append_text("0x");
  for (shift = 28; shift >= 0; shift -= 4) {
    append_hex_digit((BYTE)((value >> shift) & 0x0F));
  }
}

static void append_decimal(DWORD value) {
  char digits[11];
  int count = 0;
  if (value == 0) {
    append_char('0');
    return;
  }
  while (value != 0 && count < (int)sizeof(digits)) {
    digits[count++] = (char)('0' + value % 10);
    value /= 10;
  }
  while (count != 0) {
    append_char(digits[--count]);
  }
}

static void append_resource_name(const ResourceItem *resource) {
  DWORD index;
  for (index = 0; index < sizeof(resource->name); index++) {
    if (resource->name[index] == '\0') {
      break;
    }
    append_char(resource->name[index]);
  }
}

static void append_frame(const char *label, void *pointer) {
  DefFrame frame;
  append_text(label);
  append_text("=");
  append_hex32((DWORD)(uintptr_t)pointer);
  if (!safe_read((uintptr_t)pointer, &frame, sizeof(frame))) {
    append_text(" <unreadable>\r\n");
    return;
  }
  append_text(" name=");
  append_resource_name(&frame.resource);
  append_text(" size=");
  append_decimal((DWORD)frame.width);
  append_text("x");
  append_decimal((DWORD)frame.height);
  append_text("\r\n");
}

static BOOL read_frame(
  LoadedDef *definition,
  DWORD frame_index,
  void ***entry,
  void **frame) {
  LoadedDef snapshot;
  DefGroup *group_pointer;
  DefGroup group;

  if (
    definition == NULL ||
    !safe_read((uintptr_t)definition, &snapshot, sizeof(snapshot)) ||
    snapshot.groups_count <= 0 ||
    snapshot.groups == NULL ||
    !safe_read((uintptr_t)snapshot.groups, &group_pointer, sizeof(group_pointer)) ||
    group_pointer == NULL ||
    !safe_read((uintptr_t)group_pointer, &group, sizeof(group)) ||
    frame_index >= (DWORD)group.count ||
    group.frames == NULL) {
    return FALSE;
  }
  *entry = group.frames + frame_index;
  return safe_read((uintptr_t)*entry, frame, sizeof(*frame));
}

static BOOL read_primary_group(
  LoadedDef *definition,
  LoadedDef *definition_snapshot,
  DefGroup **group_pointer,
  DefGroup *group_snapshot) {
  return
    definition != NULL &&
    definition_snapshot != NULL &&
    group_pointer != NULL &&
    group_snapshot != NULL &&
    safe_read(
      (uintptr_t)definition,
      definition_snapshot,
      sizeof(*definition_snapshot)) &&
    definition_snapshot->groups_count > 0 &&
    definition_snapshot->groups != NULL &&
    safe_read(
      (uintptr_t)definition_snapshot->groups,
      group_pointer,
      sizeof(*group_pointer)) &&
    *group_pointer != NULL &&
    safe_read(
      (uintptr_t)*group_pointer,
      group_snapshot,
      sizeof(*group_snapshot)) &&
    group_snapshot->count > 0 &&
    group_snapshot->frames != NULL;
}

static BOOL validate_frame_size(
  void *frame_pointer,
  LONG expected_width,
  LONG expected_height) {
  DefFrame frame;
  return
    frame_pointer != NULL &&
    safe_read((uintptr_t)frame_pointer, &frame, sizeof(frame)) &&
    frame.width == expected_width &&
    frame.height == expected_height;
}

static BOOL prepare_skill_resource_pair(
  LoadDef load_def,
  const char *native_name,
  const char *spiritism_name,
  LONG expected_width,
  LONG expected_height,
  SkillResourcePair *resources,
  const char *label) {
  LoadedDef *native_definition;
  LoadedDef *spiritism_definition;
  LoadedDef native_snapshot;
  LoadedDef spiritism_snapshot;
  DefGroup *native_group_pointer = NULL;
  DefGroup *spiritism_group_pointer = NULL;
  DefGroup native_group;
  DefGroup spiritism_group;
  DWORD index;

  append_text(label);
  append_text("=");
  if (
    load_def == NULL ||
    resources == NULL ||
    (native_definition = load_def(native_name)) == NULL ||
    (spiritism_definition = load_def(spiritism_name)) == NULL ||
    !read_primary_group(
      native_definition,
      &native_snapshot,
      &native_group_pointer,
      &native_group) ||
    !read_primary_group(
      spiritism_definition,
      &spiritism_snapshot,
      &spiritism_group_pointer,
      &spiritism_group) ||
    native_snapshot.width != expected_width ||
    native_snapshot.height != expected_height ||
    spiritism_snapshot.width != expected_width ||
    spiritism_snapshot.height != expected_height ||
    native_group.count != HOTA_SECONDARY_SKILL_FRAME_COUNT ||
    spiritism_group.count != LEGACY_SECONDARY_SKILL_FRAME_COUNT) {
    append_text("unavailable\r\n");
    return FALSE;
  }

  for (
    index = 0;
    index < SPIRITISM_FRAME_COUNT;
    index++) {
    if (
      !safe_read(
        (uintptr_t)(
          native_group.frames + SPIRITISM_FRAME_FIRST + index),
        &resources->native_frames[index],
        sizeof(resources->native_frames[index])) ||
      !safe_read(
        (uintptr_t)(
          spiritism_group.frames + SPIRITISM_FRAME_FIRST + index),
        &resources->spiritism_frames[index],
        sizeof(resources->spiritism_frames[index])) ||
      !validate_frame_size(
        resources->native_frames[index],
        expected_width,
        expected_height) ||
      !validate_frame_size(
        resources->spiritism_frames[index],
        expected_width,
        expected_height)) {
      append_text("invalid Spiritism frame pair\r\n");
      append_text("failed frame index=");
      append_decimal(SPIRITISM_FRAME_FIRST + index);
      append_text("\r\n");
      append_frame("native candidate", resources->native_frames[index]);
      append_frame(
        "Spiritism candidate",
        resources->spiritism_frames[index]);
      return FALSE;
    }
  }

  resources->ready = TRUE;
  append_text("ready; native group unchanged\r\n");
  return TRUE;
}

static BOOL verify_specialty_frame(
  LoadDef load_def,
  const char *definition_name,
  DWORD frame_index,
  const char *label) {
  LoadedDef *definition = load_def(definition_name);
  void **entry = NULL;
  void *frame = NULL;

  append_text(label);
  append_text("=");
  if (!read_frame(definition, frame_index, &entry, &frame)) {
    append_text("unavailable\r\n");
    return FALSE;
  }
  append_text("available\r\n");
  append_frame("extended frame", frame);
  return TRUE;
}

static BOOL is_spiritist_hero_id(DWORD hero_id) {
  return
    hero_id >= CONFLUX_HERO_FIRST_ID &&
    hero_id <= CONFLUX_HERO_LAST_ID;
}

static BOOL read_hero_id(const void *hero, DWORD *hero_id) {
  return
    hero != NULL &&
    hero_id != NULL &&
    safe_read((uintptr_t)hero + 0x1A, hero_id, sizeof(*hero_id));
}

static BOOL read_necromancy_level(
  const void *hero,
  BYTE *necromancy_level) {
  return
    hero != NULL &&
    necromancy_level != NULL &&
    safe_read(
      (uintptr_t)hero + 0xC9 + SECONDARY_SKILL_NECROMANCY,
      necromancy_level,
      sizeof(*necromancy_level));
}

static BYTE get_spiritism_level(const void *hero) {
  DWORD hero_id = 0;
  BYTE necromancy_level = 0;
  if (
    !read_hero_id(hero, &hero_id) ||
    !is_spiritist_hero_id(hero_id) ||
    !read_necromancy_level(hero, &necromancy_level) ||
    necromancy_level < 1 ||
    necromancy_level > 3) {
    return 0;
  }
  return necromancy_level;
}

static BOOL is_spiritist_hero(const void *hero) {
  return get_spiritism_level(hero) != 0;
}

static BOOL bytes_equal(
  const char *left,
  const char *right,
  DWORD size) {
  DWORD index;
  for (index = 0; index < size; index++) {
    if (left[index] != right[index]) {
      return FALSE;
    }
  }
  return TRUE;
}

static BOOL validate_definition_alias(
  uintptr_t address,
  const char *native_name,
  DWORD size) {
  char current[SECONDARY_SKILL_DEFINITION_CAPACITY];
  if (
    size > sizeof(current) ||
    !safe_read(address, current, size)) {
    return FALSE;
  }
  return bytes_equal(current, native_name, size);
}

static BOOL apply_definition_alias(
  uintptr_t address,
  const char *native_name,
  const char *replacement_name,
  char *saved_name,
  DWORD size,
  BOOL *applied) {
  char verified[SECONDARY_SKILL_DEFINITION_CAPACITY];

  if (
    applied == NULL ||
    saved_name == NULL ||
    size > sizeof(verified)) {
    return FALSE;
  }
  if (*applied) {
    return TRUE;
  }
  if (
    !safe_read(address, saved_name, size) ||
    !bytes_equal(saved_name, native_name, size) ||
    !safe_write(address, replacement_name, size)) {
    return FALSE;
  }
  *applied = TRUE;
  if (
    !safe_read(address, verified, size) ||
    !bytes_equal(verified, replacement_name, size)) {
    safe_write(address, saved_name, size);
    *applied = FALSE;
    return FALSE;
  }
  return TRUE;
}

static void restore_definition_alias(
  uintptr_t address,
  const char *saved_name,
  DWORD size,
  BOOL *applied) {
  if (applied == NULL || !*applied) {
    return;
  }
  safe_write(address, saved_name, size);
  *applied = FALSE;
}

static BOOL get_spiritism_text_address(uintptr_t *address) {
  DWORD table = 0;
  if (
    address == NULL ||
    !safe_read(SECONDARY_SKILL_TEXT_ADDRESS, &table, sizeof(table)) ||
    table == 0) {
    return FALSE;
  }
  *address =
    (uintptr_t)table +
    SECONDARY_SKILL_NECROMANCY * sizeof(SecondarySkillText);
  return TRUE;
}

static BOOL apply_spiritism_text(void) {
  uintptr_t address = 0;
  if (
    !get_spiritism_text_address(&address) ||
    !safe_read(address, &saved_necromancy_text, sizeof(saved_necromancy_text)) ||
    !safe_write(address, &spiritism_text, sizeof(spiritism_text))) {
    return FALSE;
  }
  skill_text_applied = TRUE;
  return TRUE;
}

static BOOL validate_secondary_skill_definition(void) {
  char current[SECONDARY_SKILL_DEFINITION_CAPACITY];
  return
    safe_read(
      SECONDARY_SKILL_DEFINITION_ADDRESS,
      current,
      sizeof(current)) &&
    bytes_equal(
      current,
      native_definition_name,
      sizeof(current));
}

static BOOL validate_secondary_skill_large_definition(void) {
  char current[SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY];
  return
    safe_read(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      current,
      sizeof(current)) &&
    bytes_equal(
      current,
      native_large_definition_name,
      sizeof(current));
}

static void restore_spiritism_ui(void) {
  uintptr_t text_address = 0;

  if (
    skill_text_applied &&
    get_spiritism_text_address(&text_address)) {
    safe_write(
      text_address,
      &saved_necromancy_text,
      sizeof(saved_necromancy_text));
    skill_text_applied = FALSE;
  }
}

static BOOL begin_spiritism_ui(void) {
  LONG depth = InterlockedIncrement(&spiritism_ui_depth);
  if (depth != 1) {
    return TRUE;
  }
  if (!apply_spiritism_text()) {
    restore_spiritism_ui();
    InterlockedDecrement(&spiritism_ui_depth);
    return FALSE;
  }
  return TRUE;
}

static void end_spiritism_ui(void) {
  LONG depth = InterlockedDecrement(&spiritism_ui_depth);
  if (depth == 0) {
    restore_spiritism_ui();
  } else if (depth < 0) {
    InterlockedExchange(&spiritism_ui_depth, 0);
    restore_spiritism_ui();
  }
}

static BOOL validate_specialty_definition(void) {
  return validate_definition_alias(
    SPECIALTY_DEFINITION_ADDRESS,
    native_specialty_definition_name,
    sizeof(native_specialty_definition_name));
}

static BOOL apply_nyx_specialty_definition(void) {
  return apply_definition_alias(
    SPECIALTY_DEFINITION_ADDRESS,
    native_specialty_definition_name,
    nyx_specialty_definition_name,
    saved_specialty_definition,
    sizeof(saved_specialty_definition),
    &specialty_definition_applied);
}

static void restore_nyx_specialty_definition(void) {
  restore_definition_alias(
    SPECIALTY_DEFINITION_ADDRESS,
    saved_specialty_definition,
    sizeof(saved_specialty_definition),
    &specialty_definition_applied);
}

static BOOL validate_hd_specialty_definition(void) {
  return
    hd_specialty_definition_address != 0 &&
    validate_definition_alias(
      hd_specialty_definition_address,
      native_hd_specialty_definition_name,
      sizeof(native_hd_specialty_definition_name));
}

static BOOL apply_hd_nyx_specialty_definition(void) {
  return apply_definition_alias(
    hd_specialty_definition_address,
    native_hd_specialty_definition_name,
    nyx_hd_specialty_definition_name,
    saved_hd_specialty_definition,
    sizeof(saved_hd_specialty_definition),
    &hd_specialty_definition_applied);
}

static void restore_hd_nyx_specialty_definition(void) {
  restore_definition_alias(
    hd_specialty_definition_address,
    saved_hd_specialty_definition,
    sizeof(saved_hd_specialty_definition),
    &hd_specialty_definition_applied);
}

static BOOL validate_hd_secondary_skill_definition(void) {
  char current[HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY];
  return
    hd_secondary_skill_definition_address != 0 &&
    safe_read(
      hd_secondary_skill_definition_address,
      current,
      sizeof(current)) &&
    bytes_equal(
      current,
      native_hd_definition_name,
      sizeof(current));
}

static BOOL validate_hd_swap_specialty_definition(void) {
  return
    hd_swap_specialty_definition_address != 0 &&
    validate_definition_alias(
      hd_swap_specialty_definition_address,
      native_hd_swap_specialty_definition_name,
      sizeof(native_hd_swap_specialty_definition_name));
}

static BOOL apply_hd_swap_nyx_specialty_definition(void) {
  return apply_definition_alias(
    hd_swap_specialty_definition_address,
    native_hd_swap_specialty_definition_name,
    nyx_hd_swap_specialty_definition_name,
    saved_hd_swap_specialty_definition,
    sizeof(saved_hd_swap_specialty_definition),
    &hd_swap_specialty_definition_applied);
}

static void restore_hd_swap_nyx_specialty_definition(void) {
  restore_definition_alias(
    hd_swap_specialty_definition_address,
    saved_hd_swap_specialty_definition,
    sizeof(saved_hd_swap_specialty_definition),
    &hd_swap_specialty_definition_applied);
}

static BOOL validate_hd_swap_skill_definition(void) {
  return
    hd_swap_skill_definition_address != 0 &&
    validate_definition_alias(
      hd_swap_skill_definition_address,
      native_hd_swap_skill_definition_name,
      sizeof(native_hd_swap_skill_definition_name));
}

static BOOL validate_swap_skill_control_definition_sites(void) {
#ifdef CHINESE_HOTA_R10
  return TRUE;
#else
  DWORD layout;
  DWORD control;
  DWORD native_pointer = (DWORD)hd_swap_skill_definition_address;

  if (hd_hota_base == 0 || native_pointer == 0) {
    return FALSE;
  }
  for (layout = 0; layout < HD_HOTA_SWAP_LAYOUT_COUNT; layout++) {
    for (control = 0; control < SWAP_SKILL_CONTROL_COUNT; control++) {
      BYTE opcode = 0;
      DWORD definition_pointer = 0;
      uintptr_t instruction =
        hd_hota_base +
        hd_swap_skill_control_push_rvas[layout][control];
      if (
        !safe_read(instruction, &opcode, sizeof(opcode)) ||
        opcode != 0x68 ||
        !safe_read(
          instruction + 1,
          &definition_pointer,
          sizeof(definition_pointer)) ||
        definition_pointer != native_pointer) {
        return FALSE;
      }
    }
  }
  return TRUE;
#endif
}

static BOOL get_general_text_entries(
  uintptr_t general_text_address,
  uintptr_t *plural_entry,
  uintptr_t *singular_entry) {
  DWORD general_text = 0;
  DWORD table = 0;
  if (
    plural_entry == NULL ||
    singular_entry == NULL ||
    !safe_read(
      general_text_address,
      &general_text,
      sizeof(general_text)) ||
    general_text == 0 ||
    !safe_read(
      (uintptr_t)general_text + GENERAL_TEXT_TABLE_OFFSET,
      &table,
      sizeof(table)) ||
    table == 0) {
    return FALSE;
  }
  *plural_entry = (uintptr_t)table + NECROMANCY_MESSAGE_PLURAL_OFFSET;
  *singular_entry = (uintptr_t)table + NECROMANCY_MESSAGE_SINGULAR_OFFSET;
  return TRUE;
}

static void restore_spiritism_message_table(DWORD index) {
  if (
    index >= GENERAL_TEXT_TABLE_COUNT ||
    !spiritism_message_table_applied[index]) {
    return;
  }
  safe_write(
    plural_message_entries[index],
    &saved_necromancy_message_plural[index],
    sizeof(saved_necromancy_message_plural[index]));
  safe_write(
    singular_message_entries[index],
    &saved_necromancy_message_singular[index],
    sizeof(saved_necromancy_message_singular[index]));
  spiritism_message_table_applied[index] = FALSE;
}

static void restore_spiritism_message(void) {
  DWORD index;
  for (index = 0; index < GENERAL_TEXT_TABLE_COUNT; index++) {
    restore_spiritism_message_table(index);
  }
  spiritism_message_applied = FALSE;
}

static BOOL apply_spiritism_message_table(
  DWORD index,
  const char *plural,
  const char *singular) {
  uintptr_t plural_entry = 0;
  uintptr_t singular_entry = 0;
  const char *saved_plural = NULL;
  const char *saved_singular = NULL;

  if (
    index >= GENERAL_TEXT_TABLE_COUNT ||
    !get_general_text_entries(
      general_text_addresses[index],
      &plural_entry,
      &singular_entry) ||
    !safe_read(
      plural_entry,
      &saved_plural,
      sizeof(saved_plural)) ||
    !safe_read(
      singular_entry,
      &saved_singular,
      sizeof(saved_singular)) ||
    !safe_write(plural_entry, &plural, sizeof(plural))) {
    return FALSE;
  }
  if (!safe_write(singular_entry, &singular, sizeof(singular))) {
    safe_write(plural_entry, &saved_plural, sizeof(saved_plural));
    return FALSE;
  }

  plural_message_entries[index] = plural_entry;
  singular_message_entries[index] = singular_entry;
  saved_necromancy_message_plural[index] = saved_plural;
  saved_necromancy_message_singular[index] = saved_singular;
  spiritism_message_table_applied[index] = TRUE;
  return TRUE;
}

static BOOL set_spiritism_message(BOOL active) {
  const char *plural = spiritism_message_plural;
  const char *singular = spiritism_message_singular;
  DWORD index;

  if (!active) {
    restore_spiritism_message();
    return TRUE;
  }
  if (spiritism_message_applied) {
    return TRUE;
  }
  for (index = 0; index < GENERAL_TEXT_TABLE_COUNT; index++) {
    if (!apply_spiritism_message_table(index, plural, singular)) {
      while (index != 0) {
        restore_spiritism_message_table(--index);
      }
      return FALSE;
    }
  }
  spiritism_message_applied = TRUE;
  return TRUE;
}

static int __thiscall direct_get_necromancy_creature(void *hero) {
#ifdef CHINESE_HOTA_R10
  if (is_spiritist_hero(hero)) {
    set_spiritism_message(TRUE);
    return CREATURE_ID_PIXIE;
  }
  set_spiritism_message(FALSE);
  return chained_get_necromancy_creature(hero);
#else
  int native_creature = chained_get_necromancy_creature(hero);

  if (is_spiritist_hero(hero)) {
    DWORD hero_id = 0;
    set_spiritism_message(TRUE);
    switch (native_creature) {
      case CREATURE_ID_WALKING_DEAD:
        return CREATURE_ID_FIRE_ELEMENTAL;
      case CREATURE_ID_WIGHT:
        return CREATURE_ID_EARTH_ELEMENTAL;
      case CREATURE_ID_LICH:
        return CREATURE_ID_PSYCHIC_ELEMENTAL;
      default:
        return
          read_hero_id(hero, &hero_id) && hero_id == HERO_ID_NYX
            ? CREATURE_ID_SPRITE
            : CREATURE_ID_PIXIE;
    }
  }
  set_spiritism_message(FALSE);
  return native_creature;
#endif
}

static float __thiscall direct_get_necromancy_power(
  void *hero,
  int capped) {
  float power = chained_get_necromancy_power(hero, capped);
  BYTE level = get_spiritism_level(hero);

  if (level != 0) {
    power += SPIRITISM_RATE_BONUS_PER_LEVEL * (float)level;
    if (capped && power > 1.0f) {
      power = 1.0f;
    }
  }
  return power;
}

#ifndef CHINESE_HOTA_R10
static BOOL is_native_secondary_skill_definition(const char *definition) {
  char current[SECONDARY_SKILL_DEFINITION_CAPACITY];

  if (
    definition == NULL ||
    !safe_read((uintptr_t)definition, current, sizeof(current))) {
    return FALSE;
  }
  return
    bytes_equal(
      current,
      native_definition_name,
      sizeof(current)) ||
    bytes_equal(
      current,
      native_hd_definition_name,
      sizeof(current));
}

static void *__fastcall direct_secondary_skill_control_constructor(
  void *control,
  void *,
  int x,
  int y,
  int width,
  int height,
  int id,
  const char *definition,
  int frame,
  int state,
  int hotkey,
  int flags,
  int hint) {
  BOOL spiritism_scope =
    InterlockedCompareExchange(&spiritism_ui_depth, 0, 0) > 0 &&
    small_skill_resources.ready &&
    is_native_secondary_skill_definition(definition) &&
    (
      (
        frame >= SPIRITISM_FRAME_FIRST &&
        frame < SPIRITISM_FRAME_FIRST + SPIRITISM_FRAME_COUNT
      ) ||
      (id == 0x4F && frame == 0)
    );
  const char *selected_definition =
    spiritism_scope ? spiritism_definition_name : definition;
  void *result = chained_secondary_skill_control_constructor(
    control,
    x,
    y,
    width,
    height,
    id,
    selected_definition,
    frame,
    state,
    hotkey,
    flags,
    hint);

  if (spiritism_scope) {
    append_text("small skill control=Spiritism native-loader scope id=");
    append_decimal((DWORD)id);
    append_text(" frame=");
    append_decimal((DWORD)frame);
    append_text("\r\n");
    write_log();
  }
  return result;
}

static void __fastcall direct_show_spiritism_popup(
  const char *text,
  int mode,
  int first,
  int second,
  int type,
  int frame,
  int fifth,
  int sixth,
  int seventh,
  int eighth,
  int ninth,
  int tenth) {
  char saved_definition[SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY];
  BOOL definition_applied = FALSE;
  BOOL spiritism_scope =
    InterlockedCompareExchange(&spiritism_ui_depth, 0, 0) > 0 &&
    type == 20 &&
    frame >= SPIRITISM_FRAME_FIRST &&
    frame < SPIRITISM_FRAME_FIRST + SPIRITISM_FRAME_COUNT &&
    large_skill_resources.ready &&
    apply_definition_alias(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      native_large_definition_name,
      spiritism_large_definition_name,
      saved_definition,
      sizeof(saved_definition),
      &definition_applied);
  const char *popup_text =
    spiritism_scope
      ? spiritism_text.description[frame - SPIRITISM_FRAME_FIRST]
      : text;

  chained_show_spiritism_popup(
    popup_text,
    mode,
    first,
    second,
    type,
    frame,
    fifth,
    sixth,
    seventh,
    eighth,
    ninth,
    tenth);
  restore_definition_alias(
    SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
    saved_definition,
    sizeof(saved_definition),
    &definition_applied);
  if (spiritism_scope) {
    append_text("hero/level-up popup=Spiritism native-loader scope\r\n");
    write_log();
  }
}
#endif

static void __fastcall direct_show_hero_dialog(
  int hero_id,
  int dismissable,
  int not_in_town,
  int right_click) {
  if (is_spiritist_hero_id((DWORD)hero_id)) {
    BOOL spiritism_scope = begin_spiritism_ui();
    if (spiritism_scope) {
      if (hero_id == HERO_ID_NYX) {
        apply_nyx_specialty_definition();
      }
      chained_show_hero_dialog(
        hero_id,
        dismissable,
        not_in_town,
        right_click);
      restore_nyx_specialty_definition();
      end_spiritism_ui();
      return;
    }
  }
  chained_show_hero_dialog(
    hero_id,
    dismissable,
    not_in_town,
    right_click);
}

static void __thiscall direct_level_up(void *hero) {
  if (is_spiritist_hero(hero)) {
    BOOL spiritism_scope = begin_spiritism_ui();
    if (spiritism_scope) {
      chained_level_up(hero);
      end_spiritism_ui();
      return;
    }
  }
  chained_level_up(hero);
}

static void *__stdcall direct_hd_show_hero_dialog(int hero_id) {
  void *dialog;
  BOOL spiritism_ui_ready;

  if (!is_spiritist_hero_id((DWORD)hero_id)) {
    return chained_hd_show_hero_dialog(hero_id);
  }
  spiritism_ui_ready = begin_spiritism_ui();
  if (hero_id == HERO_ID_NYX) {
    apply_hd_nyx_specialty_definition();
  }
  dialog = chained_hd_show_hero_dialog(hero_id);
  restore_hd_nyx_specialty_definition();
  if (spiritism_ui_ready) {
    end_spiritism_ui();
  }
  return dialog;
}

static BOOL get_swap_hero_state(
  void **heroes,
  BOOL spiritist_heroes[2],
  BOOL *use_nyx_specialty) {
  BOOL nyx_present = FALSE;
  BOOL specialty_frames_supported = TRUE;
  DWORD index;

  if (
    heroes == NULL ||
    spiritist_heroes == NULL ||
    use_nyx_specialty == NULL) {
    return FALSE;
  }
  for (index = 0; index < 2; index++) {
    void *hero = NULL;
    DWORD hero_id = 0;
    BYTE necromancy_level = 0;

    if (
      !safe_read(
        (uintptr_t)heroes + index * sizeof(hero),
        &hero,
        sizeof(hero)) ||
      !read_hero_id(hero, &hero_id) ||
      !read_necromancy_level(hero, &necromancy_level)) {
      return FALSE;
    }
    spiritist_heroes[index] =
      is_spiritist_hero_id(hero_id) &&
      necromancy_level >= 1 &&
      necromancy_level <= 3;
    if (hero_id == HERO_ID_NYX) {
      nyx_present = TRUE;
    }
    if (hero_id >= LEGACY_SPECIALTY_FRAME_COUNT) {
      specialty_frames_supported = FALSE;
    }
  }
  *use_nyx_specialty = nyx_present && specialty_frames_supported;
  return TRUE;
}

static BOOL set_swap_skill_control_definitions(
  void **heroes,
  const BOOL spiritist_heroes[2],
  DWORD *spiritism_control_count) {
#ifdef CHINESE_HOTA_R10
  (void)heroes;
  (void)spiritist_heroes;
  (void)spiritism_control_count;
  return FALSE;
#else
  GetSecondarySkillId get_secondary_skill_id =
    (GetSecondarySkillId)GET_SECONDARY_SKILL_ID_ADDRESS;
  BOOL spiritism_controls[SWAP_SKILL_CONTROL_COUNT] = {FALSE};
  DWORD layout;
  DWORD control;
  DWORD hero_index;
  DWORD native_pointer = (DWORD)hd_swap_skill_definition_address;
  DWORD spiritism_pointer =
    (DWORD)(uintptr_t)spiritism_hd_swap_skill_definition_name;

  if (
    heroes == NULL ||
    spiritist_heroes == NULL ||
    spiritism_control_count == NULL ||
    hd_hota_base == 0 ||
    native_pointer == 0 ||
    !swap_skill_resources.ready) {
    return FALSE;
  }
  *spiritism_control_count = 0;
  for (hero_index = 0; hero_index < 2; hero_index++) {
    void *hero = NULL;
    DWORD skill_count = 0;
    DWORD slot;

    if (
      !safe_read(
        (uintptr_t)heroes + hero_index * sizeof(hero),
        &hero,
        sizeof(hero)) ||
      hero == NULL ||
      !safe_read(
        (uintptr_t)hero + HERO_SECONDARY_SKILL_COUNT_OFFSET,
        &skill_count,
        sizeof(skill_count)) ||
      skill_count > SWAP_SKILL_CONTROLS_PER_HERO) {
      return FALSE;
    }
    if (!spiritist_heroes[hero_index]) {
      continue;
    }
    for (slot = 0; slot < skill_count; slot++) {
      if (
        get_secondary_skill_id(hero, (int)slot) ==
        SECONDARY_SKILL_NECROMANCY) {
        spiritism_controls[
          hero_index * SWAP_SKILL_CONTROLS_PER_HERO + slot] = TRUE;
        (*spiritism_control_count)++;
      }
    }
  }
  if (*spiritism_control_count == 0) {
    return FALSE;
  }
  for (layout = 0; layout < HD_HOTA_SWAP_LAYOUT_COUNT; layout++) {
    for (control = 0; control < SWAP_SKILL_CONTROL_COUNT; control++) {
      BYTE opcode = 0;
      DWORD current_pointer = 0;
      uintptr_t instruction =
        hd_hota_base +
        hd_swap_skill_control_push_rvas[layout][control];
      DWORD desired_pointer =
        spiritism_controls[control] ? spiritism_pointer : native_pointer;

      if (
        !safe_read(instruction, &opcode, sizeof(opcode)) ||
        opcode != 0x68 ||
        !safe_read(
          instruction + 1,
          &current_pointer,
          sizeof(current_pointer)) ||
        current_pointer != native_pointer ||
        !safe_write(
          instruction + 1,
          &desired_pointer,
          sizeof(desired_pointer))) {
        DWORD restore_layout;
        DWORD restore_control;
        for (
          restore_layout = 0;
          restore_layout < HD_HOTA_SWAP_LAYOUT_COUNT;
          restore_layout++) {
          for (
            restore_control = 0;
            restore_control < SWAP_SKILL_CONTROL_COUNT;
            restore_control++) {
            safe_write(
              hd_hota_base +
                hd_swap_skill_control_push_rvas[
                  restore_layout][restore_control] +
                1,
              &native_pointer,
              sizeof(native_pointer));
          }
        }
        return FALSE;
      }
    }
  }
  return TRUE;
#endif
}

static void restore_swap_skill_control_definitions(void) {
#ifndef CHINESE_HOTA_R10
  DWORD layout;
  DWORD control;
  DWORD native_pointer = (DWORD)hd_swap_skill_definition_address;

  if (hd_hota_base == 0 || native_pointer == 0) {
    return;
  }
  for (layout = 0; layout < HD_HOTA_SWAP_LAYOUT_COUNT; layout++) {
    for (control = 0; control < SWAP_SKILL_CONTROL_COUNT; control++) {
      safe_write(
        hd_hota_base +
          hd_swap_skill_control_push_rvas[layout][control] +
          1,
        &native_pointer,
        sizeof(native_pointer));
    }
  }
#endif
}

static void *__fastcall direct_swap_dialog_builder(
  void *dialog,
  void *,
  void **heroes) {
  BOOL spiritist_heroes[2] = {FALSE, FALSE};
  BOOL use_nyx_specialty = FALSE;
  BOOL control_pointers_applied = FALSE;
  BOOL nyx_specialty_applied = FALSE;
  DWORD spiritism_control_count = 0;
  void *result;

  if (get_swap_hero_state(
    heroes,
    spiritist_heroes,
    &use_nyx_specialty)) {
    if (spiritist_heroes[0] || spiritist_heroes[1]) {
      control_pointers_applied =
        set_swap_skill_control_definitions(
          heroes,
          spiritist_heroes,
          &spiritism_control_count);
    }
    if (use_nyx_specialty) {
      nyx_specialty_applied =
        apply_hd_swap_nyx_specialty_definition();
    }
  }
  result = chained_swap_dialog_builder(dialog, heroes);
  restore_hd_swap_nyx_specialty_definition();
  restore_swap_skill_control_definitions();
  append_text("exchange control pointers=");
  append_text(control_pointers_applied ? "Spiritism" : "native");
  append_text(" slots=");
  append_decimal(spiritism_control_count);
  append_text(" left=");
  append_text(spiritist_heroes[0] ? "Spiritism" : "native");
  append_text(" right=");
  append_text(spiritist_heroes[1] ? "Spiritism" : "native");
  append_text(" Nyx=");
  append_text(nyx_specialty_applied ? "active\r\n" : "native\r\n");
  write_log();
  return result;
}

#ifndef CHINESE_HOTA_R10
static int __fastcall direct_swap_get_secondary_skill_id(
  void *hero,
  void *,
  int slot) {
  int skill_id = chained_swap_get_secondary_skill_id(hero, slot);
  LONG spiritism_level = 0;

  if (skill_id == SECONDARY_SKILL_NECROMANCY) {
    spiritism_level = (LONG)get_spiritism_level(hero);
  }
  InterlockedExchange(
    &swap_right_click_spiritism_level,
    spiritism_level);
  return skill_id;
}

static void __fastcall direct_show_swap_skill_popup(
  const char *text,
  int mode,
  int first,
  int second,
  int type,
  int frame,
  int fifth,
  int sixth,
  int seventh,
  int eighth,
  int ninth,
  int tenth) {
  LONG spiritism_level = InterlockedExchange(
    &swap_right_click_spiritism_level,
    0);
  char saved_definition[SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY];
  BOOL definition_applied = FALSE;
  BOOL spiritism_popup =
    spiritism_level >= 1 &&
    spiritism_level <= 3 &&
    type == 20 &&
    frame == SPIRITISM_FRAME_FIRST + spiritism_level - 1 &&
    large_skill_resources.ready &&
    apply_definition_alias(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      native_large_definition_name,
      spiritism_large_definition_name,
      saved_definition,
      sizeof(saved_definition),
      &definition_applied);
  const char *popup_text =
    spiritism_popup
      ? spiritism_text.description[spiritism_level - 1]
      : text;

  chained_show_swap_skill_popup(
    popup_text,
    mode,
    first,
    second,
    type,
    frame,
    fifth,
    sixth,
    seventh,
    eighth,
    ninth,
    tenth);
  restore_definition_alias(
    SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
    saved_definition,
    sizeof(saved_definition),
    &definition_applied);
  if (spiritism_level != 0) {
    append_text("exchange right-click popup=");
    append_text(
      spiritism_popup
        ? "Spiritism native-loader scope\r\n"
        : "native fallback\r\n");
    write_log();
  }
}

static int __fastcall direct_hota_hermit_shack(
  void *shack,
  void *,
  void *hero,
  int second,
  int third,
  int fourth,
  int fifth) {
  BOOL spiritist = is_spiritist_hero(hero);
  int result;

  if (spiritist) {
    InterlockedIncrement(&hermit_spiritism_depth);
  }
  result = chained_hota_hermit_shack(
    shack,
    hero,
    second,
    third,
    fourth,
    fifth);
  if (spiritist) {
    InterlockedDecrement(&hermit_spiritism_depth);
  }
  return result;
}

static void __fastcall direct_show_hermit_skill_popup(
  const char *text,
  int mode,
  int first,
  int second,
  int type,
  int frame,
  int fifth,
  int sixth,
  int seventh,
  int eighth,
  int ninth,
  int tenth) {
  char saved_definition[SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY];
  BOOL definition_applied = FALSE;
  BOOL spiritism_scope =
    InterlockedCompareExchange(
      &hermit_spiritism_depth,
      0,
      0) > 0 &&
    type == 20 &&
    frame >= SPIRITISM_FRAME_FIRST &&
    frame < SPIRITISM_FRAME_FIRST + SPIRITISM_FRAME_COUNT &&
    large_skill_resources.ready &&
    apply_definition_alias(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      native_large_definition_name,
      spiritism_large_definition_name,
      saved_definition,
      sizeof(saved_definition),
      &definition_applied);
  const char *popup_text =
    spiritism_scope
      ? spiritism_text.description[frame - SPIRITISM_FRAME_FIRST]
      : text;

  chained_show_swap_skill_popup(
    popup_text,
    mode,
    first,
    second,
    type,
    frame,
    fifth,
    sixth,
    seventh,
    eighth,
    ninth,
    tenth);
  restore_definition_alias(
    SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
    saved_definition,
    sizeof(saved_definition),
    &definition_applied);
  append_text("Hermit Shack popup=");
  append_text(
    spiritism_scope
      ? "Spiritism native-loader scope\r\n"
      : "native\r\n");
  write_log();
}
#endif

static void append_code_bytes(const char *label, uintptr_t address) {
  BYTE bytes[8];
  DWORD index;
  append_text(label);
  append_text("=");
  if (!safe_read(address, bytes, sizeof(bytes))) {
    append_text("<unreadable>\r\n");
    return;
  }
  for (index = 0; index < sizeof(bytes); index++) {
    if (index != 0) {
      append_char(' ');
    }
    append_hex_digit((BYTE)(bytes[index] >> 4));
    append_hex_digit((BYTE)(bytes[index] & 0x0F));
  }
  append_text("\r\n");
}

static BOOL get_relative_target(
  uintptr_t address,
  BYTE expected_opcode,
  uintptr_t *target) {
  BYTE instruction[5];
  int32_t displacement;

  if (
    target == NULL ||
    !safe_read(address, instruction, sizeof(instruction)) ||
    instruction[0] != expected_opcode) {
    return FALSE;
  }
  displacement =
    (int32_t)(
      (DWORD)instruction[1] |
      ((DWORD)instruction[2] << 8) |
      ((DWORD)instruction[3] << 16) |
      ((DWORD)instruction[4] << 24));
  *target = address + 5 + displacement;
  return TRUE;
}

static BOOL get_absolute_mov_target(
  uintptr_t address,
  uintptr_t *target) {
  BYTE instruction[5];

  if (
    target == NULL ||
    !safe_read(address, instruction, sizeof(instruction)) ||
    instruction[0] != 0xB8) {
    return FALSE;
  }
  *target =
    (uintptr_t)(
      (DWORD)instruction[1] |
      ((DWORD)instruction[2] << 8) |
      ((DWORD)instruction[3] << 16) |
      ((DWORD)instruction[4] << 24));
  return TRUE;
}

static BOOL build_relative_instruction(
  BYTE *instruction,
  uintptr_t address,
  BYTE opcode,
  uintptr_t target) {
  intptr_t displacement;

  if (instruction == NULL) {
    return FALSE;
  }
  displacement =
    (intptr_t)target -
    (intptr_t)(address + 5);
  if (displacement < INT32_MIN || displacement > INT32_MAX) {
    return FALSE;
  }
  instruction[0] = opcode;
  instruction[1] = (BYTE)(displacement & 0xFF);
  instruction[2] = (BYTE)((displacement >> 8) & 0xFF);
  instruction[3] = (BYTE)((displacement >> 16) & 0xFF);
  instruction[4] = (BYTE)((displacement >> 24) & 0xFF);
  return TRUE;
}

static void write_u32(BYTE *destination, DWORD value) {
  destination[0] = (BYTE)(value & 0xFF);
  destination[1] = (BYTE)((value >> 8) & 0xFF);
  destination[2] = (BYTE)((value >> 16) & 0xFF);
  destination[3] = (BYTE)((value >> 24) & 0xFF);
}

static BOOL prepare_relative_patch(
  uintptr_t address,
  BYTE expected_opcode,
  void *replacement,
  uintptr_t expected_target,
  uintptr_t *original_target,
  CodePatch *patch) {
  if (
    patch == NULL ||
    original_target == NULL ||
    !safe_read(address, patch->original, 5) ||
    !get_relative_target(address, expected_opcode, original_target) ||
    (expected_target != 0 && *original_target != expected_target)) {
    return FALSE;
  }
  if (
    !build_relative_instruction(
      patch->replacement,
      address,
      expected_opcode,
      (uintptr_t)replacement)) {
    return FALSE;
  }
  patch->address = address;
  patch->size = 5;
  return TRUE;
}

static BOOL prepare_absolute_mov_patch(
  uintptr_t address,
  uintptr_t expected_target,
  void *replacement,
  CodePatch *patch) {
  BYTE live[5];
  DWORD target;
  DWORD replacement_target = (DWORD)(uintptr_t)replacement;
  DWORD index;

  if (
    patch == NULL ||
    !safe_read(address, live, sizeof(live)) ||
    live[0] != 0xB8) {
    return FALSE;
  }
  target =
    (DWORD)live[1] |
    ((DWORD)live[2] << 8) |
    ((DWORD)live[3] << 16) |
    ((DWORD)live[4] << 24);
  if (target != (DWORD)expected_target) {
    return FALSE;
  }
  patch->address = address;
  patch->size = sizeof(live);
  for (index = 0; index < sizeof(live); index++) {
    patch->original[index] = live[index];
    patch->replacement[index] = live[index];
  }
  write_u32(patch->replacement + 1, replacement_target);
  return TRUE;
}

static BOOL entry_patch_target_ready(
  uintptr_t address,
  const BYTE *expected_prologue,
  DWORD prologue_size) {
  BYTE live[CODE_PATCH_CAPACITY];
  uintptr_t target;

  if (
    expected_prologue == NULL ||
    prologue_size < 5 ||
    prologue_size > sizeof(live) ||
    !safe_read(address, live, prologue_size)) {
    return FALSE;
  }
  if (live[0] == 0xE9) {
    return get_relative_target(address, 0xE9, &target);
  }
  return bytes_equal(
    (const char *)live,
    (const char *)expected_prologue,
    prologue_size);
}

static BOOL prepare_entry_patch(
  uintptr_t address,
  const BYTE *expected_prologue,
  DWORD prologue_size,
  void *replacement,
  uintptr_t *original_target,
  CodePatch *patch) {
  BYTE live[CODE_PATCH_CAPACITY];
  BYTE *trampoline;
  DWORD index;

  if (
    expected_prologue == NULL ||
    original_target == NULL ||
    patch == NULL ||
    prologue_size < 5 ||
    prologue_size > sizeof(live) ||
    !safe_read(address, live, prologue_size)) {
    return FALSE;
  }
  if (live[0] == 0xE9) {
    return prepare_relative_patch(
      address,
      0xE9,
      replacement,
      0,
      original_target,
      patch);
  }
  if (
    !bytes_equal(
      (const char *)live,
      (const char *)expected_prologue,
      prologue_size)) {
    return FALSE;
  }

  trampoline = (BYTE *)VirtualAlloc(
    NULL,
    prologue_size + 5,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE);
  if (trampoline == NULL) {
    return FALSE;
  }
  for (index = 0; index < prologue_size; index++) {
    trampoline[index] = live[index];
  }
  if (
    !build_relative_instruction(
      trampoline + prologue_size,
      (uintptr_t)trampoline + prologue_size,
      0xE9,
      address + prologue_size)) {
    VirtualFree(trampoline, 0, MEM_RELEASE);
    return FALSE;
  }
  FlushInstructionCache(
    GetCurrentProcess(),
    trampoline,
    prologue_size + 5);

  patch->address = address;
  patch->size = prologue_size;
  for (index = 0; index < prologue_size; index++) {
    patch->original[index] = live[index];
    patch->replacement[index] = 0x90;
  }
  if (
    !build_relative_instruction(
      patch->replacement,
      address,
      0xE9,
      (uintptr_t)replacement)) {
    VirtualFree(trampoline, 0, MEM_RELEASE);
    return FALSE;
  }
  *original_target = (uintptr_t)trampoline;
  return TRUE;
}

static BOOL prepare_inspection_guard_patch(
  HMODULE hota,
  uintptr_t site_rva,
  uintptr_t continue_rva,
  uintptr_t return_rva,
  CodePatch *patch) {
  BYTE live[CODE_PATCH_CAPACITY];
  BYTE *stub;
  DWORD pointer_address;
  DWORD index;
  uintptr_t module = (uintptr_t)hota;
  uintptr_t site = module + site_rva;
  uintptr_t expected_pointer = module + HOTA_INSPECTED_HERO_POINTER_RVA;

  if (
    hota == NULL ||
    patch == NULL ||
    !safe_read(site, live, sizeof(live))) {
    return FALSE;
  }
  pointer_address =
    (DWORD)live[2] |
    ((DWORD)live[3] << 8) |
    ((DWORD)live[4] << 16) |
    ((DWORD)live[5] << 24);
  if (
    live[0] != 0x8B ||
    live[1] != 0x0D ||
    pointer_address != (DWORD)expected_pointer ||
    live[6] != 0x8B ||
    live[7] != 0x49 ||
    live[8] != 0x1A) {
    return FALSE;
  }

  stub = (BYTE *)VirtualAlloc(
    NULL,
    32,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE);
  if (stub == NULL) {
    return FALSE;
  }

  /* Preserve the original load/dereference and skip it only for NULL. */
  stub[0] = 0x8B;
  stub[1] = 0x0D;
  write_u32(stub + 2, pointer_address);
  stub[6] = 0x85;
  stub[7] = 0xC9;
  stub[8] = 0x75;
  stub[9] = 0x05;
  if (
    !build_relative_instruction(
      stub + 10,
      (uintptr_t)stub + 10,
      0xE9,
      module + return_rva)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }
  stub[15] = 0x8B;
  stub[16] = 0x49;
  stub[17] = 0x1A;
  if (
    !build_relative_instruction(
      stub + 18,
      (uintptr_t)stub + 18,
      0xE9,
      module + continue_rva)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }
  FlushInstructionCache(GetCurrentProcess(), stub, 23);

  patch->address = site;
  patch->size = sizeof(live);
  for (index = 0; index < sizeof(live); index++) {
    patch->original[index] = live[index];
    patch->replacement[index] = 0x90;
  }
  if (
    !build_relative_instruction(
      patch->replacement,
      site,
      0xE9,
      (uintptr_t)stub)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }
  return TRUE;
}

static BOOL prepare_necromancy_power_patch(
  uintptr_t *original_target,
  CodePatch *patch) {
  static const BYTE expected_prologue[
    GET_NECROMANCY_POWER_PROLOGUE_SIZE] = {
      0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08,
    };
  BYTE live[GET_NECROMANCY_POWER_PROLOGUE_SIZE];
  BYTE *trampoline;
  DWORD index;

  if (
    original_target == NULL ||
    patch == NULL ||
    !safe_read(GET_NECROMANCY_POWER_ADDRESS, live, sizeof(live))) {
    return FALSE;
  }
  if (live[0] == 0xE9) {
    BOOL prepared = prepare_relative_patch(
      GET_NECROMANCY_POWER_ADDRESS,
      0xE9,
      (void *)direct_get_necromancy_power,
      0,
      original_target,
      patch);
    if (prepared) {
      append_text("necromancy power backend=relative chain\r\n");
    }
    return prepared;
  }
  if (
    !bytes_equal(
      (const char *)live,
      (const char *)expected_prologue,
      sizeof(live))) {
    return FALSE;
  }

  trampoline = (BYTE *)VirtualAlloc(
    NULL,
    GET_NECROMANCY_POWER_PROLOGUE_SIZE + 5,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE);
  if (trampoline == NULL) {
    return FALSE;
  }
  for (index = 0; index < sizeof(live); index++) {
    trampoline[index] = live[index];
  }
  if (
    !build_relative_instruction(
      trampoline + GET_NECROMANCY_POWER_PROLOGUE_SIZE,
      (uintptr_t)trampoline + GET_NECROMANCY_POWER_PROLOGUE_SIZE,
      0xE9,
      GET_NECROMANCY_POWER_ADDRESS +
        GET_NECROMANCY_POWER_PROLOGUE_SIZE)) {
    return FALSE;
  }
  FlushInstructionCache(
    GetCurrentProcess(),
    trampoline,
    GET_NECROMANCY_POWER_PROLOGUE_SIZE + 5);

  patch->address = GET_NECROMANCY_POWER_ADDRESS;
  patch->size = GET_NECROMANCY_POWER_PROLOGUE_SIZE;
  for (index = 0; index < sizeof(live); index++) {
    patch->original[index] = live[index];
  }
  if (
    !build_relative_instruction(
      patch->replacement,
      GET_NECROMANCY_POWER_ADDRESS,
      0xE9,
      (uintptr_t)direct_get_necromancy_power)) {
    return FALSE;
  }
  patch->replacement[5] = 0x90;
  *original_target = (uintptr_t)trampoline;
  append_text("necromancy power backend=validated entry trampoline\r\n");
  return TRUE;
}

static BOOL apply_code_patches(
  CodePatch *patches,
  DWORD count) {
  DWORD applied = 0;
  BYTE verified[CODE_PATCH_CAPACITY];

  while (applied < count) {
    if (
      !safe_write(
        patches[applied].address,
        patches[applied].replacement,
        patches[applied].size) ||
      !safe_read(
        patches[applied].address,
        verified,
        patches[applied].size) ||
      !bytes_equal(
        (const char *)verified,
        (const char *)patches[applied].replacement,
        patches[applied].size)) {
      safe_write(
        patches[applied].address,
        patches[applied].original,
        patches[applied].size);
      while (applied != 0) {
        applied--;
        safe_write(
          patches[applied].address,
          patches[applied].original,
          patches[applied].size);
      }
      return FALSE;
    }
    applied++;
  }
  return TRUE;
}

static BOOL runtime_targets_ready(
  HMODULE *hota,
  HMODULE *hd_hota,
  uintptr_t *hd_dialog,
  uintptr_t *hd_call_1,
  uintptr_t *hd_call_2) {
  static const BYTE power_prologue[
    GET_NECROMANCY_POWER_PROLOGUE_SIZE] = {
      0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08,
    };
  BYTE live_power[GET_NECROMANCY_POWER_PROLOGUE_SIZE];
  uintptr_t target = 0;
  HMODULE hota_module = GetModuleHandleA("HotA.dll");
  HMODULE module = GetModuleHandleA("HD_HOTA.dll");

  if (
    hota == NULL ||
    hota_module == NULL ||
    module == NULL ||
    !safe_read(
      GET_NECROMANCY_POWER_ADDRESS,
      live_power,
      sizeof(live_power)) ||
    (
      live_power[0] != 0xE9 &&
      !bytes_equal(
        (const char *)live_power,
        (const char *)power_prologue,
        sizeof(live_power))
    ) ||
    !get_relative_target(
      GET_NECROMANCY_CREATURE_ADDRESS,
      0xE9,
      &target) ||
    !get_relative_target(
      SHOW_HERO_DIALOG_ADDRESS,
      0xE9,
      &target) ||
    !get_relative_target(
      LEVEL_UP_ADDRESS,
      0xE9,
      &target) ||
    !get_relative_target(
      SWAP_DIALOG_BUILDER_ADDRESS,
      0xE9,
      &target)
#ifndef CHINESE_HOTA_R10
    ||
    !entry_patch_target_ready(
      SECONDARY_SKILL_CONTROL_CONSTRUCTOR_ADDRESS,
      secondary_skill_control_constructor_prologue,
      sizeof(secondary_skill_control_constructor_prologue)) ||
    !entry_patch_target_ready(
      SHOW_POPUP_ADDRESS,
      show_popup_prologue,
      sizeof(show_popup_prologue)) ||
    !get_relative_target(
      SWAP_SKILL_ID_CALL_ADDRESS,
      0xE8,
      &target) ||
    target != GET_SECONDARY_SKILL_ID_ADDRESS ||
    !get_relative_target(
      SWAP_SKILL_POPUP_CALL_ADDRESS,
      0xE8,
      &target) ||
    target != SHOW_POPUP_ADDRESS ||
    !entry_patch_target_ready(
      (uintptr_t)hota_module + HOTA_HERMIT_SHACK_RVA,
      hota_hermit_shack_prologue,
      sizeof(hota_hermit_shack_prologue)) ||
    !get_absolute_mov_target(
      (uintptr_t)hota_module + HOTA_HERMIT_POPUP_MOV_RVA,
      &target) ||
    target != SHOW_POPUP_ADDRESS
#endif
    ) {
    return FALSE;
  }
  *hota = hota_module;
  *hd_hota = module;
  *hd_dialog = (uintptr_t)module + HD_HOTA_HERO_DIALOG_RVA;
  *hd_call_1 = (uintptr_t)module + HD_HOTA_HERO_DIALOG_CALL_1_RVA;
  *hd_call_2 = (uintptr_t)module + HD_HOTA_HERO_DIALOG_CALL_2_RVA;
  if (
    !get_relative_target(*hd_call_1, 0xE8, &target) ||
    target != *hd_dialog ||
    !get_relative_target(*hd_call_2, 0xE8, &target) ||
    target != *hd_dialog) {
    return FALSE;
  }
  return TRUE;
}

static BOOL wait_for_runtime_targets(
  HMODULE *hota,
  HMODULE *hd_hota,
  uintptr_t *hd_dialog,
  uintptr_t *hd_call_1,
  uintptr_t *hd_call_2) {
#ifdef CHINESE_HOTA_R10
  BYTE previous[36];
  BYTE current[36];
#else
  BYTE previous[67];
  BYTE current[67];
#endif
  BOOL have_previous = FALSE;
  DWORD stable_checks = 0;
  DWORD attempt;

  Sleep(2000);
  for (attempt = 0; attempt < 40; attempt++) {
    if (
      runtime_targets_ready(
        hota,
        hd_hota,
        hd_dialog,
        hd_call_1,
        hd_call_2) &&
      safe_read(
        GET_NECROMANCY_POWER_ADDRESS,
        current,
        GET_NECROMANCY_POWER_PROLOGUE_SIZE) &&
      safe_read(GET_NECROMANCY_CREATURE_ADDRESS, current + 6, 5) &&
      safe_read(SHOW_HERO_DIALOG_ADDRESS, current + 11, 5) &&
      safe_read(LEVEL_UP_ADDRESS, current + 16, 5) &&
      safe_read(*hd_call_1, current + 21, 5) &&
      safe_read(*hd_call_2, current + 26, 5) &&
      safe_read(SWAP_DIALOG_BUILDER_ADDRESS, current + 31, 5)
#ifndef CHINESE_HOTA_R10
      &&
      safe_read(
        SECONDARY_SKILL_CONTROL_CONSTRUCTOR_ADDRESS,
        current + 36,
        SECONDARY_SKILL_CONTROL_CONSTRUCTOR_PROLOGUE_SIZE) &&
      safe_read(
        SHOW_POPUP_ADDRESS,
        current + 41,
        SHOW_POPUP_PROLOGUE_SIZE) &&
      safe_read(SWAP_SKILL_ID_CALL_ADDRESS, current + 46, 5) &&
      safe_read(SWAP_SKILL_POPUP_CALL_ADDRESS, current + 51, 5) &&
      safe_read(
        (uintptr_t)*hota + HOTA_HERMIT_SHACK_RVA,
        current + 56,
        HOTA_HERMIT_SHACK_PROLOGUE_SIZE) &&
      safe_read(
        (uintptr_t)*hota + HOTA_HERMIT_POPUP_MOV_RVA,
        current + 62,
        5)
#endif
      ) {
      if (
        have_previous &&
        bytes_equal(
          (const char *)previous,
          (const char *)current,
          sizeof(current))) {
        stable_checks++;
        if (stable_checks >= 4) {
          return TRUE;
        }
      } else {
        DWORD index;
        for (index = 0; index < sizeof(current); index++) {
          previous[index] = current[index];
        }
        have_previous = TRUE;
        stable_checks = 0;
      }
    } else {
      have_previous = FALSE;
      stable_checks = 0;
    }
    Sleep(250);
  }
  return FALSE;
}

static BOOL install_hooks(void) {
  CodePatch patches[HOOK_PATCH_COUNT];
  HMODULE hota = NULL;
  HMODULE hd_hota = NULL;
  uintptr_t hd_dialog_address = 0;
  uintptr_t hd_call_1 = 0;
  uintptr_t hd_call_2 = 0;
  uintptr_t target_necromancy = 0;
  uintptr_t target_necromancy_power = 0;
  uintptr_t target_dialog = 0;
  uintptr_t target_level_up = 0;
  uintptr_t target_hd_1 = 0;
  uintptr_t target_hd_2 = 0;
  uintptr_t target_swap_dialog_builder = 0;
#ifndef CHINESE_HOTA_R10
  uintptr_t target_secondary_skill_control_constructor = 0;
  uintptr_t target_show_spiritism_popup = 0;
  uintptr_t target_swap_get_secondary_skill_id = 0;
  uintptr_t target_show_swap_skill_popup = 0;
  uintptr_t target_hota_hermit_shack = 0;
#endif
  BOOL prepared;

  if (!wait_for_runtime_targets(
    &hota,
    &hd_hota,
    &hd_dialog_address,
    &hd_call_1,
    &hd_call_2)) {
    append_text("direct hooks=runtime targets did not stabilize\r\n");
    return FALSE;
  }
  hd_secondary_skill_definition_address =
    (uintptr_t)hd_hota + HD_HOTA_SECONDARY_SKILL_DEFINITION_RVA;
  hd_hota_base = (uintptr_t)hd_hota;
  hd_specialty_definition_address =
    (uintptr_t)hd_hota + HD_HOTA_SPECIALTY_DEFINITION_RVA;
  hd_swap_specialty_definition_address =
    (uintptr_t)hd_hota + HD_HOTA_SWAP_SPECIALTY_DEFINITION_RVA;
  hd_swap_skill_definition_address =
    (uintptr_t)hd_hota + HD_HOTA_SWAP_SKILL_DEFINITION_RVA;

  append_code_bytes(
    "live GetNecromancyCreatureId",
    GET_NECROMANCY_CREATURE_ADDRESS);
  append_code_bytes(
    "live GetNecromancyPower",
    GET_NECROMANCY_POWER_ADDRESS);
  append_code_bytes("live ShowHeroDialog", SHOW_HERO_DIALOG_ADDRESS);
  append_code_bytes("live LevelUp", LEVEL_UP_ADDRESS);
  append_code_bytes("live HD hero selection call 1", hd_call_1);
  append_code_bytes("live HD hero selection call 2", hd_call_2);
  append_code_bytes(
    "live HD exchange dialog builder",
    SWAP_DIALOG_BUILDER_ADDRESS);
#ifndef CHINESE_HOTA_R10
  append_code_bytes(
    "live secondary skill control constructor",
    SECONDARY_SKILL_CONTROL_CONSTRUCTOR_ADDRESS);
  append_code_bytes("live skill popup", SHOW_POPUP_ADDRESS);
  append_code_bytes(
    "live HD exchange skill-id call",
    SWAP_SKILL_ID_CALL_ADDRESS);
  append_code_bytes(
    "live HD exchange skill-popup call",
    SWAP_SKILL_POPUP_CALL_ADDRESS);
  append_code_bytes(
    "live HotA Hermit Shack",
    (uintptr_t)hota + HOTA_HERMIT_SHACK_RVA);
  append_code_bytes(
    "live HotA Hermit Shack popup",
    (uintptr_t)hota + HOTA_HERMIT_POPUP_MOV_RVA);
#endif
  append_code_bytes(
    "live hero inspection dereference 1",
    (uintptr_t)hota + HOTA_INSPECTION_GUARD_1_SITE_RVA);
  append_code_bytes(
    "live hero inspection dereference 2",
    (uintptr_t)hota + HOTA_INSPECTION_GUARD_2_SITE_RVA);
  append_text("native small skill literal=");
  append_text(
    validate_secondary_skill_definition()
      ? "ready\r\n"
      : "unexpected filename literal\r\n");
  append_text("native large skill literal=");
  append_text(
    validate_secondary_skill_large_definition()
      ? "ready\r\n"
      : "unexpected filename literal\r\n");
  append_text("HD hero selection skill literal=");
  append_text(
    validate_hd_secondary_skill_definition()
      ? "ready\r\n"
      : "unavailable or unexpected filename literal\r\n");
  append_text("scoped Nyx specialty aliases=");
  append_text(
    validate_specialty_definition() &&
      validate_hd_specialty_definition() &&
      validate_hd_swap_specialty_definition()
      ? "ready\r\n"
      : "unavailable or unexpected filename literal\r\n");
  append_text("HD exchange skill literal=");
  append_text(
    validate_hd_swap_skill_definition()
      ? "ready\r\n"
      : "unavailable or unexpected filename literal\r\n");
  append_text("exchange control pointer sites=");
  append_text(
    validate_swap_skill_control_definition_sites()
      ? "ready\r\n"
      : "unavailable or unexpected constructor operands\r\n");
  append_text("post-battle message tables=");
  {
    uintptr_t plural = 0;
    uintptr_t singular = 0;
    BOOL primary_ready = get_general_text_entries(
      GENERAL_TEXT_ADDRESS,
      &plural,
      &singular);
    BOOL adventure_ready = get_general_text_entries(
      ADVENTURE_GENERAL_TEXT_ADDRESS,
      &plural,
      &singular);
    append_text(
      primary_ready && adventure_ready
        ? "primary+adventure ready\r\n"
        : "one or more unavailable\r\n");
  }

  prepared =
    prepare_necromancy_power_patch(
      &target_necromancy_power,
      &patches[0]) &&
    prepare_relative_patch(
      GET_NECROMANCY_CREATURE_ADDRESS,
      0xE9,
      (void *)direct_get_necromancy_creature,
      0,
      &target_necromancy,
      &patches[1]) &&
    prepare_relative_patch(
      SHOW_HERO_DIALOG_ADDRESS,
      0xE9,
      (void *)direct_show_hero_dialog,
      0,
      &target_dialog,
      &patches[2]) &&
    prepare_relative_patch(
      LEVEL_UP_ADDRESS,
      0xE9,
      (void *)direct_level_up,
      0,
      &target_level_up,
      &patches[3]) &&
    prepare_relative_patch(
      hd_call_1,
      0xE8,
      (void *)direct_hd_show_hero_dialog,
      hd_dialog_address,
      &target_hd_1,
      &patches[4]) &&
    prepare_relative_patch(
      hd_call_2,
      0xE8,
      (void *)direct_hd_show_hero_dialog,
      hd_dialog_address,
      &target_hd_2,
      &patches[5]) &&
    prepare_inspection_guard_patch(
      hota,
      HOTA_INSPECTION_GUARD_1_SITE_RVA,
      HOTA_INSPECTION_GUARD_1_CONTINUE_RVA,
      HOTA_INSPECTION_GUARD_1_RETURN_RVA,
      &patches[6]) &&
    prepare_inspection_guard_patch(
      hota,
      HOTA_INSPECTION_GUARD_2_SITE_RVA,
      HOTA_INSPECTION_GUARD_2_CONTINUE_RVA,
      HOTA_INSPECTION_GUARD_2_RETURN_RVA,
      &patches[7]) &&
    prepare_relative_patch(
      SWAP_DIALOG_BUILDER_ADDRESS,
      0xE9,
      (void *)direct_swap_dialog_builder,
      0,
      &target_swap_dialog_builder,
      &patches[8])
#ifndef CHINESE_HOTA_R10
    &&
    prepare_entry_patch(
      SECONDARY_SKILL_CONTROL_CONSTRUCTOR_ADDRESS,
      secondary_skill_control_constructor_prologue,
      sizeof(secondary_skill_control_constructor_prologue),
      (void *)direct_secondary_skill_control_constructor,
      &target_secondary_skill_control_constructor,
      &patches[9]) &&
    prepare_entry_patch(
      SHOW_POPUP_ADDRESS,
      show_popup_prologue,
      sizeof(show_popup_prologue),
      (void *)direct_show_spiritism_popup,
      &target_show_spiritism_popup,
      &patches[10]) &&
    prepare_relative_patch(
      SWAP_SKILL_ID_CALL_ADDRESS,
      0xE8,
      (void *)direct_swap_get_secondary_skill_id,
      GET_SECONDARY_SKILL_ID_ADDRESS,
      &target_swap_get_secondary_skill_id,
      &patches[11]) &&
    prepare_relative_patch(
      SWAP_SKILL_POPUP_CALL_ADDRESS,
      0xE8,
      (void *)direct_show_swap_skill_popup,
      SHOW_POPUP_ADDRESS,
      &target_show_swap_skill_popup,
      &patches[12]) &&
    prepare_entry_patch(
      (uintptr_t)hota + HOTA_HERMIT_SHACK_RVA,
      hota_hermit_shack_prologue,
      sizeof(hota_hermit_shack_prologue),
      (void *)direct_hota_hermit_shack,
      &target_hota_hermit_shack,
      &patches[13]) &&
    prepare_absolute_mov_patch(
      (uintptr_t)hota + HOTA_HERMIT_POPUP_MOV_RVA,
      SHOW_POPUP_ADDRESS,
      (void *)direct_show_hermit_skill_popup,
      &patches[14])
#endif
    &&
    target_hd_1 == target_hd_2 &&
    validate_secondary_skill_definition() &&
    validate_secondary_skill_large_definition() &&
    validate_hd_secondary_skill_definition() &&
    validate_specialty_definition() &&
    validate_hd_specialty_definition() &&
    validate_hd_swap_specialty_definition() &&
    validate_hd_swap_skill_definition() &&
    validate_swap_skill_control_definition_sites();
  if (!prepared) {
    append_text("direct hooks=target validation failed\r\n");
    return FALSE;
  }

  chained_get_necromancy_creature =
    (GetNecromancyCreature)target_necromancy;
  chained_get_necromancy_power =
    (GetNecromancyPower)target_necromancy_power;
  chained_show_hero_dialog = (ShowHeroDialog)target_dialog;
  chained_level_up = (LevelUp)target_level_up;
  chained_hd_show_hero_dialog = (HdShowHeroDialog)target_hd_1;
  chained_swap_dialog_builder =
    (SwapDialogBuilder)target_swap_dialog_builder;
#ifndef CHINESE_HOTA_R10
  chained_secondary_skill_control_constructor =
    (SecondarySkillControlConstructor)
      target_secondary_skill_control_constructor;
  chained_show_spiritism_popup =
    (ShowSwapSkillPopup)target_show_spiritism_popup;
  chained_swap_get_secondary_skill_id =
    (GetSecondarySkillId)target_swap_get_secondary_skill_id;
  chained_show_swap_skill_popup =
    (ShowSwapSkillPopup)target_show_swap_skill_popup;
  chained_hota_hermit_shack =
    (HotaHermitShack)target_hota_hermit_shack;
#endif

  prepared = apply_code_patches(patches, HOOK_PATCH_COUNT);
  append_text("necromancy hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("necromancy rate hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("hero dialog hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("level-up hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
#ifndef CHINESE_HOTA_R10
  append_text("small skill native-loader hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("large skill native-loader hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
#else
  append_text("small skill native-loader hook=disabled\r\n");
  append_text("large skill native-loader hook=disabled\r\n");
#endif
  append_text("HD hero selection hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("HD exchange dialog hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
#ifndef CHINESE_HOTA_R10
  append_text(
    "HD exchange skill right-click call hooks=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
#else
  append_text("HD exchange skill right-click call hooks=disabled\r\n");
#endif
#ifndef CHINESE_HOTA_R10
  append_text("Hermit Shack entry+popup hooks=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
#else
  append_text("Hermit skill display hook=disabled\r\n");
#endif
  append_text("hero inspection null guards=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text(
    "hook backend=relative chaining; no exchange event entry\r\n");
  return prepared;
}

static void write_log(void) {
  char path[MAX_PATH];
  DWORD length = GetModuleFileNameA(self_module, path, MAX_PATH);
  HANDLE file;
  DWORD written;

  if (length == 0 || length >= MAX_PATH) {
    return;
  }
  while (length != 0 && path[length - 1] != '\\' && path[length - 1] != '/') {
    length--;
  }
  {
    const char *name = "ConfluxSpiritism.log";
    while (*name != '\0' && length + 1 < MAX_PATH) {
      path[length++] = *name++;
    }
    path[length] = '\0';
  }
  file = CreateFileA(
    path,
    GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
  if (file == INVALID_HANDLE_VALUE) {
    return;
  }
  WriteFile(file, log_buffer, log_length, &written, NULL);
  FlushFileBuffers(file);
  CloseHandle(file);
}

static DWORD WINAPI patch_thread(LPVOID) {
  LoadDef load_def = (LoadDef)LOAD_DEF_ADDRESS;
  BOOL small_skill_resources_ready;
  BOOL large_skill_resources_ready;
  BOOL swap_skill_resources_ready;
  BOOL skill_resources_ready;
  BOOL vehr_frame_available;
  BOOL hooks_installed;

  append_text("Conflux Spiritism runtime 22 native skill loaders\r\n");
#ifdef CHINESE_HOTA_R10
  append_text(
    "heroes=128-143 creature=118 cloak=114/113/120 "
    "rates=10/20/30 underlying-skill=12\r\n");
#else
  append_text(
    "heroes=128-143 creature=118 nyx=119 cloak=114/113/120 "
    "rates=10/20/30 underlying-skill=12\r\n");
#endif
  write_log();

  hooks_installed = install_hooks();
  append_text("specialty atlas mutation=disabled\r\n");
  vehr_frame_available = verify_specialty_frame(
    load_def,
    "UN44.def",
    HERO_ID_VEHR,
    "extended specialty Vehr frame");

  Sleep(1500);
  small_skill_resources_ready = prepare_skill_resource_pair(
    load_def,
    "secskill.def",
    "SPIRIT.def",
    44,
    44,
    &small_skill_resources,
    "small skill resource pair");
  large_skill_resources_ready = prepare_skill_resource_pair(
    load_def,
    "secsk82.def",
    "SPIR82.def",
    82,
    93,
    &large_skill_resources,
    "large skill resource pair");
  swap_skill_resources_ready = prepare_skill_resource_pair(
    load_def,
    "secsk32.def",
    "SPIR32.def",
    32,
    32,
    &swap_skill_resources,
    "exchange skill resource pair");
  skill_resources_ready =
    small_skill_resources_ready &&
    large_skill_resources_ready &&
    swap_skill_resources_ready;
  append_text("secondary skill frame-table writes=disabled\r\n");
  append_text(
    "hero+level-up scope=native control/popup loaders only\r\n");
  append_text(
    "exchange right-click scope=native popup loader filename only\r\n");
  append_text(
    "Hermit scope=exact Shack success popup native loader only\r\n");

  append_text("final=");
  append_text(
    skill_resources_ready &&
      vehr_frame_available &&
      hooks_installed
      ? "hero+level-up+transfer+Hermit native-loader hooks installed; "
        "native HotA groups unchanged"
      : "one or more runtime operations failed");
  append_text("\r\n");
  write_log();
  return 0;
}

extern "C" BOOL WINAPI DllMain(
  HINSTANCE module,
  DWORD reason,
  LPVOID reserved) {
  (void)reserved;
  if (reason == DLL_PROCESS_ATTACH) {
    HANDLE thread;
    self_module = module;
    DisableThreadLibraryCalls(module);
    thread = CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    if (thread != NULL) {
      CloseHandle(thread);
    }
  }
  return TRUE;
}
