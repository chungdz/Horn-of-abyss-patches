#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define RUNTIME_VERSION 7
#define CREATURE_ID_SKELETON 56
#define CREATURE_ID_BONE_DRAGON 68
#define CREATURE_ID_PIXIE 118
#define CREATURE_ID_SPRITE 119
#define CREATURE_ID_FIREBIRD 130
#define TOWN_TYPE_CONFLUX 8
#define BUILDING_HORDE1 18
#define BUILDING_HORDE1_UPGRADED 19
#define CREATURE_INFORMATION_POINTER_ADDRESS 0x006747B0
#define CREATURE_INFORMATION_STRIDE 0x74
#define CREATURE_TOWN_OFFSET 0x00
#define CREATURE_LEVEL_OFFSET 0x04
#define CREATURE_HORDE_GROWTH_OFFSET 0x48
#define GENERAL_TEXT_ADDRESS 0x006A5DC4
#define GENERAL_TEXT_TABLE_OFFSET 0x20
#define TRANSFORMER_TEXT_OFFSET 0x798
#define TRANSFORMER_TEXT_COUNT 4
#define HOTA_TRANSFORMER_TARGET_COUNT_RVA 0x006354BC
#define HOTA_TRANSFORMER_TARGET_POINTER_RVA 0x006354E4
#define TRANSFORMER_TARGET_CAPACITY 1024
#define OPEN_SKELETON_TRANSFORMER_ADDRESS 0x005D3390
#define GARDEN_HANDLER_ADDRESS 0x005D38E1
#define GARDEN_HANDLER_CONTINUE_ADDRESS 0x005D38E6
#define TOWN_CLICK_HANDLER_CLEANUP_ADDRESS 0x005D4617
#define TOWN_MANAGER_TOWN_OFFSET 0x38
#define TOWN_TYPE_OFFSET 0x04
#define TOWN_BUILT_BUILDINGS_OFFSET 0x150
#define GARDEN_BUILT_MASK 0x000C0000
#define LOG_CAPACITY 8192

typedef void (__thiscall *OpenTransformer)(void *town_manager);

static HMODULE self_module;
static HMODULE spiritism_module;
static char log_buffer[LOG_CAPACITY];
static DWORD log_length;
static volatile LONG transformer_open;
static volatile LONG last_garden_click_consumed;
static uintptr_t transformer_target_table;
static DWORD transformer_target_count;
static DWORD saved_targets[TRANSFORMER_TARGET_CAPACITY];
static DWORD garden_targets[TRANSFORMER_TARGET_CAPACITY];

static const char pixie_transformer_holding[] = "Holding Area";
static const char pixie_transformer_title[] = "Pixie Transformer";
static const char pixie_transformer_move[] =
  "Move creatures to the Transformer to create Pixies or Firebirds";
static const char pixie_transformer_result[] =
  "Creatures in the Transformer will become Pixies or Firebirds";

static BOOL locate_hota_transformer_table(void);

static void append_char(char value) {
  if (log_length + 1 < LOG_CAPACITY) {
    log_buffer[log_length++] = value;
    log_buffer[log_length] = '\0';
  }
}

static void append_text(const char *text) {
  if (text == NULL) {
    return;
  }
  while (*text != '\0' && log_length + 1 < LOG_CAPACITY) {
    log_buffer[log_length++] = *text++;
  }
  log_buffer[log_length] = '\0';
}

static void append_unsigned(DWORD value) {
  char digits[16];
  DWORD count = 0;
  if (value == 0) {
    append_char('0');
    return;
  }
  while (value != 0 && count < sizeof(digits)) {
    digits[count++] = (char)('0' + value % 10);
    value /= 10;
  }
  while (count != 0) {
    append_char(digits[--count]);
  }
}

static BOOL bytes_equal(
  const void *left,
  const void *right,
  SIZE_T size) {
  const BYTE *a = (const BYTE *)left;
  const BYTE *b = (const BYTE *)right;
  SIZE_T index;
  for (index = 0; index < size; index++) {
    if (a[index] != b[index]) {
      return FALSE;
    }
  }
  return TRUE;
}

static BOOL safe_read(
  uintptr_t address,
  void *destination,
  SIZE_T size) {
  SIZE_T copied = 0;
  if (
    address == 0 ||
    destination == NULL ||
    size == 0 ||
    !ReadProcessMemory(
      GetCurrentProcess(),
      (const void *)address,
      destination,
      size,
      &copied)) {
    return FALSE;
  }
  return copied == size;
}

