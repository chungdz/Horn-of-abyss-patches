#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define HERO_ID_NYX 140
#define HERO_ID_VEHR 212
#define CONFLUX_HERO_FIRST_ID 128
#define CONFLUX_HERO_LAST_ID 143
#define LEGACY_SPECIALTY_FRAME_COUNT 156
#define CREATURE_ID_WALKING_DEAD 58
#define CREATURE_ID_WIGHT 60
#define CREATURE_ID_LICH 64
#define CREATURE_ID_EARTH_ELEMENTAL 113
#define CREATURE_ID_FIRE_ELEMENTAL 114
#define CREATURE_ID_PIXIE 118
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
#define SWAP_DIALOG_BUILDER_ADDRESS 0x005AAD90
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
typedef void *(__thiscall *SwapDialogBuilder)(
  void *dialog,
  void **heroes);

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
static char saved_secondary_skill_definition[
  SECONDARY_SKILL_DEFINITION_CAPACITY];
static BOOL skill_definition_applied;
static char saved_secondary_skill_large_definition[
  SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY];
static BOOL skill_large_definition_applied;
static char saved_specialty_definition[SPECIALTY_DEFINITION_CAPACITY];
static BOOL specialty_definition_applied;
static uintptr_t hd_specialty_definition_address;
static char saved_hd_specialty_definition[
  HD_HOTA_SPECIALTY_DEFINITION_CAPACITY];
static BOOL hd_specialty_definition_applied;
static uintptr_t hd_secondary_skill_definition_address;
static char saved_hd_secondary_skill_definition[
  HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY];
static BOOL hd_skill_definition_applied;
static uintptr_t hd_swap_specialty_definition_address;
static char saved_hd_swap_specialty_definition[
  HD_HOTA_SWAP_DEFINITION_CAPACITY];
static BOOL hd_swap_specialty_definition_applied;
static uintptr_t hd_swap_skill_definition_address;
static char saved_hd_swap_skill_definition[
  HD_HOTA_SWAP_DEFINITION_CAPACITY];
static BOOL hd_swap_skill_definition_applied;
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
static SwapDialogBuilder chained_swap_dialog_builder;

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
  "as Pixies. With the Cloak of the Undead King, Fire Elementals are "
  "summoned instead. Buildings and artifacts add their normal bonuses.";
static const char advanced_spiritism_description[] =
  "{Advanced Spiritism}\n\n"
  "After combat, 20% of the health of slain living creatures is summoned "
  "as Pixies. With the Cloak of the Undead King, Earth Elementals are "
  "summoned instead. Buildings and artifacts add their normal bonuses.";
static const char expert_spiritism_description[] =
  "{Expert Spiritism}\n\n"
  "After combat, 30% of the health of slain living creatures is summoned "
  "as Pixies. With the Cloak of the Undead King, Psychic Elementals are "
  "summoned instead. Buildings and artifacts add their normal bonuses.";
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

static BOOL apply_spiritism_definition(void) {
  char verified[SECONDARY_SKILL_DEFINITION_CAPACITY];
  if (
    !safe_read(
      SECONDARY_SKILL_DEFINITION_ADDRESS,
      saved_secondary_skill_definition,
      sizeof(saved_secondary_skill_definition)) ||
    !bytes_equal(
      saved_secondary_skill_definition,
      native_definition_name,
      sizeof(saved_secondary_skill_definition))) {
    return FALSE;
  }
  if (!safe_write(
    SECONDARY_SKILL_DEFINITION_ADDRESS,
    spiritism_definition_name,
    sizeof(spiritism_definition_name))) {
    return FALSE;
  }
  skill_definition_applied = TRUE;
  if (
    !safe_read(
      SECONDARY_SKILL_DEFINITION_ADDRESS,
      verified,
      sizeof(verified)) ||
    !bytes_equal(
      verified,
      spiritism_definition_name,
      sizeof(verified))) {
    return FALSE;
  }
  return TRUE;
}

static BOOL apply_spiritism_large_definition(void) {
  char verified[SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY];
  if (
    !safe_read(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      saved_secondary_skill_large_definition,
      sizeof(saved_secondary_skill_large_definition)) ||
    !bytes_equal(
      saved_secondary_skill_large_definition,
      native_large_definition_name,
      sizeof(saved_secondary_skill_large_definition))) {
    return FALSE;
  }
  if (!safe_write(
    SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
    spiritism_large_definition_name,
    sizeof(spiritism_large_definition_name))) {
    return FALSE;
  }
  skill_large_definition_applied = TRUE;
  if (
    !safe_read(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      verified,
      sizeof(verified)) ||
    !bytes_equal(
      verified,
      spiritism_large_definition_name,
      sizeof(verified))) {
    return FALSE;
  }
  return TRUE;
}

