#include "fs_wrapper.h"

static FsType preferredFs = FS_NONE;
static FsType currentFs = FS_NONE;
String FsName = "None";

void fs_set_preferred(FsType t)
{
    preferredFs = t;
}

FsType fs_get_preferred()
{
    return preferredFs;
}

FsType fs_get_current()
{
    return currentFs;
}

String fs_get_name()
{
    return FsName;
}

static bool try_littlefs()
{
#if defined(USE_LITTLEFS)
    if (LittleFS.begin())
    {
        currentFs = FS_LITTLEFS;
        FsName = "LittleFS";
        return true;
    }
#endif
    return false;
}

static bool try_sd()
{
#if defined(USE_SD)
    // SD_CS_PIN should be defined in build flags (platformio.ini)
    if (SD.begin(SD_CS_PIN))
    {
        currentFs = FS_SD;
        FsName = "SD";
        return true;
    }
#endif
    return false;
}

bool fs_begin()
{
    currentFs = FS_NONE;
    FsName = "None";

    // If a preferred FS is set, try it first
    if (preferredFs == FS_SD)
    {
        if (try_sd())
            return true;
        if (try_littlefs())
            return true;
    }
    else if (preferredFs == FS_LITTLEFS)
    {
        if (try_littlefs())
            return true;
        if (try_sd())
            return true;
    }

    // If no preferred or preferred failed, try whichever is compiled in
    if (try_littlefs())
        return true;
    if (try_sd())
        return true;

    return false;
}

bool fs_exists(const char *path)
{
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        return LittleFS.exists(path);
#else
        return false;
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        return SD.exists(path);
#else
        return false;
#endif
    }
    return false;
}

bool fs_exists(const String &path)
{
    return fs_exists(path.c_str());
}

File fs_open(const char *path, const char *mode)
{
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        return LittleFS.open(path, mode);
#else
        return File();
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        return SD.open(path, mode);
#else
        return File();
#endif
    }
    return File();
}

File fs_open(const String &path, const char *mode)
{
    return fs_open(path.c_str(), mode);
}

bool fs_remove(const char *path)
{
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        return LittleFS.remove(path);
#else
        return false;
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        return SD.remove(path);
#else
        return false;
#endif
    }
    return false;
}

bool fs_remove(const String &path)
{
    return fs_remove(path.c_str());
}

bool fs_format()
{
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        return LittleFS.format();
#else
        return false;
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        // SD library doesn't provide a format; emulate by removing files in root
        File root = SD.open("/");
        if (!root)
            return false;
        File file = root.openNextFile();
        while (file)
        {
            String name = String(file.name());
            file.close();
            SD.remove(name);
            file = root.openNextFile();
        }
        root.close();
        return true;
#else
        return false;
#endif
    }
    return false;
}

size_t fs_usedBytes()
{
    size_t used = 0;
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        used = LittleFS.usedBytes();
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        File root = SD.open("/");
        if (root && root.isDirectory())
        {
            File f = root.openNextFile();
            while (f)
            {
                if (!f.isDirectory())
                    used += f.size();
                f = root.openNextFile();
            }
            root.close();
        }
#endif
    }
    return used;
}

size_t fs_totalBytes()
{
    size_t total = 0;
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        total = LittleFS.totalBytes();
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        total = SD.totalBytes();
#endif
    }
    return total;
}

File fs_open_root()
{
    if (currentFs == FS_LITTLEFS)
    {
#if defined(USE_LITTLEFS)
        return LittleFS.open("/");
#else
        return File();
#endif
    }
    else if (currentFs == FS_SD)
    {
#if defined(USE_SD)
        return SD.open("/");
#else
        return File();
#endif
    }
    return File();
}

void log_file()
{
#if defined(USE_LITTLEFS)
    if (currentFs == FS_LITTLEFS)
    {
        LOG_ATTACH_FS_MANUAL(LittleFS, LOG_FILE_NAME, FILE_APPEND);
    }
#endif
#if defined(USE_SD)
    if (currentFs == FS_SD)
    {
        LOG_ATTACH_FS_MANUAL(SD, LOG_FILE_NAME, FILE_APPEND);
    }
#endif
}