#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define HERO_ID 140
#define LOG_CAPACITY 32768
#define LOAD_DEF_ADDRESS 0x0055C9C0

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

static_assert(sizeof(ResourceItem) == 0x1C, "Unexpected ResourceItem layout");
static_assert(sizeof(DefGroup) == 0x0C, "Unexpected DefGroup layout");
static_assert(sizeof(LoadedDef) == 0x38, "Unexpected LoadedDef layout");
static_assert(sizeof(DefFrame) == 0x48, "Unexpected DefFrame layout");

typedef LoadedDef *(__thiscall *LoadDef)(const char *name);

static HMODULE self_module;
static char log_buffer[LOG_CAPACITY];
static DWORD log_length;

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
  append_text(" full=");
  append_decimal((DWORD)frame.frame_width);
  append_text("x");
  append_decimal((DWORD)frame.frame_height);
  append_text(" data=");
  append_decimal((DWORD)frame.data_size);
  append_text(" raw=");
  append_hex32((DWORD)(uintptr_t)frame.raw_data);
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

static void append_definition(const char *label, LoadedDef *definition) {
  LoadedDef snapshot;
  append_text(label);
  append_text("=");
  append_hex32((DWORD)(uintptr_t)definition);
  if (!safe_read((uintptr_t)definition, &snapshot, sizeof(snapshot))) {
    append_text(" <unreadable>\r\n");
    return;
  }
  append_text(" name=");
  append_resource_name(&snapshot.resource);
  append_text(" refs=");
  append_decimal((DWORD)snapshot.resource.references);
  append_text(" groups=");
  append_decimal((DWORD)snapshot.groups_count);
  append_text(" size=");
  append_decimal((DWORD)snapshot.width);
  append_text("x");
  append_decimal((DWORD)snapshot.height);
  append_text("\r\n");
}

static BOOL patch_frame(
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

  append_text("\r\nPatch ");
  append_text(target_name);
  append_text(" <- ");
  append_text(source_name);
  append_text("\r\n");
  append_definition("target", target);
  append_definition("source", source);
  if (
    target == NULL ||
    source == NULL ||
    !read_frame(target, HERO_ID, &target_entry, &target_frame) ||
    !read_frame(source, HERO_ID, &source_entry, &source_frame)) {
    append_text("result=invalid definition or frame table\r\n");
    return FALSE;
  }
  append_frame("target frame", target_frame);
  append_frame("source frame", source_frame);
  append_text("target entry=");
  append_hex32((DWORD)(uintptr_t)target_entry);
  append_text(" source entry=");
  append_hex32((DWORD)(uintptr_t)source_entry);
  append_text("\r\n");

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
    const char *name = "NyxRuntimeFix.log";
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
  BYTE specialty[28];
  BOOL small_patched;
  BOOL large_patched;

  Sleep(1500);
  append_text("Nyx runtime specialty frame fix 1\r\n");
  append_text("hero=140\r\n");
  if (safe_read(0x00679A00, specialty, sizeof(specialty))) {
    append_text("live specialty type=");
    append_decimal(*(DWORD *)&specialty[0]);
    append_text(" subtype=");
    append_decimal(*(DWORD *)&specialty[4]);
    append_text("\r\n");
  } else {
    append_text("live specialty record unreadable\r\n");
  }

  small_patched = patch_frame(load_def, "UN32.def", "IX32.def");
  large_patched = patch_frame(load_def, "UN44.def", "IX44.def");
  append_text("\r\nfinal=");
  append_text(
    small_patched && large_patched
      ? "both specialty atlases patched"
      : "one or more atlas patches failed");
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
