#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#include "patcher_x86.hpp"

#define EVENT_CAPACITY 8192
#define LOG_CAPACITY (2 * 1024 * 1024)
#define IMAGE_CONSTRUCTOR 0x004EA800

struct ImageEvent {
  DWORD thread_id;
  DWORD caller;
  DWORD object;
  DWORD x;
  DWORD y;
  DWORD width;
  DWORD height;
  DWORD frame;
  DWORD resource_pointer;
  char resource[32];
};

static HMODULE self_module;
static volatile LONG event_count;
static ImageEvent events[EVENT_CAPACITY];
static char log_buffer[LOG_CAPACITY];
static DWORD log_length;
static const char *install_status = "not attempted";

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

static char lower_ascii(char value) {
  if (value >= 'A' && value <= 'Z') {
    return (char)(value + ('a' - 'A'));
  }
  return value;
}

static BOOL contains_def(const char *text) {
  DWORD index;
  for (index = 0; index + 3 < 31 && text[index] != '\0'; index++) {
    if (
      text[index] == '.' &&
      lower_ascii(text[index + 1]) == 'd' &&
      lower_ascii(text[index + 2]) == 'e' &&
      lower_ascii(text[index + 3]) == 'f') {
      return TRUE;
    }
  }
  return FALSE;
}

static int __stdcall image_constructor_hook(LoHook *, HookContext *context) {
  DWORD stack[7];
  char resource[32];
  LONG index;
  DWORD offset;

  if (!safe_read((uintptr_t)context->esp, stack, sizeof(stack))) {
    return EXEC_DEFAULT;
  }
  for (offset = 0; offset < sizeof(resource); offset++) {
    resource[offset] = '\0';
  }
  if (
    !safe_read((uintptr_t)stack[6], resource, sizeof(resource) - 1) ||
    !contains_def(resource)) {
    return EXEC_DEFAULT;
  }

  index = InterlockedIncrement(&event_count) - 1;
  if (index < 0 || index >= EVENT_CAPACITY) {
    return EXEC_DEFAULT;
  }

  events[index].thread_id = GetCurrentThreadId();
  events[index].caller = stack[0];
  events[index].object = (DWORD)context->ecx;
  events[index].x = stack[1];
  events[index].y = stack[2];
  events[index].width = stack[3];
  events[index].height = stack[4];
  events[index].frame = stack[5];
  events[index].resource_pointer = stack[6];
  for (offset = 0; offset < sizeof(events[index].resource); offset++) {
    events[index].resource[offset] = resource[offset];
  }
  return EXEC_DEFAULT;
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

static void append_module(const char *name, HMODULE module) {
  char path[MAX_PATH];
  char memory_name[16];
  DWORD length;
  DWORD index;

  append_text(name);
  append_text(" base=");
  append_hex32((DWORD)(uintptr_t)module);
  append_text(" path=");
  if (module == NULL) {
    append_text("<not loaded>\r\n");
    return;
  }
  length = GetModuleFileNameA(module, path, MAX_PATH);
  if (length == 0 || length >= MAX_PATH) {
    append_text("<unavailable>\r\n");
    return;
  }
  path[length] = '\0';
  append_text(path);
  append_text("\r\n");

  if (name[0] == 'm' && name[1] == 'a') {
    if (safe_read(0x006817DC, memory_name, 9)) {
      memory_name[9] = '\0';
      append_text("  main scenario string=");
      append_text(memory_name);
      append_text("\r\n");
    }
  } else if (name[0] == 'H' && name[1] == 'D') {
    uintptr_t address = (uintptr_t)module + 0x2975F0;
    for (index = 0; index < sizeof(memory_name); index++) {
      memory_name[index] = '\0';
    }
    if (safe_read(address, memory_name, 9)) {
      append_text("  HD scenario string=");
      append_text(memory_name);
      append_text(" at ");
      append_hex32((DWORD)address);
      append_text("\r\n");
    }
  }
}

static void write_log(void) {
  char path[MAX_PATH];
  DWORD length = GetModuleFileNameA(self_module, path, MAX_PATH);
  HANDLE file;
  DWORD written;
  LONG count = event_count;
  LONG index;

  if (length == 0 || length >= MAX_PATH) {
    return;
  }
  while (length != 0 && path[length - 1] != '\\' && path[length - 1] != '/') {
    length--;
  }
  {
    const char *name = "NyxImageTrace.log";
    while (*name != '\0' && length + 1 < MAX_PATH) {
      path[length++] = *name++;
    }
    path[length] = '\0';
  }

  log_length = 0;
  append_text("Nyx image constructor trace 1\r\n");
  append_text("Hook status: ");
  append_text(install_status);
  append_text("\r\n");
  append_module("main", GetModuleHandleA(NULL));
  append_module("HD_HOTA.dll", GetModuleHandleA("HD_HOTA.dll"));
  append_module("HotA.dll", GetModuleHandleA("HotA.dll"));
  append_text("events=");
  append_decimal((DWORD)(count < EVENT_CAPACITY ? count : EVENT_CAPACITY));
  append_text("\r\n\r\n");

  if (count > EVENT_CAPACITY) {
    count = EVENT_CAPACITY;
  }
  for (index = 0; index < count; index++) {
    const ImageEvent *event = &events[index];
    append_text("#");
    append_decimal((DWORD)index);
    append_text(" thread=");
    append_decimal(event->thread_id);
    append_text(" caller=");
    append_hex32(event->caller);
    append_text(" object=");
    append_hex32(event->object);
    append_text(" x=");
    append_decimal(event->x);
    append_text(" y=");
    append_decimal(event->y);
    append_text(" w=");
    append_decimal(event->width);
    append_text(" h=");
    append_decimal(event->height);
    append_text(" frame=");
    append_decimal(event->frame);
    append_text(" resource_ptr=");
    append_hex32(event->resource_pointer);
    append_text(" resource=");
    append_text(event->resource);
    append_text("\r\n");
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

static DWORD WINAPI trace_thread(LPVOID) {
  Patcher *patcher;
  PatcherInstance *instance;
  LoHook *hook;
  DWORD attempt;

  Sleep(1000);
  patcher = GetPatcher();
  if (patcher == NULL) {
    install_status = "GetPatcher failed";
    write_log();
    return 0;
  }
  instance = patcher->CreateInstance((char *)"Nyx.ImageTrace");
  if (instance == NULL) {
    instance = patcher->GetInstance((char *)"Nyx.ImageTrace");
  }
  if (instance == NULL) {
    install_status = "CreateInstance failed";
    write_log();
    return 0;
  }
  hook = instance->WriteLoHook(IMAGE_CONSTRUCTOR, image_constructor_hook);
  if (hook == NULL) {
    install_status = "WriteLoHook failed";
    write_log();
    return 0;
  }
  install_status = "installed";
  for (attempt = 0; attempt < 1200; attempt++) {
    write_log();
    Sleep(500);
  }
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
    thread = CreateThread(NULL, 0, trace_thread, NULL, 0, NULL);
    if (thread != NULL) {
      CloseHandle(thread);
    }
  }
  return TRUE;
}