static BOOL safe_write(
  uintptr_t address,
  const void *source,
  SIZE_T size) {
  DWORD previous_protection;
  DWORD ignored;
  SIZE_T written = 0;
  if (
    address == 0 ||
    source == NULL ||
    size == 0 ||
    !VirtualProtect(
      (void *)address,
      size,
      PAGE_EXECUTE_READWRITE,
      &previous_protection)) {
    return FALSE;
  }
  if (!WriteProcessMemory(
    GetCurrentProcess(),
    (void *)address,
    source,
    size,
    &written)) {
    VirtualProtect((void *)address, size, previous_protection, &ignored);
    return FALSE;
  }
  FlushInstructionCache(GetCurrentProcess(), (void *)address, size);
  VirtualProtect((void *)address, size, previous_protection, &ignored);
  return written == size;
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
  displacement = (intptr_t)target - (intptr_t)(address + 5);
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

static BOOL write_relative_displacement(
  BYTE *destination,
  uintptr_t instruction_end,
  uintptr_t target) {
  intptr_t displacement =
    (intptr_t)target - (intptr_t)instruction_end;
  if (
    destination == NULL ||
    displacement < INT32_MIN ||
    displacement > INT32_MAX) {
    return FALSE;
  }
  destination[0] = (BYTE)(displacement & 0xFF);
  destination[1] = (BYTE)((displacement >> 8) & 0xFF);
  destination[2] = (BYTE)((displacement >> 16) & 0xFF);
  destination[3] = (BYTE)((displacement >> 24) & 0xFF);
  return TRUE;
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
    const char *name = "PixieTransformer.log";
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

static HMODULE load_spiritism_runtime(void) {
  char path[MAX_PATH];
  DWORD length = GetModuleFileNameA(self_module, path, MAX_PATH);
  const char *name = "ConfluxSpiritismRuntime.dll";

  if (length == 0 || length >= MAX_PATH) {
    return NULL;
  }
  while (length != 0 && path[length - 1] != '\\' && path[length - 1] != '/') {
    length--;
  }
  while (*name != '\0' && length + 1 < MAX_PATH) {
    path[length++] = *name++;
  }
  path[length] = '\0';
  return LoadLibraryA(path);
}

static BOOL __stdcall handle_pixie_transformer_click(void *town_manager) {
  const char *saved_text[TRANSFORMER_TEXT_COUNT];
  const char *pixie_text[TRANSFORMER_TEXT_COUNT] = {
    pixie_transformer_holding,
    pixie_transformer_title,
    pixie_transformer_move,
    pixie_transformer_result,
  };
  uintptr_t general_text = 0;
  uintptr_t text_table = 0;
  uintptr_t town = 0;
  BYTE town_type = 0xFF;
  DWORD built_buildings = 0;
  DWORD index;
  DWORD firebird_classes = 0;
  BOOL targets_applied = FALSE;
  BOOL text_applied = FALSE;
  BOOL opened = FALSE;

  if (town_manager == NULL) {
    return FALSE;
  }
  if (
    !safe_read(
      (uintptr_t)town_manager + TOWN_MANAGER_TOWN_OFFSET,
      &town,
      sizeof(town)) ||
    !safe_read(
      town + TOWN_TYPE_OFFSET,
      &town_type,
      sizeof(town_type)) ||
    town_type != TOWN_TYPE_CONFLUX ||
    !safe_read(
      town + TOWN_BUILT_BUILDINGS_OFFSET,
      &built_buildings,
      sizeof(built_buildings)) ||
    (built_buildings & GARDEN_BUILT_MASK) == 0) {
    return FALSE;
  }
  if (InterlockedCompareExchange(&transformer_open, 1, 0) != 0) {
    append_text("Garden click=consumed; transformer already open\r\n");
    write_log();
    return TRUE;
  }
  if (
    (
      transformer_target_table == 0 ||
      transformer_target_count == 0 ||
      transformer_target_count > TRANSFORMER_TARGET_CAPACITY
    ) &&
    !locate_hota_transformer_table()) {
    append_text(
      "Garden click=consumed; transformer target table unavailable\r\n");
    write_log();
    InterlockedExchange(&transformer_open, 0);
    return TRUE;
  }
  if (
    !safe_read(
      transformer_target_table,
      saved_targets,
      transformer_target_count * sizeof(DWORD)) ||
    !safe_read(
      GENERAL_TEXT_ADDRESS,
      &general_text,
      sizeof(general_text)) ||
    !safe_read(
      general_text + GENERAL_TEXT_TABLE_OFFSET,
      &text_table,
      sizeof(text_table)) ||
    !safe_read(
      text_table + TRANSFORMER_TEXT_OFFSET,
      saved_text,
      sizeof(saved_text))) {
    append_text(
      "Garden click=consumed; transformer resources unavailable\r\n");
    write_log();
    InterlockedExchange(&transformer_open, 0);
    return TRUE;
  }

  for (index = 0; index < transformer_target_count; index++) {
    if (saved_targets[index] == CREATURE_ID_BONE_DRAGON) {
      garden_targets[index] = CREATURE_ID_FIREBIRD;
      firebird_classes++;
    } else {
      garden_targets[index] = CREATURE_ID_PIXIE;
    }
  }
  targets_applied = safe_write(
    transformer_target_table,
    garden_targets,
    transformer_target_count * sizeof(DWORD));
  if (targets_applied) {
    text_applied = safe_write(
      text_table + TRANSFORMER_TEXT_OFFSET,
      pixie_text,
      sizeof(pixie_text));
  }
  if (targets_applied && text_applied) {
    ((OpenTransformer)OPEN_SKELETON_TRANSFORMER_ADDRESS)(town_manager);
    opened = TRUE;
  }
  if (text_applied) {
    safe_write(
      text_table + TRANSFORMER_TEXT_OFFSET,
      saved_text,
      sizeof(saved_text));
  }
  if (targets_applied) {
    safe_write(
      transformer_target_table,
      saved_targets,
      transformer_target_count * sizeof(DWORD));
  }
  InterlockedExchange(&transformer_open, 0);
  if (opened) {
    append_text("Garden dialog=opened target count=");
    append_unsigned(transformer_target_count);
    append_text(" firebird classes=");
    append_unsigned(firebird_classes);
    append_text("\r\n");
    write_log();
  } else {
    append_text(
      "Garden click=consumed; transformer memory update failed\r\n");
    write_log();
  }
  return TRUE;
}

static BOOL locate_hota_transformer_table(void) {
  HMODULE hota;
  uintptr_t module;
  uintptr_t table = 0;
  DWORD count = 0;
  DWORD first_target = 0;
  DWORD attempt;

  transformer_target_table = 0;
  transformer_target_count = 0;
  for (attempt = 0; attempt < 20; attempt++) {
    hota = GetModuleHandleA("HotA.dll");
    if (hota != NULL) {
      module = (uintptr_t)hota;
      if (
        safe_read(
          module + HOTA_TRANSFORMER_TARGET_COUNT_RVA,
          &count,
          sizeof(count)) &&
        safe_read(
          module + HOTA_TRANSFORMER_TARGET_POINTER_RVA,
          &table,
          sizeof(table)) &&
        count > CREATURE_ID_SPRITE &&
        count <= TRANSFORMER_TARGET_CAPACITY &&
        table != 0 &&
        safe_read(table, &first_target, sizeof(first_target)) &&
        first_target == CREATURE_ID_SKELETON) {
        transformer_target_count = count;
        transformer_target_table = table;
        return TRUE;
      }
    }
    Sleep(50);
  }
  return FALSE;
}

static BOOL suppress_garden_growth(
  uintptr_t *pixie_horde_growth,
  uintptr_t *sprite_horde_growth) {
  uintptr_t creature_information = 0;
  uintptr_t pixie_record;
  uintptr_t sprite_record;
  int pixie_town = -1;
  int sprite_town = -1;
  int pixie_level = -1;
  int sprite_level = -1;
  int pixie_growth = -1;
  int sprite_growth = -1;
  int zero = 0;
  DWORD attempt;

  if (pixie_horde_growth == NULL || sprite_horde_growth == NULL) {
    return FALSE;
  }
  for (attempt = 0; attempt < 200; attempt++) {
    if (
      safe_read(
        CREATURE_INFORMATION_POINTER_ADDRESS,
        &creature_information,
        sizeof(creature_information)) &&
      creature_information != 0) {
      pixie_record =
        creature_information +
        CREATURE_ID_PIXIE * CREATURE_INFORMATION_STRIDE;
      sprite_record =
        creature_information +
        CREATURE_ID_SPRITE * CREATURE_INFORMATION_STRIDE;
      if (
        safe_read(
          pixie_record + CREATURE_TOWN_OFFSET,
          &pixie_town,
          sizeof(pixie_town)) &&
        safe_read(
          sprite_record + CREATURE_TOWN_OFFSET,
          &sprite_town,
          sizeof(sprite_town)) &&
        safe_read(
          pixie_record + CREATURE_LEVEL_OFFSET,
          &pixie_level,
          sizeof(pixie_level)) &&
        safe_read(
          sprite_record + CREATURE_LEVEL_OFFSET,
          &sprite_level,
          sizeof(sprite_level)) &&
        safe_read(
          pixie_record + CREATURE_HORDE_GROWTH_OFFSET,
          &pixie_growth,
          sizeof(pixie_growth)) &&
        safe_read(
          sprite_record + CREATURE_HORDE_GROWTH_OFFSET,
          &sprite_growth,
          sizeof(sprite_growth)) &&
        pixie_town == TOWN_TYPE_CONFLUX &&
        sprite_town == TOWN_TYPE_CONFLUX &&
        pixie_level == 0 &&
        sprite_level == 0 &&
        pixie_growth == 10 &&
        sprite_growth == 10) {
        *pixie_horde_growth =
          pixie_record + CREATURE_HORDE_GROWTH_OFFSET;
        *sprite_horde_growth =
          sprite_record + CREATURE_HORDE_GROWTH_OFFSET;
        if (!safe_write(*pixie_horde_growth, &zero, sizeof(zero))) {
          return FALSE;
        }
        if (!safe_write(*sprite_horde_growth, &zero, sizeof(zero))) {
          int ten = 10;
          safe_write(*pixie_horde_growth, &ten, sizeof(ten));
          return FALSE;
        }
        return TRUE;
      }
    }
    Sleep(50);
  }

  append_text("growth validation=failed pixie=");
  append_unsigned((DWORD)pixie_growth);
  append_text(" sprite=");
  append_unsigned((DWORD)sprite_growth);
  append_text("\r\n");
  return FALSE;
}

static void restore_garden_growth(
  uintptr_t pixie_horde_growth,
  uintptr_t sprite_horde_growth) {
  int ten = 10;
  if (pixie_horde_growth != 0) {
    safe_write(pixie_horde_growth, &ten, sizeof(ten));
  }
  if (sprite_horde_growth != 0) {
    safe_write(sprite_horde_growth, &ten, sizeof(ten));
  }
}

static BOOL install_garden_click_hook(void) {
  static const BYTE expected[5] = {
    0x8B, 0x45, 0x08, 0x85, 0xC0,
  };
  BYTE live[sizeof(expected)];
  BYTE replacement[sizeof(expected)];
  BYTE *stub;
  uintptr_t fallback;
  uintptr_t transform;

  if (
    !safe_read(GARDEN_HANDLER_ADDRESS, live, sizeof(live)) ||
    !bytes_equal(live, expected, sizeof(expected))) {
    return FALSE;
  }
  stub = (BYTE *)VirtualAlloc(
    NULL,
    128,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE);
  if (stub == NULL) {
    return FALSE;
  }

  /* Garden IDs 18/19 normally reuse a creature-recruitment handler. */
  stub[0] = 0x83;
  stub[1] = 0x7D;
  stub[2] = 0x08;
  stub[3] = 0x00;
  stub[4] = 0x0F;
  stub[5] = 0x85;
  stub[10] = 0x83;
  stub[11] = 0xFF;
  stub[12] = BUILDING_HORDE1;
  stub[13] = 0x0F;
  stub[14] = 0x84;
  stub[19] = 0x83;
  stub[20] = 0xFF;
  stub[21] = BUILDING_HORDE1_UPGRADED;
  stub[22] = 0x0F;
  stub[23] = 0x85;

  transform = (uintptr_t)stub + 28;
  fallback = (uintptr_t)stub + 61;
  if (
    !write_relative_displacement(
      stub + 6,
      (uintptr_t)stub + 10,
      fallback) ||
    !write_relative_displacement(
      stub + 15,
      (uintptr_t)stub + 19,
      transform) ||
    !write_relative_displacement(
      stub + 24,
      (uintptr_t)stub + 28,
      fallback)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }

  stub[28] = 0x9C;
  stub[29] = 0x60;
  stub[30] = 0x53;
  if (
    !build_relative_instruction(
      stub + 31,
      (uintptr_t)stub + 31,
      0xE8,
      (uintptr_t)handle_pixie_transformer_click)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }
  stub[36] = 0xA3;
  {
    DWORD result_address = (DWORD)(uintptr_t)&last_garden_click_consumed;
    stub[37] = (BYTE)(result_address & 0xFF);
    stub[38] = (BYTE)((result_address >> 8) & 0xFF);
    stub[39] = (BYTE)((result_address >> 16) & 0xFF);
    stub[40] = (BYTE)((result_address >> 24) & 0xFF);
  }
  stub[41] = 0x61;
  stub[42] = 0x9D;
  stub[43] = 0x83;
  stub[44] = 0x3D;
  {
    DWORD result_address = (DWORD)(uintptr_t)&last_garden_click_consumed;
    stub[45] = (BYTE)(result_address & 0xFF);
    stub[46] = (BYTE)((result_address >> 8) & 0xFF);
    stub[47] = (BYTE)((result_address >> 16) & 0xFF);
    stub[48] = (BYTE)((result_address >> 24) & 0xFF);
  }
  stub[49] = 0x00;
  stub[50] = 0x0F;
  stub[51] = 0x84;
  if (
    !write_relative_displacement(
      stub + 52,
      (uintptr_t)stub + 56,
      fallback) ||
    !build_relative_instruction(
      stub + 56,
      (uintptr_t)stub + 56,
      0xE9,
      TOWN_CLICK_HANDLER_CLEANUP_ADDRESS)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }

  stub[61] = 0x8B;
  stub[62] = 0x45;
  stub[63] = 0x08;
  stub[64] = 0x85;
  stub[65] = 0xC0;
  if (
    !build_relative_instruction(
      stub + 66,
      (uintptr_t)stub + 66,
      0xE9,
      GARDEN_HANDLER_CONTINUE_ADDRESS) ||
    !build_relative_instruction(
      replacement,
      GARDEN_HANDLER_ADDRESS,
      0xE9,
      (uintptr_t)stub)) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }
  FlushInstructionCache(GetCurrentProcess(), stub, 71);
  if (!safe_write(GARDEN_HANDLER_ADDRESS, replacement, sizeof(replacement))) {
    VirtualFree(stub, 0, MEM_RELEASE);
    return FALSE;
  }
  return TRUE;
}

static DWORD WINAPI patch_thread(LPVOID) {
  uintptr_t pixie_horde_growth = 0;
  uintptr_t sprite_horde_growth = 0;
  BOOL growth_suppressed;
  BOOL click_hook_installed;

  append_text("Pixie Transformer runtime 7\r\n");
  append_text(
    "town=Conflux click=Garden 18/19 "
    "target=Pixie(118) "
    "native Bone Dragon(68)->Firebird(130) conversion=1:1\r\n");
  spiritism_module = load_spiritism_runtime();
  append_text("Conflux Spiritism runtime=");
  append_text(spiritism_module != NULL ? "loaded\r\n" : "failed\r\n");
  if (spiritism_module == NULL) {
    append_text("final=failed; prerequisite runtime was not loaded\r\n");
    write_log();
    return 0;
  }

  Sleep(250);
  append_text("HotA transformer target table=deferred until Garden click\r\n");
  growth_suppressed = suppress_garden_growth(
    &pixie_horde_growth,
    &sprite_horde_growth);
  append_text("Garden horde growth=");
  append_text(growth_suppressed ? "disabled\r\n" : "failed\r\n");

  click_hook_installed =
    growth_suppressed &&
    install_garden_click_hook();
  if (growth_suppressed && !click_hook_installed) {
    restore_garden_growth(pixie_horde_growth, sprite_horde_growth);
  }
  append_text("Garden click hook=");
  append_text(click_hook_installed ? "installed\r\n" : "failed\r\n");
  append_text(
    "Garden left-click=recruitment disabled; click consumed\r\n");
  append_text("Magic Lantern dwelling=unchanged\r\n");
  append_text("native Skeleton Transformer=unchanged outside Garden dialog\r\n");
  append_text("final=");
  append_text(
    growth_suppressed &&
      click_hook_installed
      ? "Pixie Transformer installed"
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