static void restore_spiritism_ui(void) {
  uintptr_t text_address = 0;

  if (skill_large_definition_applied) {
    safe_write(
      SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS,
      saved_secondary_skill_large_definition,
      sizeof(saved_secondary_skill_large_definition));
    skill_large_definition_applied = FALSE;
  }
  if (skill_definition_applied) {
    safe_write(
      SECONDARY_SKILL_DEFINITION_ADDRESS,
      saved_secondary_skill_definition,
      sizeof(saved_secondary_skill_definition));
    skill_definition_applied = FALSE;
  }
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
  if (
    !apply_spiritism_text() ||
    !apply_spiritism_definition() ||
    !apply_spiritism_large_definition()) {
    restore_spiritism_ui();
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

static BOOL apply_hd_spiritism_definition(void) {
  char verified[HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY];
  if (
    hd_secondary_skill_definition_address == 0 ||
    !safe_read(
      hd_secondary_skill_definition_address,
      saved_hd_secondary_skill_definition,
      sizeof(saved_hd_secondary_skill_definition)) ||
    !bytes_equal(
      saved_hd_secondary_skill_definition,
      native_hd_definition_name,
      sizeof(saved_hd_secondary_skill_definition)) ||
    !safe_write(
      hd_secondary_skill_definition_address,
      spiritism_hd_definition_name,
      sizeof(spiritism_hd_definition_name))) {
    return FALSE;
  }
  hd_skill_definition_applied = TRUE;
  if (
    !safe_read(
      hd_secondary_skill_definition_address,
      verified,
      sizeof(verified)) ||
    !bytes_equal(
      verified,
      spiritism_hd_definition_name,
      sizeof(verified))) {
    return FALSE;
  }
  return TRUE;
}

static void restore_hd_spiritism_definition(void) {
  if (!hd_skill_definition_applied) {
    return;
  }
  safe_write(
    hd_secondary_skill_definition_address,
    saved_hd_secondary_skill_definition,
    sizeof(saved_hd_secondary_skill_definition));
  hd_skill_definition_applied = FALSE;
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

static BOOL apply_hd_swap_spiritism_definition(void) {
  return apply_definition_alias(
    hd_swap_skill_definition_address,
    native_hd_swap_skill_definition_name,
    spiritism_hd_swap_skill_definition_name,
    saved_hd_swap_skill_definition,
    sizeof(saved_hd_swap_skill_definition),
    &hd_swap_skill_definition_applied);
}

static void restore_hd_swap_spiritism_definition(void) {
  restore_definition_alias(
    hd_swap_skill_definition_address,
    saved_hd_swap_skill_definition,
    sizeof(saved_hd_swap_skill_definition),
    &hd_swap_skill_definition_applied);
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
    set_spiritism_message(TRUE);
    switch (native_creature) {
      case CREATURE_ID_WALKING_DEAD:
        return CREATURE_ID_FIRE_ELEMENTAL;
      case CREATURE_ID_WIGHT:
        return CREATURE_ID_EARTH_ELEMENTAL;
      case CREATURE_ID_LICH:
        return CREATURE_ID_PSYCHIC_ELEMENTAL;
      default:
        return CREATURE_ID_PIXIE;
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

static void __fastcall direct_show_hero_dialog(
  int hero_id,
  int dismissable,
  int not_in_town,
  int right_click) {
  if (is_spiritist_hero_id((DWORD)hero_id)) {
    begin_spiritism_ui();
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
  chained_show_hero_dialog(
    hero_id,
    dismissable,
    not_in_town,
    right_click);
}

static void __thiscall direct_level_up(void *hero) {
  if (is_spiritist_hero(hero)) {
    begin_spiritism_ui();
    chained_level_up(hero);
    end_spiritism_ui();
    return;
  }
  chained_level_up(hero);
}

static void *__stdcall direct_hd_show_hero_dialog(int hero_id) {
  void *dialog;

  if (!is_spiritist_hero_id((DWORD)hero_id)) {
    return chained_hd_show_hero_dialog(hero_id);
  }
  begin_spiritism_ui();
  apply_hd_spiritism_definition();
  if (hero_id == HERO_ID_NYX) {
    apply_hd_nyx_specialty_definition();
  }
  dialog = chained_hd_show_hero_dialog(hero_id);
  restore_hd_nyx_specialty_definition();
  restore_hd_spiritism_definition();
  end_spiritism_ui();
  return dialog;
}

static BOOL get_swap_alias_state(
  void **heroes,
  BOOL *use_spiritism,
  BOOL *use_nyx_specialty) {
  BOOL any_spiritist = FALSE;
  BOOL ordinary_necromancer = FALSE;
  BOOL nyx_present = FALSE;
  BOOL specialty_frames_supported = TRUE;
  DWORD index;

  if (
    heroes == NULL ||
    use_spiritism == NULL ||
    use_nyx_specialty == NULL) {
    return FALSE;
  }
  for (index = 0; index < 2; index++) {
    void *hero = NULL;
    DWORD hero_id = 0;
    BYTE necromancy_level = 0;
    BOOL spiritist;

    if (
      !safe_read(
        (uintptr_t)heroes + index * sizeof(hero),
        &hero,
        sizeof(hero)) ||
      !read_hero_id(hero, &hero_id) ||
      !read_necromancy_level(hero, &necromancy_level)) {
      return FALSE;
    }
    spiritist =
      is_spiritist_hero_id(hero_id) &&
      necromancy_level >= 1 &&
      necromancy_level <= 3;
    if (spiritist) {
      any_spiritist = TRUE;
    } else if (necromancy_level != 0) {
      ordinary_necromancer = TRUE;
    }
    if (hero_id == HERO_ID_NYX) {
      nyx_present = TRUE;
    }
    if (hero_id >= LEGACY_SPECIALTY_FRAME_COUNT) {
      specialty_frames_supported = FALSE;
    }
  }
  *use_spiritism = any_spiritist && !ordinary_necromancer;
  *use_nyx_specialty = nyx_present && specialty_frames_supported;
  return TRUE;
}

static void *__fastcall direct_swap_dialog_builder(
  void *dialog,
  void *,
  void **heroes) {
  BOOL use_spiritism = FALSE;
  BOOL use_nyx_specialty = FALSE;
  void *result;

  if (get_swap_alias_state(
    heroes,
    &use_spiritism,
    &use_nyx_specialty)) {
    if (use_spiritism) {
      apply_hd_swap_spiritism_definition();
    }
    if (use_nyx_specialty) {
      apply_hd_swap_nyx_specialty_definition();
    }
  }
  result = chained_swap_dialog_builder(dialog, heroes);
  restore_hd_swap_nyx_specialty_definition();
  restore_hd_swap_spiritism_definition();
  return result;
}

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
      &target)) {
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
  BYTE previous[36];
  BYTE current[36];
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
      safe_read(SWAP_DIALOG_BUILDER_ADDRESS, current + 31, 5)) {
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
  CodePatch patches[9];
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
  append_code_bytes(
    "live hero inspection dereference 1",
    (uintptr_t)hota + HOTA_INSPECTION_GUARD_1_SITE_RVA);
  append_code_bytes(
    "live hero inspection dereference 2",
    (uintptr_t)hota + HOTA_INSPECTION_GUARD_2_SITE_RVA);
  append_text("scoped skill resource alias=");
  append_text(
    validate_secondary_skill_definition()
      ? "ready\r\n"
      : "unexpected filename literal\r\n");
  append_text("scoped large skill resource alias=");
  append_text(
    validate_secondary_skill_large_definition()
      ? "ready\r\n"
      : "unexpected filename literal\r\n");
  append_text("HD hero selection alias=");
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
  append_text("scoped exchange Spiritism alias=");
  append_text(
    validate_hd_swap_skill_definition()
      ? "ready\r\n"
      : "unavailable or unexpected filename literal\r\n");
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
      &patches[8]) &&
    target_hd_1 == target_hd_2 &&
    validate_secondary_skill_definition() &&
    validate_secondary_skill_large_definition() &&
    validate_hd_secondary_skill_definition() &&
    validate_specialty_definition() &&
    validate_hd_specialty_definition() &&
    validate_hd_swap_specialty_definition() &&
    validate_hd_swap_skill_definition();
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

  prepared = apply_code_patches(patches, 9);
  append_text("necromancy hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("necromancy rate hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("hero dialog hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("level-up hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("HD hero selection hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("HD exchange dialog hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("hero inspection null guards=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("hook backend=direct relative chaining\r\n");
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
  BOOL vehr_frame_available;
  BOOL hooks_installed;

  append_text("Conflux Spiritism runtime 9\r\n");
  append_text(
    "heroes=128-143 creature=118 cloak=114/113/120 "
    "rates=10/20/30 underlying-skill=12\r\n");
  write_log();

  hooks_installed = install_hooks();
  append_text("specialty atlas mutation=disabled\r\n");
  vehr_frame_available = verify_specialty_frame(
    load_def,
    "UN44.def",
    HERO_ID_VEHR,
    "extended specialty Vehr frame");

  append_text("final=");
  append_text(
    vehr_frame_available &&
      hooks_installed
      ? "Spiritism and Nyx UI hooks installed; shared atlas untouched"
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
