#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#include "patcher_x86.hpp"

#define HERO_ID_NYX 140
#define CREATURE_ID_PIXIE 118
#define SECONDARY_SKILL_NECROMANCY 12
#define SECONDARY_SKILL_TEXT_ADDRESS 0x0067DCF0
#define GET_NECROMANCY_CREATURE_ADDRESS 0x004E3ED0
#define SHOW_HERO_DIALOG_ADDRESS 0x004E1A70
#define LEVEL_UP_ADDRESS 0x004DA990
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

struct FrameSwap {
  void **entry;
  void *original;
  BOOL applied;
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

static HMODULE self_module;
static char log_buffer[LOG_CAPACITY];
static DWORD log_length;
static volatile LONG ui_alias_depth;
static SecondarySkillText saved_necromancy_text;
static BOOL skill_text_applied;
static FrameSwap skill_frame_swaps[3];
static LoadedDef *secondary_skill_definition;
static LoadedDef *spiritism_definition;

static const char spiritism_name[] = "Spiritism";
static const char basic_spiritism_description[] =
  "{Basic Spiritism}\n\n"
  "After combat, 5% of the health of slain living creatures is summoned "
  "as Pixies. Necromancy Amplifiers and Necromancy artifacts increase "
  "this percentage.";
static const char advanced_spiritism_description[] =
  "{Advanced Spiritism}\n\n"
  "After combat, 10% of the health of slain living creatures is summoned "
  "as Pixies. Necromancy Amplifiers and Necromancy artifacts increase "
  "this percentage.";
static const char expert_spiritism_description[] =
  "{Expert Spiritism}\n\n"
  "After combat, 15% of the health of slain living creatures is summoned "
  "as Pixies. Necromancy Amplifiers and Necromancy artifacts increase "
  "this percentage.";
static const SecondarySkillText spiritism_text = {
  spiritism_name,
  {
    basic_spiritism_description,
    advanced_spiritism_description,
    expert_spiritism_description,
  },
};

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
  return address != 0 &&
    WriteProcessMemory(
      GetCurrentProcess(),
      (LPVOID)address,
      source,
      size,
      &bytes_written) &&
    bytes_written == size;
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

static BOOL apply_spiritism_frames(void) {
  LoadDef load_def = (LoadDef)LOAD_DEF_ADDRESS;
  DWORD index;

  if (secondary_skill_definition == NULL) {
    secondary_skill_definition = load_def("Secskill.def");
  }
  if (spiritism_definition == NULL) {
    spiritism_definition = load_def("SPIRIT.def");
  }
  if (
    secondary_skill_definition == NULL ||
    spiritism_definition == NULL) {
    return FALSE;
  }
  for (index = 0; index < 3; index++) {
    void **target_entry = NULL;
    void **source_entry = NULL;
    void *target_frame = NULL;
    void *source_frame = NULL;
    DWORD frame_index = 39 + index;
    if (
      !read_frame(
        secondary_skill_definition,
        frame_index,
        &target_entry,
        &target_frame) ||
      !read_frame(
        spiritism_definition,
        frame_index,
        &source_entry,
        &source_frame) ||
      !safe_write(
        (uintptr_t)target_entry,
        &source_frame,
        sizeof(source_frame))) {
      return FALSE;
    }
    skill_frame_swaps[index].entry = target_entry;
    skill_frame_swaps[index].original = target_frame;
    skill_frame_swaps[index].applied = TRUE;
  }
  return TRUE;
}

static void restore_spiritism_ui(void) {
  int index;
  uintptr_t text_address = 0;

  for (index = 2; index >= 0; index--) {
    if (skill_frame_swaps[index].applied) {
      safe_write(
        (uintptr_t)skill_frame_swaps[index].entry,
        &skill_frame_swaps[index].original,
        sizeof(skill_frame_swaps[index].original));
      skill_frame_swaps[index].applied = FALSE;
    }
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

static void begin_spiritism_ui(void) {
  LONG depth = InterlockedIncrement(&ui_alias_depth);
  if (depth != 1) {
    return;
  }
  apply_spiritism_text();
  if (!apply_spiritism_frames()) {
    restore_spiritism_ui();
  }
}

static void end_spiritism_ui(void) {
  LONG depth = InterlockedDecrement(&ui_alias_depth);
  if (depth == 0) {
    restore_spiritism_ui();
  } else if (depth < 0) {
    InterlockedExchange(&ui_alias_depth, 0);
  }
}

static int __stdcall hook_get_necromancy_creature(
  HiHook *hook,
  void *hero) {
  if (is_spiritist_hero(hero)) {
    return CREATURE_ID_PIXIE;
  }
  return ((GetNecromancyCreature)hook->GetDefaultFunc())(hero);
}

static void __stdcall hook_show_hero_dialog(
  HiHook *hook,
  int hero_id,
  int dismissable,
  int not_in_town,
  int right_click) {
  if (hero_id == HERO_ID_NYX) {
    begin_spiritism_ui();
    ((ShowHeroDialog)hook->GetDefaultFunc())(
      hero_id,
      dismissable,
      not_in_town,
      right_click);
    end_spiritism_ui();
    return;
  }
  ((ShowHeroDialog)hook->GetDefaultFunc())(
    hero_id,
    dismissable,
    not_in_town,
    right_click);
}

static void __stdcall hook_level_up(HiHook *hook, void *hero) {
  if (is_spiritist_hero(hero)) {
    begin_spiritism_ui();
    ((LevelUp)hook->GetDefaultFunc())(hero);
    end_spiritism_ui();
    return;
  }
  ((LevelUp)hook->GetDefaultFunc())(hero);
}

static BOOL expected_code(uintptr_t address, const BYTE *bytes, SIZE_T size) {
  BYTE actual[8];
  return size <= sizeof(actual) &&
    safe_read(address, actual, size) &&
    __builtin_memcmp(actual, bytes, size) == 0;
}

static BOOL install_hooks(void) {
  static const BYTE necromancy_prologue[] = {0x56, 0x8B, 0xF1, 0x33, 0xC0};
  static const BYTE dialog_prologue[] = {0x55, 0x8B, 0xEC, 0x6A, 0xFF};
  static const BYTE level_up_prologue[] = {0x55, 0x8B, 0xEC, 0x6A, 0xFF};
  Patcher *patcher;
  PatcherInstance *instance;
  HiHook *necromancy_hook;
  HiHook *dialog_hook;
  HiHook *level_up_hook;

  if (
    !expected_code(
      GET_NECROMANCY_CREATURE_ADDRESS,
      necromancy_prologue,
      sizeof(necromancy_prologue)) ||
    !expected_code(
      SHOW_HERO_DIALOG_ADDRESS,
      dialog_prologue,
      sizeof(dialog_prologue)) ||
    !expected_code(
      LEVEL_UP_ADDRESS,
      level_up_prologue,
      sizeof(level_up_prologue))) {
    append_text("hooks=unsupported executable bytes\r\n");
    return FALSE;
  }

  patcher = GetPatcher();
  if (patcher == NULL) {
    append_text("hooks=patcher_x86 unavailable\r\n");
    return FALSE;
  }
  instance = patcher->CreateInstance((char *)"NyxSpiritism");
  if (instance == NULL) {
    append_text("hooks=patcher instance unavailable\r\n");
    return FALSE;
  }

  necromancy_hook = instance->WriteHiHook(
    GET_NECROMANCY_CREATURE_ADDRESS,
    SPLICE_,
    EXTENDED_,
    THISCALL_,
    (void *)hook_get_necromancy_creature);
  dialog_hook = instance->WriteHiHook(
    SHOW_HERO_DIALOG_ADDRESS,
    SPLICE_,
    EXTENDED_,
    FASTCALL_,
    (void *)hook_show_hero_dialog);
  level_up_hook = instance->WriteHiHook(
    LEVEL_UP_ADDRESS,
    SPLICE_,
    EXTENDED_,
    THISCALL_,
    (void *)hook_level_up);

  append_text("necromancy hook=");
  append_text(necromancy_hook != NULL ? "installed\r\n" : "failed\r\n");
  append_text("hero dialog hook=");
  append_text(dialog_hook != NULL ? "installed\r\n" : "failed\r\n");
  append_text("level-up hook=");
  append_text(level_up_hook != NULL ? "installed\r\n" : "failed\r\n");
  return necromancy_hook != NULL &&
    dialog_hook != NULL &&
    level_up_hook != NULL;
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

  Sleep(1500);
  append_text("Nyx Spiritism runtime 2\r\n");
  append_text("hero=140 creature=118 underlying-skill=12\r\n");

  small_patched = patch_specialty_frame(load_def, "UN32.def", "IX32.def");
  large_patched = patch_specialty_frame(load_def, "UN44.def", "IX44.def");
  hooks_installed = install_hooks();

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
