#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define HERO_ID_NYX 140
#define CREATURE_ID_PIXIE 118
#define SECONDARY_SKILL_NECROMANCY 12
#define SECONDARY_SKILL_TEXT_ADDRESS 0x0067DCF0
#define SECONDARY_SKILL_DEFINITION_ADDRESS 0x006601D0
#define SECONDARY_SKILL_DEFINITION_CAPACITY 16
#define SECONDARY_SKILL_LARGE_DEFINITION_ADDRESS 0x006600F8
#define SECONDARY_SKILL_LARGE_DEFINITION_CAPACITY 12
#define GET_NECROMANCY_CREATURE_ADDRESS 0x004E3ED0
#define SHOW_HERO_DIALOG_ADDRESS 0x004E1A70
#define LEVEL_UP_ADDRESS 0x004DA990
#define HD_HOTA_HERO_DIALOG_RVA 0x002350E0
#define HD_HOTA_HERO_DIALOG_CALL_1_RVA 0x0023708A
#define HD_HOTA_HERO_DIALOG_CALL_2_RVA 0x00237C9D
#define HD_HOTA_SECONDARY_SKILL_DEFINITION_RVA 0x002A0450
#define HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY 16
#define GENERAL_TEXT_ADDRESS 0x006A5DC4
#define GENERAL_TEXT_TABLE_OFFSET 0x20
#define NECROMANCY_MESSAGE_PLURAL_OFFSET 0x2AC
#define NECROMANCY_MESSAGE_SINGULAR_OFFSET 0x2B0
#define LOAD_DEF_ADDRESS 0x0055C9C0
#define LOG_CAPACITY 32768

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
typedef void (__fastcall *ShowHeroDialog)(
  int hero_id,
  int dismissable,
  int not_in_town,
  int right_click);
typedef void (__thiscall *LevelUp)(void *hero);
typedef void *(__stdcall *HdShowHeroDialog)(int hero_id);

