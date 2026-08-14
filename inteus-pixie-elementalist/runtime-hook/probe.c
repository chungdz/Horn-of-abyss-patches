#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define LOG_CAPACITY 65536
#define FIRST_HERO 136
#define LAST_HERO 143

typedef uintptr_t(__stdcall *HdCommonGet)(const char *name, uintptr_t fallback);

static HMODULE self_module;
static char log_buffer[LOG_CAPACITY];
static DWORD log_length;

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

static void append_hex8(BYTE value) {
  append_hex_digit((BYTE)(value >> 4));
  append_hex_digit((BYTE)(value & 0x0F));
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

static BOOL safe_read(uintptr_t address, void *destination, SIZE_T size) {
  SIZE_T bytes_read = 0;
  if (address == 0) {
    return FALSE;
  }
  return ReadProcessMemory(
      GetCurrentProcess(),
      (LPCVOID)address,
      destination,
      size,
      &bytes_read) &&
    bytes_read == size;
}

static void append_memory_info(uintptr_t address) {
  MEMORY_BASIC_INFORMATION memory;
  SIZE_T result = VirtualQuery((LPCVOID)address, &memory, sizeof(memory));
  append_text(" virtual-query=");
  if (result != sizeof(memory)) {
    append_text("failed");
    return;
  }
  append_text("base:");
  append_hex32((DWORD)(uintptr_t)memory.BaseAddress);
  append_text(" size:");
  append_hex32((DWORD)memory.RegionSize);
  append_text(" state:");
  append_hex32(memory.State);
  append_text(" protect:");
  append_hex32(memory.Protect);
  append_text(" type:");
  append_hex32(memory.Type);
}

static void append_memory_dump(uintptr_t address, DWORD size) {
  BYTE bytes[96];
  DWORD offset;
  if (size > sizeof(bytes)) {
    size = sizeof(bytes);
  }
  if (!safe_read(address, bytes, size)) {
    append_text("    memory: unreadable\r\n");
    return;
  }
  for (offset = 0; offset < size; offset += 16) {
    DWORD column;
    append_text("    +");
    append_hex32(offset);
    append_text(": ");
    for (column = 0; column < 16 && offset + column < size; column++) {
      append_hex8(bytes[offset + column]);
      append_char(' ');
    }
    append_text("\r\n");
  }
}

static DWORD read_table_entry(uintptr_t table, DWORD index, BOOL *read_ok) {
  DWORD value = 0;
  *read_ok = safe_read(table + index * sizeof(DWORD), &value, sizeof(value));
  return value;
}

static void dump_table(
    const char *name,
    uintptr_t table,
    BOOL inspect_entries) {
  DWORD index;
  append_text("\r\n");
  append_text(name);
  append_text(" base=");
  append_hex32((DWORD)table);
  append_memory_info(table);
  append_text("\r\n");

  for (index = FIRST_HERO; index <= LAST_HERO; index++) {
    BOOL read_ok;
    DWORD value = read_table_entry(table, index, &read_ok);
    append_text("  [");
    append_decimal(index);
    append_text("] = ");
    if (!read_ok) {
      append_text("<unreadable>\r\n");
      continue;
    }
    append_hex32(value);
    append_memory_info(value);
    append_text("\r\n");
    if (inspect_entries && value != 0) {
      append_memory_dump(value, 96);
    }
  }
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
  if (length + sizeof("NyxRuntimeProbe.log") >= MAX_PATH) {
    return;
  }
  {
    const char *name = "NyxRuntimeProbe.log";
    while (*name != '\0') {
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

static DWORD WINAPI probe_thread(LPVOID unused) {
  HMODULE hd_module;
  HdCommonGet hd_common_get;
  uintptr_t hpl_table;
  uintptr_t hps_table;
  uintptr_t default_portraits;
  uintptr_t portrait_count;
  DWORD attempt;
  (void)unused;

  append_text("Nyx runtime portrait probe 1\r\n");
  append_text("Read-only diagnostic; no game memory is modified.\r\n");

  Sleep(2000);
  hd_module = GetModuleHandleA("_HD3_.dll");
  append_text("_HD3_.dll module=");
  append_hex32((DWORD)(uintptr_t)hd_module);
  append_text("\r\n");
  if (hd_module == NULL) {
    append_text("ERROR: _HD3_.dll is not loaded.\r\n");
    write_log();
    return 0;
  }

  hd_common_get = (HdCommonGet)GetProcAddress(hd_module, "HdCommon_Get");
  append_text("HdCommon_Get=");
  append_hex32((DWORD)(uintptr_t)hd_common_get);
  append_text("\r\n");
  if (hd_common_get == NULL) {
    append_text("ERROR: HdCommon_Get export was not found.\r\n");
    write_log();
    return 0;
  }

  hpl_table = 0;
  hps_table = 0;
  default_portraits = 0;
  portrait_count = 0;
  for (attempt = 0; attempt < 60; attempt++) {
    hpl_table = hd_common_get("HotA.HPL_tbl", 0);
    hps_table = hd_common_get("HotA.HPS_tbl", 0);
    default_portraits =
      hd_common_get("HotA.HeroesDefaultPortraits", 0);
    portrait_count = hd_common_get("HotA.PortraitsCount", 0);
    if (
        hpl_table != 0 &&
        hps_table != 0 &&
        default_portraits != 0 &&
        portrait_count > LAST_HERO) {
      break;
    }
    Sleep(250);
  }

  append_text("attempts=");
  append_decimal(attempt < 60 ? attempt + 1 : attempt);
  append_text("\r\nHotA.PortraitsCount=");
  append_decimal((DWORD)portrait_count);
  append_text(" (");
  append_hex32((DWORD)portrait_count);
  append_text(")\r\n");

  if (
      hpl_table == 0 ||
      hps_table == 0 ||
      default_portraits == 0 ||
      portrait_count <= LAST_HERO) {
    append_text("ERROR: portrait runtime tables were not initialized.\r\n");
    write_log();
    return 0;
  }

  append_text("HPS-HPL distance=");
  append_decimal((DWORD)(hps_table - hpl_table));
  append_text("\r\nDefaults-HPS distance=");
  append_decimal((DWORD)(default_portraits - hps_table));
  append_text("\r\n");

  dump_table("HotA.HPL_tbl", hpl_table, TRUE);
  dump_table("HotA.HPS_tbl", hps_table, TRUE);
  dump_table(
      "HotA.HeroesDefaultPortraits",
      default_portraits,
      FALSE);
  append_text("\r\nProbe complete.\r\n");
  write_log();
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved) {
  (void)reserved;
  if (reason == DLL_PROCESS_ATTACH) {
    HANDLE thread;
    self_module = module;
    DisableThreadLibraryCalls(module);
    thread = CreateThread(NULL, 0, probe_thread, NULL, 0, NULL);
    if (thread != NULL) {
      CloseHandle(thread);
    }
  }
  return TRUE;
}
