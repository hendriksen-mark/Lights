#include "fs_wrapper.h"

#ifdef USE_LITTLEFS
static FsType preferredFs = FS_LITTLEFS;
#elif defined(USE_SD)
static FsType preferredFs = FS_SD;
#else
static FsType preferredFs = FS_NONE;
#endif
static fs::FS *fsys = nullptr;

static FsType currentFs = FS_NONE;
String FsName = "None";

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
    REMOTE_LOG_INFO("fs_begin: try mount LittleFS");
    if (LittleFS.begin())
    {
        REMOTE_LOG_DEBUG("LittleFS Filesystem initialized");
        currentFs = FS_LITTLEFS;
        FsName = "LittleFS";
        fsys = &LittleFS;
        return true;
    }
#endif
    return false;
}

static bool try_sd()
{
#if defined(USE_SD)
    REMOTE_LOG_INFO("fs_begin: try mount SD");

    SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, -1);

    if (SD.begin(SD_CS_PIN, SPI, 4000000)) // Use 4 MHz for SD card
    {
        REMOTE_LOG_DEBUG("SD Filesystem initialized");
        currentFs = FS_SD;
        FsName = "SD";
        fsys = &SD;
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
        {
            return true;
        }
        if (try_littlefs())
        {
            return true;
        }
    }
    else if (preferredFs == FS_LITTLEFS)
    {
        if (try_littlefs())
        {
            return true;
        }
        if (try_sd())
        {
            return true;
        }
    }

    // If no preferred or preferred failed, try whichever is compiled in
    if (try_littlefs())
    {
        return true;
    }
    if (try_sd())
    {
        return true;
    }

    return false;
}

void listDir(const char *dirname, uint8_t levels)
{
    REMOTE_LOG_INFO("Listing directory:", dirname);
    if (!fsys)
    {
        REMOTE_LOG_ERROR("listDir: no filesystem mounted");
        return;
    }

    File root = fsys->open(dirname);
    if (!root)
    {
        REMOTE_LOG_ERROR("Failed to open directory", dirname);
        return;
    }
    if (!root.isDirectory())
    {
        REMOTE_LOG_ERROR("Not a directory", dirname);
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            REMOTE_LOG_INFO("  DIR :", file.name());
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            REMOTE_LOG_INFO("  FILE:", file.name(), "  SIZE:", file.size());
        }
        file = root.openNextFile();
    }
}

bool fs_exists(const char *path)
{
    if (!fsys)
    {
        REMOTE_LOG_ERROR("fs_exists: no filesystem mounted");
        return false;
    }
    bool ex = fsys->exists(path);
    if (!ex)
    {
        REMOTE_LOG_ERROR("fs_exists: not found on", FsName, path);
    }
    return ex;
}

File fs_open(const char *path, const char *mode)
{
    if (!fsys)
    {
        REMOTE_LOG_ERROR("fs_open: no filesystem mounted", path);
        return File();
    }
    return fsys->open(path, mode);
}

bool fs_remove(const char *path)
{
    if (!fsys)
    {
        REMOTE_LOG_ERROR("fs_remove: no filesystem mounted", path);
        return false;
    }
    return fsys->remove(path);
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
        if (!fsys)
            return false;
        File root = fsys->open("/");
        if (!root)
            return false;
        File file = root.openNextFile();
        while (file)
        {
            String name = String(file.name());
            if (name.length() == 0)
            {
                file.close();
                file = root.openNextFile();
                continue;
            }
            if (name.charAt(0) != '/')
                name = "/" + name;
            file.close();
            if (fsys)
                fsys->remove(name.c_str());
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
        if (!fsys)
            return used;
        File root = fsys->open("/");
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
    return fs_open("/", "r");
}

void log_file()
{
    if (fsys)
    {
        LOG_ATTACH_FS_MANUAL(*fsys, LOG_FILE_NAME, FILE_APPEND);
    }
}

void fs_set_preferred(FsType t)
{
    preferredFs = t;
}

FsType fs_get_preferred()
{
    return preferredFs;
}