struct RelativePatch {
  uintptr_t address;
  BYTE original[5];
  BYTE replacement[5];
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
static uintptr_t hd_secondary_skill_definition_address;
static char saved_hd_secondary_skill_definition[
  HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY];
static BOOL hd_skill_definition_applied;
static uintptr_t plural_message_entry;
static uintptr_t singular_message_entry;
static const char *saved_necromancy_message_plural;
static const char *saved_necromancy_message_singular;
static GetNecromancyCreature chained_get_necromancy_creature;
static ShowHeroDialog chained_show_hero_dialog;
static LevelUp chained_level_up;
static HdShowHeroDialog chained_hd_show_hero_dialog;

static const char spiritism_name[] = "Spiritism";
static const char basic_spiritism_description[] =
  "{Basic Spiritism}\n\n"
  "After combat, 5% of the health of slain living creatures is summoned "
  "as Pixies. Buildings and artifacts that enhance this power increase "
  "the percentage.";
static const char advanced_spiritism_description[] =
  "{Advanced Spiritism}\n\n"
  "After combat, 10% of the health of slain living creatures is summoned "
  "as Pixies. Buildings and artifacts that enhance this power increase "
  "the percentage.";
static const char expert_spiritism_description[] =
  "{Expert Spiritism}\n\n"
  "After combat, 15% of the health of slain living creatures is summoned "
  "as Pixies. Buildings and artifacts that enhance this power increase "
  "the percentage.";
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
static const char native_hd_definition_name[
  HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY] = "Secskill.def";
static const char spiritism_hd_definition_name[
  HD_HOTA_SECONDARY_SKILL_DEFINITION_CAPACITY] = "SPIRIT.def";
static const char spiritism_message_plural[] =
  "Practicing the art of Spiritism, your hero is able to raise %d of the "
  "enemy's dead to return under their service as %s.";
static const char spiritism_message_singular[] =
  "Practicing the art of Spiritism, your hero is able to raise one of the "
  "enemy's dead to return under their service as a %s.";

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

static BOOL patch_specialty_frame(
  LoadDef load_def,
  const char *target_name,
  const char *source_name) {
  LoadedDef *target = load_def(target_name);
  LoadedDef *source = load_def(source_name);
  void **target_entry = NULL;
  void **source_entry = NULL;
  void *target_frame = NULL;
  void *source_frame = NULL;
  void *verified = NULL;

  append_text("specialty ");
  append_text(target_name);
  append_text(" <- ");
  append_text(source_name);
  append_text("\r\n");
  if (
    target == NULL ||
    source == NULL ||
    !read_frame(target, HERO_ID_NYX, &target_entry, &target_frame) ||
    !read_frame(source, HERO_ID_NYX, &source_entry, &source_frame)) {
    append_text("result=invalid definition or frame table\r\n");
    return FALSE;
  }
  append_frame("target frame", target_frame);
  append_frame("source frame", source_frame);
  if (target_frame == source_frame) {
    append_text("result=already shared\r\n");
    return TRUE;
  }
  if (!safe_write((uintptr_t)target_entry, &source_frame, sizeof(source_frame))) {
    append_text("result=write failed\r\n");
    return FALSE;
  }
  if (
    !safe_read((uintptr_t)target_entry, &verified, sizeof(verified)) ||
    verified != source_frame) {
    append_text("result=verification failed\r\n");
    return FALSE;
  }
  append_text("result=patched\r\n");
  return TRUE;
}

static BOOL is_spiritist_hero(const void *hero) {
  DWORD hero_id = 0;
  BYTE necromancy_level = 0;
  return hero != NULL &&
    safe_read((uintptr_t)hero + 0x1A, &hero_id, sizeof(hero_id)) &&
    hero_id == HERO_ID_NYX &&
    safe_read(
      (uintptr_t)hero + 0xC9 + SECONDARY_SKILL_NECROMANCY,
      &necromancy_level,
      sizeof(necromancy_level)) &&
    necromancy_level != 0;
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

static BOOL get_general_text_entries(
  uintptr_t *plural_entry,
  uintptr_t *singular_entry) {
  DWORD general_text = 0;
  DWORD table = 0;
  if (
    plural_entry == NULL ||
    singular_entry == NULL ||
    !safe_read(GENERAL_TEXT_ADDRESS, &general_text, sizeof(general_text)) ||
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

static void restore_spiritism_message(void) {
  if (!spiritism_message_applied) {
    return;
  }
  safe_write(
    plural_message_entry,
    &saved_necromancy_message_plural,
    sizeof(saved_necromancy_message_plural));
  safe_write(
    singular_message_entry,
    &saved_necromancy_message_singular,
    sizeof(saved_necromancy_message_singular));
  spiritism_message_applied = FALSE;
}

static BOOL set_spiritism_message(BOOL active) {
  const char *plural = spiritism_message_plural;
  const char *singular = spiritism_message_singular;

  if (!active) {
    restore_spiritism_message();
    return TRUE;
  }
  if (spiritism_message_applied) {
    return TRUE;
  }
  if (
    !get_general_text_entries(
      &plural_message_entry,
      &singular_message_entry) ||
    !safe_read(
      plural_message_entry,
      &saved_necromancy_message_plural,
      sizeof(saved_necromancy_message_plural)) ||
    !safe_read(
      singular_message_entry,
      &saved_necromancy_message_singular,
      sizeof(saved_necromancy_message_singular)) ||
    !safe_write(plural_message_entry, &plural, sizeof(plural)) ||
    !safe_write(singular_message_entry, &singular, sizeof(singular))) {
    if (plural_message_entry != 0 && saved_necromancy_message_plural != NULL) {
      safe_write(
        plural_message_entry,
        &saved_necromancy_message_plural,
        sizeof(saved_necromancy_message_plural));
    }
    return FALSE;
  }
  spiritism_message_applied = TRUE;
  return TRUE;
}

static int __thiscall direct_get_necromancy_creature(void *hero) {
  if (is_spiritist_hero(hero)) {
    set_spiritism_message(TRUE);
    return CREATURE_ID_PIXIE;
  }
  set_spiritism_message(FALSE);
  return chained_get_necromancy_creature(hero);
}

static void __fastcall direct_show_hero_dialog(
  int hero_id,
  int dismissable,
  int not_in_town,
  int right_click) {
  if (hero_id == HERO_ID_NYX) {
    begin_spiritism_ui();
    chained_show_hero_dialog(
      hero_id,
      dismissable,
      not_in_town,
      right_click);
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

  if (hero_id != HERO_ID_NYX) {
    return chained_hd_show_hero_dialog(hero_id);
  }
  begin_spiritism_ui();
  apply_hd_spiritism_definition();
  dialog = chained_hd_show_hero_dialog(hero_id);
  restore_hd_spiritism_definition();
  end_spiritism_ui();
  return dialog;
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

static BOOL prepare_relative_patch(
  uintptr_t address,
  BYTE expected_opcode,
  void *replacement,
  uintptr_t expected_target,
  uintptr_t *original_target,
  RelativePatch *patch) {
  intptr_t displacement;

  if (
    patch == NULL ||
    original_target == NULL ||
    !safe_read(address, patch->original, sizeof(patch->original)) ||
    !get_relative_target(address, expected_opcode, original_target) ||
    (expected_target != 0 && *original_target != expected_target)) {
    return FALSE;
  }
  displacement =
    (intptr_t)(uintptr_t)replacement -
    (intptr_t)(address + sizeof(patch->replacement));
  if (displacement < INT32_MIN || displacement > INT32_MAX) {
    return FALSE;
  }
  patch->address = address;
  patch->replacement[0] = expected_opcode;
  patch->replacement[1] = (BYTE)(displacement & 0xFF);
  patch->replacement[2] = (BYTE)((displacement >> 8) & 0xFF);
  patch->replacement[3] = (BYTE)((displacement >> 16) & 0xFF);
  patch->replacement[4] = (BYTE)((displacement >> 24) & 0xFF);
  return TRUE;
}

static BOOL apply_relative_patches(
  RelativePatch *patches,
  DWORD count) {
  DWORD applied = 0;
  BYTE verified[5];

  while (applied < count) {
    if (
      !safe_write(
        patches[applied].address,
        patches[applied].replacement,
        sizeof(patches[applied].replacement)) ||
      !safe_read(
        patches[applied].address,
        verified,
        sizeof(verified)) ||
      !bytes_equal(
        (const char *)verified,
        (const char *)patches[applied].replacement,
        sizeof(verified))) {
      safe_write(
        patches[applied].address,
        patches[applied].original,
        sizeof(patches[applied].original));
      while (applied != 0) {
        applied--;
        safe_write(
          patches[applied].address,
          patches[applied].original,
          sizeof(patches[applied].original));
      }
      return FALSE;
    }
    applied++;
  }
  return TRUE;
}

static BOOL runtime_targets_ready(
  HMODULE *hd_hota,
  uintptr_t *hd_dialog,
  uintptr_t *hd_call_1,
  uintptr_t *hd_call_2) {
  uintptr_t target = 0;
  HMODULE module = GetModuleHandleA("HD_HOTA.dll");

  if (
    module == NULL ||
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
      &target)) {
    return FALSE;
  }
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
  HMODULE *hd_hota,
  uintptr_t *hd_dialog,
  uintptr_t *hd_call_1,
  uintptr_t *hd_call_2) {
  BYTE previous[25];
  BYTE current[25];
  BOOL have_previous = FALSE;
  DWORD stable_checks = 0;
  DWORD attempt;

  Sleep(2000);
  for (attempt = 0; attempt < 40; attempt++) {
    if (
      runtime_targets_ready(
        hd_hota,
        hd_dialog,
        hd_call_1,
        hd_call_2) &&
      safe_read(GET_NECROMANCY_CREATURE_ADDRESS, current, 5) &&
      safe_read(SHOW_HERO_DIALOG_ADDRESS, current + 5, 5) &&
      safe_read(LEVEL_UP_ADDRESS, current + 10, 5) &&
      safe_read(*hd_call_1, current + 15, 5) &&
      safe_read(*hd_call_2, current + 20, 5)) {
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
  RelativePatch patches[5];
  HMODULE hd_hota = NULL;
  uintptr_t hd_dialog_address = 0;
  uintptr_t hd_call_1 = 0;
  uintptr_t hd_call_2 = 0;
  uintptr_t target_necromancy = 0;
  uintptr_t target_dialog = 0;
  uintptr_t target_level_up = 0;
  uintptr_t target_hd_1 = 0;
  uintptr_t target_hd_2 = 0;
  BOOL prepared;

  if (!wait_for_runtime_targets(
    &hd_hota,
    &hd_dialog_address,
    &hd_call_1,
    &hd_call_2)) {
    append_text("direct hooks=runtime targets did not stabilize\r\n");
    return FALSE;
  }
  hd_secondary_skill_definition_address =
    (uintptr_t)hd_hota + HD_HOTA_SECONDARY_SKILL_DEFINITION_RVA;

  append_code_bytes(
    "live GetNecromancyCreatureId",
    GET_NECROMANCY_CREATURE_ADDRESS);
  append_code_bytes("live ShowHeroDialog", SHOW_HERO_DIALOG_ADDRESS);
  append_code_bytes("live LevelUp", LEVEL_UP_ADDRESS);
  append_code_bytes("live HD hero selection call 1", hd_call_1);
  append_code_bytes("live HD hero selection call 2", hd_call_2);
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

  prepared =
    prepare_relative_patch(
      GET_NECROMANCY_CREATURE_ADDRESS,
      0xE9,
      (void *)direct_get_necromancy_creature,
      0,
      &target_necromancy,
      &patches[0]) &&
    prepare_relative_patch(
      SHOW_HERO_DIALOG_ADDRESS,
      0xE9,
      (void *)direct_show_hero_dialog,
      0,
      &target_dialog,
      &patches[1]) &&
    prepare_relative_patch(
      LEVEL_UP_ADDRESS,
      0xE9,
      (void *)direct_level_up,
      0,
      &target_level_up,
      &patches[2]) &&
    prepare_relative_patch(
      hd_call_1,
      0xE8,
      (void *)direct_hd_show_hero_dialog,
      hd_dialog_address,
      &target_hd_1,
      &patches[3]) &&
    prepare_relative_patch(
      hd_call_2,
      0xE8,
      (void *)direct_hd_show_hero_dialog,
      hd_dialog_address,
      &target_hd_2,
      &patches[4]) &&
    target_hd_1 == target_hd_2 &&
    validate_secondary_skill_definition() &&
    validate_secondary_skill_large_definition() &&
    validate_hd_secondary_skill_definition();
  if (!prepared) {
    append_text("direct hooks=target validation failed\r\n");
    return FALSE;
  }

  chained_get_necromancy_creature =
    (GetNecromancyCreature)target_necromancy;
  chained_show_hero_dialog = (ShowHeroDialog)target_dialog;
  chained_level_up = (LevelUp)target_level_up;
  chained_hd_show_hero_dialog = (HdShowHeroDialog)target_hd_1;

  prepared = apply_relative_patches(patches, 5);
  append_text("necromancy hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("hero dialog hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("level-up hook=");
  append_text(prepared ? "installed\r\n" : "failed\r\n");
  append_text("HD hero selection hook=");
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
    const char *name = "NyxSpiritism.log";
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
  BOOL small_patched;
  BOOL large_patched;
  BOOL hooks_installed;

  append_text("Nyx Spiritism runtime 8\r\n");
  append_text("hero=140 creature=118 underlying-skill=12\r\n");
  write_log();

  hooks_installed = install_hooks();
  small_patched = patch_specialty_frame(load_def, "UN32.def", "IX32.def");
  large_patched = patch_specialty_frame(load_def, "UN44.def", "IX44.def");

  append_text("final=");
  append_text(
    small_patched && large_patched && hooks_installed
      ? "specialty fix and Spiritism hooks installed"
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
