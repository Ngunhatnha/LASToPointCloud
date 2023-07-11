#include "Utils.h"
#include <dirent.h>
#include <cstring>
#include <unistd.h>     /*argument*/
#include <string.h>
#include <string>

#include <locale>
#include <codecvt>
#include <iomanip>
#include <sys/resource.h>
#include <sys/stat.h>
#include <map>

#include <stdio.h>
#include <fcntl.h>
#include <zip.h>
#include <cassert>

#include <uuid/uuid.h> /* GUID */
#define SUCCESS_STAT 0

std::set<unsigned int> m_setColor;

double m_circleSegments_distance = CS_ALTITUDE_PIPE_CROSS_TO_SEGMENT;

Utils::Utils()
{
}


Utils::~Utils()
{
}
double Utils::getCircleSegmentDistance()
{
    return m_circleSegments_distance;
}
/**
 * checks if a specific directory exists
 * @param dir_path the path to check
 * @return if the path exists
 */
bool dirExists(std::string dir_path)
{
    struct stat sb;

    if (stat(dir_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        return true;
    else
        return false;
}
bool Utils::FileExists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
/**
 * deletes all the files in a folder (but not the folder itself). optionally
 * this can traverse subfolders and delete all contents when recursive is true
 * @param dirpath the directory to delete the contents of (can be full or
 * relative path)
 * @param recursive true = delete all files/folders in all subfolders
 *                  false = delete only files in toplevel dir
 * @return SUCCESS_STAT on success
 *         errno on failure, values can be from unlink or rmdir
 * @note this does NOT delete the named directory, only its contents
 */
int Utils::DeleteFilesInDirectory(std::string dirpath, bool recursive)
{
    if (dirpath.empty())
        return SUCCESS_STAT;

    DIR* theFolder = opendir(dirpath.c_str());
    struct dirent* next_file;
    char filepath[1024];
    int ret_val;

    if (theFolder == NULL)
        return errno;

    while ((next_file = readdir(theFolder)) != NULL)
    {
        // build the path for each file in the folder
        sprintf(filepath, "%s/%s", dirpath.c_str(), next_file->d_name);

        //we don't want to process the pointer to "this" or "parent" directory
        if ((strcmp(next_file->d_name, "..") == 0) ||
            (strcmp(next_file->d_name, ".") == 0))
        {
            continue;
        }

        //dirExists will check if the "filepath" is a directory
        if (dirExists(filepath))
        {
            if (!recursive)
                //if we aren't recursively deleting in subfolders, skip this dir
                continue;

            ret_val = DeleteFilesInDirectory(filepath, recursive);

            if (ret_val != SUCCESS_STAT)
            {
                closedir(theFolder);
                return ret_val;
            }
        }

        ret_val = remove(filepath);
        //ENOENT occurs when i folder is empty, or is a dangling link, in
        //which case we will say it was a success because the file is gone
        if (ret_val != SUCCESS_STAT && ret_val != ENOENT)
        {
            closedir(theFolder);
            return ret_val;
        }

    }

    closedir(theFolder);

    return SUCCESS_STAT;
}
bool Utils::CreateThreadFunc(void* (*ThreadFunc)(void*), StackData* pStackData)
{
    //printf("Testing creation of a thread with stack size:  %zu\n", pStackDatasize->dStackSize);

    pthread_attr_t attr;
    bool bRet = false;
    if (pthread_attr_init(&attr) != 0)
    {
        perror("pthread_attr_init");
        return false;
    }
    // minimum 16384 bytes
    int r = pthread_attr_setstacksize(&attr, pStackData->dStackSize);
    if (r == 0)
    {
        pthread_t thread;
        if (pthread_create(&thread, &attr, ThreadFunc, pStackData) == 0)
        {
            // Wait for thread to exit
            void* res;
            if (pthread_join(thread, &res) != 0)
            {
                perror("pthread_join");
                bRet = false;
            }
            else
                bRet = (bool)(res);
        }
        else
        {
            perror("pthread_create");
            bRet = false;
        }
    }
    else
    {
        perror("pthread_attr_setstacksize");
        printf("pthread_attr_setstacksize returned %i\n", r);
        bRet = false;
    }
    //https://www.linuxquestions.org/questions/linux-kernel-70/call-stack-memory-management-657743/
    if (pthread_attr_destroy(&attr) != 0)
    {
        perror("pthread_attr_destroy");
        bRet = false;
    }
    return bRet;
}

int Utils::CopyFile(const char* to, const char* from)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char* out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}
std::string Utils::validJSON(const char* input)
{
    // convert UNICODE to UTF8
    std::string strRet;
    if (input == NULL)
        return strRet;
    size_t nChar = strlen(input);
    std::string output;
    output.reserve(nChar);

    for (std::string::size_type i = 0; i < nChar; ++i)
    {
        // remove control charracter
        if (input[i] < 32)
            continue;

        switch (input[i]) {
        case '"':
            output += "\\\"";
            break;
        case '/':
            output += "\\/";
            break;
        case '\b':
            output += "\\b";
            break;
        case '\f':
            output += "\\f";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\r";
            break;
        case '\t':
            output += "\\t";
            break;
        case '\\':
            output += "\\\\";
            break;
        case '<':
            output += "\\u003C";
            break;
        case '>':
            output += "\\u003E";
            break;
        case '&':
            output += "\\u0026";
            break;
        default:
            output += input[i];
            break;
        }
    }
    return output;
}
std::string Utils::validJSON(const std::string& input)
{
    // convert UNICODE to UTF8
    size_t nChar = input.size();
    std::string strRet;
    if (nChar == 0)
        return strRet;

    std::string output;
    output.reserve(input.length());

    for (std::string::size_type i = 0; i < input.length(); ++i)
    {
        // remove control charracter
        if (input[i] < 32)
            continue;

        switch (input[i]) {
        case '"':
            output += "\\\"";
            break;
        case '/':
            output += "\\/";
            break;
        case '\b':
            output += "\\b";
            break;
        case '\f':
            output += "\\f";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\r";
            break;
        case '\t':
            output += "\\t";
            break;
        case '\\':
            output += "\\\\";
            break;
        case '<':
            output += "\\u003C";
            break;
        case '>':
            output += "\\u003E";
            break;
        case '&':
            output += "\\u0026";
            break;
        default:
            output += input[i];
            break;
        }
    }
    return output;
}
std::string Utils::UnicodeToJSON(const std::wstring& input)
{
    // convert UNICODE to JSON UNICODE
    std::string output;
    output.reserve(input.length());
    char pBuff[16];
    unsigned short u, hi, lo;
    for (std::wstring::size_type i = 0; i < input.length(); ++i)
    {
        u = input[i];
        if (u > 65535)
        {
            u = u - 0x10000;
            hi = (u & 0xFFC00) >> 10;
            lo = u & 0x003FF;
            hi += 0xD800;
            lo += 0xDC00;
            sprintf(pBuff, "\\u%04X\\u%04X", hi, lo);
            output += std::string(pBuff);
        }
        else
        {
            sprintf(pBuff, "\\u%04X", u);
            output += std::string(pBuff);
        }
    }
    return output;
}
/////////////////////////////////////// zip tool
static bool is_dir(const std::string& dir)
{
    struct stat st;
    ::stat(dir.c_str(), &st);
    return S_ISDIR(st.st_mode);
}
static void walk_directory(const std::string& startdir, const std::string& inputdir, zip_t* zipper)
{
    DIR* dp = ::opendir(inputdir.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Failed to open input directory: " + std::string(::strerror(errno)));
    }

    struct dirent* dirp;
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name != std::string(".") && dirp->d_name != std::string("..")) {
            std::string fullname = inputdir + "/" + dirp->d_name;
            if (is_dir(fullname)) {
                if (zip_dir_add(zipper, fullname.substr(startdir.length() + 1).c_str(), ZIP_FL_ENC_UTF_8) < 0) {
                    throw std::runtime_error("Failed to add directory to zip: " + std::string(zip_strerror(zipper)));
                }
                walk_directory(startdir, fullname, zipper);
            }
            else {
                zip_source_t* source = zip_source_file(zipper, fullname.c_str(), 0, 0);
                if (source == nullptr) {
                    throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                }
                if (zip_file_add(zipper, fullname.substr(startdir.length() + 1).c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
                    zip_source_free(source);
                    throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                }
            }
        }
    }
    ::closedir(dp);
}

bool Utils::zip_directory(const std::string& inputdir, const std::string& output_filename)
{
    int errorp;
    zip_t* zipper = zip_open(output_filename.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if (zipper == nullptr) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        throw std::runtime_error("Failed to open output file " + output_filename + ": " + zip_error_strerror(&ziperror));
        return false;
    }

    try {
        walk_directory(inputdir, inputdir, zipper);
    }
    catch (...) {
        zip_close(zipper);
        throw;
        return false;
    }
    zip_close(zipper);
    return true;
}
bool Utils::zip_directory(const std::string& inputdir, const std::string& baseFileOut, const unsigned int limitSize, std::vector<std::string>& outputVecFilename)
{
    /*auto idx = inputdir.find_last_of("/");
    if (idx == std::string::npos)
        return false;
    std::string dirName = inputdir.substr(idx + 1);    */

    int errorp;
    unsigned int countName = 1;
    std::string output_filename = baseFileOut + "/" + std::to_string(countName) + ".zipxd";

    zip_t* zipper = zip_open(output_filename.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if (nullptr == zipper) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        throw std::runtime_error("Failed to open output file " + output_filename + ": " + zip_error_strerror(&ziperror));
        return false;
    }

    // Open files to get its size
    // Count files, get file names
    DIR* dp = ::opendir(inputdir.c_str());
    if (nullptr == dp) {
        throw std::runtime_error("Failed to open input directory: " + std::string(::strerror(errno)));
        return false;
    }
    struct dirent* dirp;
    unsigned int countSize = 0;
    float compress_ratio = 0.7f;
    std::vector<std::string> lstFileName;
    struct stat st;
    while ((dirp = readdir(dp)) != NULL) {
        if (nullptr == zipper)
        {
            zipper = zip_open(output_filename.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
            if (nullptr == zipper)
            {
                zip_error_t ziperror;
                zip_error_init_with_code(&ziperror, errorp);
                throw std::runtime_error("Failed to open output file " + output_filename + ": " + zip_error_strerror(&ziperror));
                return false;
            }
        }
        // Assume that every files have their size less than the limitSize
        if (dirp->d_name != std::string(".") && dirp->d_name != std::string(".."))
        {
            std::string fullname = inputdir + "/" + dirp->d_name;

            //if (is_dir(fullname)) 
            //{
            //    if (zip_dir_add(zipper, fullname.substr(inputdir.length() + 1).c_str(), ZIP_FL_ENC_UTF_8) < 0) {
            //        throw std::runtime_error("Failed to add directory to zip: " + std::string(zip_strerror(zipper)));
            //    }
            //    //walk_directory(startdir, fullname, zipper);
            //}
            ::stat(fullname.c_str(), &st);
            countSize += st.st_size;
            // Calculate max size by total size * compress_ratio
            if (countSize * compress_ratio <= limitSize)
            {
                lstFileName.push_back(fullname);
            }
            else
            {
                for (auto& i : lstFileName)
                {
                    zip_source_t* source = zip_source_file(zipper, i.c_str(), 0, 0);
                    if (nullptr == source) {
                        throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                        return false;
                    }
                    if (zip_file_add(zipper, i.substr(inputdir.length() + 1).c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
                    {
                        zip_source_free(source);
                        throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                        return false;
                    }
                }
                lstFileName.clear();
                lstFileName.push_back(fullname);
                // Re-calculate the countsize

                countSize = 0;
                ::stat(fullname.c_str(), &st);
                countSize += st.st_size;

                // Output by folder_name + auto increment number
                outputVecFilename.push_back(output_filename);
                countName++;
                output_filename = baseFileOut + "/" + std::to_string(countName) + ".zipxd";
                // reset zipper
                zip_close(zipper);
                zipper = nullptr;
            }
        }
    }
    // After end of loop while, zip the rest of list file
    if (nullptr != zipper)
    {
        for (auto& i : lstFileName)
        {
            zip_source_t* source = zip_source_file(zipper, i.c_str(), 0, 0);
            if (nullptr == source) {
                throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                return false;
            }
            if (zip_file_add(zipper, i.substr(inputdir.length() + 1).c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
            {
                zip_source_free(source);
                throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                return false;
            }
        }
        lstFileName.clear();
        // Store the last zip file name
        outputVecFilename.push_back(output_filename);
    }
    ::closedir(dp);
    zip_close(zipper);
    zipper = nullptr;
    return true;
}
//-----------------------------------------------------------------------------
double Utils::calGeometricError(double radius, double screenPixels)
{
    return radius;
    //return (3.1415926 * radius * radius) / (screenPixels);// *10;
}
std::string Utils::generateGUID()
{
    uuid_t uuid;
    int result = uuid_generate_time_safe(uuid);
    char pBuff[3];
    std::string strGUID;
    for (size_t i = 0; i < sizeof(uuid); i++)
    {
        sprintf(pBuff, "%02x", uuid[i]);
        strGUID += std::string(pBuff);
    }
    return strGUID;
}
void Utils::getRGBA(unsigned int x, float& red, float& green, float& blue, float& alpha)
{
    /*
    - Red component : bits 24 - 31 (hexadecimal mask `0xff000000`)
    - Green component : bits 16 - 23 (hexadecimal mask `0x00ff0000`)
    - Blue component : bits  8 - 15 (hexadecimal mask `0x0000ff00`)
    - Alpha component : bits  0 - 7 (hexadecimal mask `0x000000ff`)
    */
    unsigned int value = (x & 0xff000000) >> 24;
    red = (float)value / 255.0f;
    value = (x & 0x00ff0000) >> 16;
    green = (float)value / 255.0f;
    value = (x & 0x0000ff00) >> 8;
    blue = (float)value / 255.0f;
    alpha = (x & 0x000000ff);
    alpha = (float)value / 255.0f;
}
unsigned int Utils::getRandomRGBA()
{
    srand(time(NULL));
    unsigned int r = (rand() % 256) << 24;
    unsigned int g = (rand() % 256) << 16;
    unsigned int b = (rand() % 256) << 8;
    unsigned int a = rand() % 256;
    unsigned int curValue = (r | g | b | a);
    while (m_setColor.count(curValue))
    {
        r = (rand() % 256) << 24;
        g = (rand() % 256) << 16;
        b = (rand() % 256) << 8;
        a = rand() % 256;
        curValue = (r | g | b | a);
    }
    m_setColor.insert(curValue);
    return curValue;
    /*
    // https://www.cadforum.cz/cadforum_en/table-of-colors-html-x11-names-rgb-components-hex-values-tip8620
    unsigned int colorTable[] = { 0xFFA50000,0xFFA50000,0xEE9A0000,0xCD85000,0x8B5A0000,0xFA807200,0xFF8C6900,0xEE826200,0xCD705400,0x8B4C3900,0xA0522D00,0xFF824700,0xEE794200,0xCD683900,0x8B472600};
    size_t nSize = sizeof(colorTable) / sizeof(unsigned int)-1;
    return colorTable[rand()% nSize];
    */
}
unsigned int Utils::getRandomRGBAForBreakline()
{
    // https://www.cadforum.cz/cadforum_en/table-of-colors-html-x11-names-rgb-components-hex-values-tip8620
    /*unsigned int colorTable[] = { 0xFF00FF00,0xFF00FF00,0xEE00EE00,0xCD00CD00,0x8B008B00,0xB0306000,0xFF34B300,0xEE30A700,0xCD299000,0x8B1C6200,0xDA70D60,0xFF83FA00,0xEE7AE900,0xCD69C900,0x8B478900,0xDDA0DD00,0xFFBBFF00,0xEEAEEE00,0xCD96CD00,0x8B668B00,0xA020F000,0x9B30FF00,0x912CEE00,0x7D26CD00,0x551A8B00 };
    size_t nSize = sizeof(colorTable) / sizeof(unsigned int) - 1;
    return colorTable[rand() % nSize];*/
    return 0xFFFF0000;
}

void Utils::deleteFile(const std::string& strFile)
{
    remove(strFile.c_str());
}

std::vector<std::string> Utils::split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

void Utils::splitFilepath(const std::string& filepath, std::string& dir, std::string& name, std::string& ext)
{
    int lastdot = -1;
    int lastslash = -1;
    int secondslash = -1;
    for (int i = filepath.size() - 1; i >= 0; i--)
    {
        if (lastslash == -1 && (filepath[i] == '/' || filepath[i] == '\\'))
        {
            lastslash = i;
            //break;
        }
        if (lastslash != -1 && secondslash == -1 && (filepath[i] == '/' || filepath[i] == '\\'))
        {
            secondslash = i;
            //break;
        }
        if (lastdot == -1 && filepath[i] == '.')
        {
            lastdot = i;
        }
    }
    ext = filepath.substr(lastdot, filepath.size() - lastdot);
    name = filepath.substr(lastslash + 1, filepath.size() - (lastslash + 1) - ext.size());
    dir = filepath.substr(0, secondslash + 1);

}
std::vector<std::string> Utils::findSubdirs(const std::string& dir)
{
    // These are data types defined in the "dirent" header
    std::vector<std::string> MyVect;
    DIR* theFolder = opendir(dir.c_str());
    if (theFolder == NULL)
        return MyVect;
    struct dirent* next_file;
    while ((next_file = readdir(theFolder)) != NULL)
    {
        if (strcmp(next_file->d_name, ".") == 0 ||
            strcmp(next_file->d_name, "..") == 0)
            MyVect.push_back(next_file->d_name);
    }
    closedir(theFolder);
    return MyVect;
}
EnumOut Utils::ProcessArgumentsLambda(const std::string& strArgv, std::vector<std::string>& vecURL, std::string& outFile, std::string& serverType, std::string& strCRC, std::string& source_geoId, std::string& epsgDest, std::string& geoId,std::vector<std::string>& crop ,std::string& cell_size , std::string& geometry_scale, bool& exportAtributes)
{
    EnumOut mEnumOut = TYPE_NONE;
    std::vector<std::string> strtmp;
    std::vector<std::string> vecArgv;
    std::string del =  ",";

    std::string deltmp = " ";
    int starttmp, endtmp = -1 * deltmp.size();
    do {
        starttmp = endtmp + deltmp.size();
        endtmp = strArgv.find(deltmp, starttmp);
        vecArgv.push_back(strArgv.substr(starttmp, endtmp - starttmp));
    } while (endtmp != -1);

   
    size_t nParam = vecArgv.size();
    size_t i = 0;
    while (i < nParam)
    {
        if (vecArgv[i] == "-s") //server type
        {
            i++;
            if (i < nParam)
            {
                serverType = std::string(vecArgv[i].c_str(), vecArgv[i].size());
                i++;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
        }
        else if (vecArgv[i] == "-T") // LAS tools
        {
            //"https://obt-test-eu.s3.eu-west-1.amazonaws.com/TestPointCloud/Atlantinsilta_12_Nov_2019_LAZ_ETRS89_GK25FIN.laz -s s3 -sour 3879 -dest 4326 ";
            // -s s3 -T source_epsg,source_geoId,dest_epsg,dest_geoid  (LASTOLAS change file's epsg and geoId)
            i++;
            if (i < nParam)
            {
                //str = std::string(vecArgv[i].c_str(), vecArgv[i].size()); 
                int start, end = -1 * del.size();
                do {
                    start = end + del.size();
                    end = vecArgv[i].find(del, start);
                    strtmp.push_back(vecArgv[i].substr(start, end - start));
                } while (end != -1);
                if (strtmp.size() != 4) mEnumOut = TYPE_ERROR_INPUT;
                else 
                {
                    strCRC = strtmp[0];
                    source_geoId = strtmp[1];
                    epsgDest = strtmp[2];
                    geoId = strtmp[3];
                    mEnumOut = TYPE_LAS_TOOL;
                }
                i++;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
         }
        else if (vecArgv[i] == "-C") // Cropping
        {
            //"https://obt-test-eu.s3.eu-west-1.amazonaws.com/TestPointCloud/Atlantinsilta_12_Nov_2019_LAZ_ETRS89_GK25FIN.laz -s s3 -crop 10 10 20 20 ";
            // take point in rectangle (10, 10) to (20, 20)
            // -s s3 -C source_epsg,source_geoId,x_min,y_min,x_max,y_max (Cropping)
            i++;
            if (i < nParam )
            {
                int start, end = -1 * del.size();
                do {
                    start = end + del.size();
                    end = vecArgv[i].find(del, start);
                    strtmp.push_back(vecArgv[i].substr(start, end - start));
                } while (end != -1);
                if (strtmp.size() != 6) mEnumOut = TYPE_ERROR_INPUT;
                else
                {
                    strCRC = strtmp[0];
                    source_geoId = strtmp[1];
                    crop.push_back(std::string(vecArgv[2].c_str(), vecArgv[2].size()));
                    crop.push_back(std::string(vecArgv[3].c_str(), vecArgv[3].size()));
                    crop.push_back(std::string(vecArgv[4].c_str(), vecArgv[4].size()));
                    crop.push_back(std::string(vecArgv[5].c_str(), vecArgv[5].size()));
                    if (crop.size() != 4) mEnumOut = TYPE_ERROR_INPUT;
                    else mEnumOut = TYPE_LAS_TOOL_CROP;
                    i++;
                }
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
        }
        else if (vecArgv[i] == "-P") // PNTS
        {
            //"https://obt-test-eu.s3.eu-west-1.amazonaws.com/TestPointCloud/Atlantinsilta_12_Nov_2019_LAZ_ETRS89_GK25FIN.laz -s s3 -crop 10 10 20 20 ";
            // 
            // -s s3 -P source_epsg,source_geoId,cell_size,geometricError (convert from LAS to Pnts and make tileset)
            i++;
            if (i < nParam + 4)
            {
                int start, end = -1 * del.size();
                do {
                    start = end + del.size();
                    end = vecArgv[i].find(del, start);
                    strtmp.push_back(vecArgv[i].substr(start, end - start));
                } while (end != -1);
                if (strtmp.size() != 4) mEnumOut = TYPE_ERROR_INPUT;
                else
                {
                    strCRC = strtmp[0];
                    source_geoId = strtmp[1];
                    cell_size = strtmp[2];
                    geometry_scale = strtmp[3];
                    mEnumOut = TYPE_LAS_TOOL_PNTS;
                }
                i++;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
        }
        else
        {
            vecURL.push_back(std::string(vecArgv[i].c_str(), vecArgv[i].size()));
            i++;
        }
    }
    if (mEnumOut == TYPE_GET_POSITION || mEnumOut == TYPE_GET_DISTANCE || mEnumOut == TYPE_GET_INV_POSITION || mEnumOut == TYPE_MERGE_JSON || mEnumOut == TYPE_GET_HEIGHT || mEnumOut == TYPE_GET_CVPOSITION ||
        mEnumOut == TYPE_PROJECT_CENTER || mEnumOut == TYPE_PROJECT_MATONPLANE || mEnumOut == TYPE_PROJECT_POSONLOCAL || mEnumOut == TYPE_PROJECT_POSONGLOBAL || mEnumOut == TYPE_PROJECT_CVTOGLOBAL || mEnumOut == TYPE_PROJECT_CVTOLOCAL || mEnumOut == TYPE_PROJECT_FIRST_CENTER)
        return mEnumOut;
    if (vecURL.size() == 0)
        mEnumOut = TYPE_ERROR_INPUT;
    else
    {
        if (mEnumOut == TYPE_NONE)
            mEnumOut = TYPE_CONV;
    }
    return mEnumOut;
}
EnumOut Utils::ProcessArguments(const std::string& strArgv, std::vector<std::string>& vecURL, std::string& outFile, std::string& serverType, std::string& strCRC, std::string& epsgDest, std::string& geoId, std::vector<std::string>& crop, bool& exportAtributes)
{
    EnumOut mEnumOut = TYPE_NONE;
    unsigned int nChar = strArgv.length();
    if (nChar == 0)
        return mEnumOut;
    std::string strTmpArgv = strArgv;
    std::vector<unsigned int> vecSpace;
    unsigned int i;
    vecSpace.push_back(0);
    for (i = 0; i < nChar; i++)
    {
        if (strArgv[i] == ' ')
        {
            strTmpArgv[i] = 0;
            vecSpace.push_back(i + 1);
        }
    }
    int argc = vecSpace.size() + 1;
    char** argv = new char* [argc];
    std::string strDumy = "Tileset";
    argv[0] = (char*)strDumy.c_str();
    for (i = 1; i < argc; i++)
        argv[i] = (char*)&(strTmpArgv[vecSpace[i - 1]]);
    mEnumOut = Utils::ProcessArguments(argc, argv, vecURL, outFile, serverType, strCRC, epsgDest, geoId, crop,exportAtributes);
    delete[]argv;
    return mEnumOut;
}
EnumOut Utils::ProcessArguments(int argc, char* argv[], std::vector<std::string>& vecURL, std::string& outFile, std::string& serverType, std::string& strCRC, std::string& epsgDest, std::string& geoId, std::vector<std::string>& crop, bool& exportAtributes)
{
    EnumOut mEnumOut = TYPE_NONE;
    int opt;
    while ((opt = getopt(argc, argv, "f:js:vwm:i:d:q:r:p:az:o:b:h:k:A:B:C:D:")) != -1)
    {
        switch (opt)
        {
        case 'v':     // version       
            return TYPE_VERSION;
            break;
        case 'b':
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_MERGE_JSON;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'f':    // convert to file
        {
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_OUT_FILE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            //printf("option output file : %s\n", titleOutput.c_str());
            break;
        }
        case 'j':   // convert to json 
            mEnumOut = TYPE_OUT_JSON;
            //printf("option output JSON\n"); 
            break;
        case 's':  // server type  
            if (optarg != NULL)
                serverType = std::string(optarg);
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'w':  // convert all point to WGS84
            mEnumOut = TYPE_CONV_WGS84;
            break;
        case 'm':
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_GET_POSITION;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            //printf("option output file : %s\n", titleOutput.c_str());
            break;
        case 'd': // convert 2 points to local coordinate
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_GET_DISTANCE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            //printf("option output file : %s\n", titleOutput.c_str());
            break;
        case 'i': // convert local point to global coordinate
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_GET_INV_POSITION;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'h': // get height
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_GET_HEIGHT;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'k': // convert from local to global
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_GET_CVPOSITION;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'q': // quad tree spatial index
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_SPATIALINDEX_QUADTREE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
            ///////////////////////////////////////////////////////////// new method
        case 'z': // quad tree spatial index base on object size
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_SPATIALINDEX_QUADTREE_OBJECTSIZE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'o': // one object to one file B3DM
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_ONEOBJECT_ONEFILE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
            // end new method
        case 'r': // insert point
        // -r 3878,N2000,24543900.00,6697000.00,0.0,60.405182,24.772099,77.064574
        // EPSG:3878 , GEOID N2000, Insert point [24543900.00,6697000.00,0.0], RefPoint [60.405182,24.772099,77.064574]
            if (optarg != NULL)
            {
                strCRC = std::string(optarg);
                mEnumOut = TYPE_SPATIALINDEX_QUADTREE_INSERT;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'p': // project merge
        // -p TestLandXml/SpatialIndex/Merge/Project_yyp_ayp_kan
            if (optarg != NULL)
            {
                strCRC = std::string(optarg);
                mEnumOut = TYPE_SPATIALINDEX_QUADTREE_MERGE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'a': // export attribute
            exportAtributes = true;
            break;

        case 'A': // get model center 
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_PROJECT_CENTER;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'B': // get Matrix Transform On Plane
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_PROJECT_MATONPLANE;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'C': // convert Position project plane to Global WGS84
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_PROJECT_POSONLOCAL;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
        case 'D': // convert Position On Local project plane to Global WGS84
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_PROJECT_POSONGLOBAL;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
         case 'E': // convert Position On Local project plane to Global WGS84
            if (optarg != NULL)
            {
                outFile = std::string(optarg);
                mEnumOut = TYPE_PROJECT_CVTOLOCAL;
            }
            else
                mEnumOut = TYPE_ERROR_INPUT;
            break;
         case 'F': // convert Position On Local project plane to Global WGS84
             if (optarg != NULL)
             {
                 outFile = std::string(optarg);
                 mEnumOut = TYPE_PROJECT_CVTOGLOBAL;
             }
             else
                 mEnumOut = TYPE_ERROR_INPUT;
             break;
         case 'G':
             if (optarg != NULL)
             {
                 outFile = std::string(optarg);
                 mEnumOut = TYPE_PROJECT_FIRST_CENTER;
             }
             else
                 mEnumOut = TYPE_ERROR_INPUT;
             break;
         case 'Z':
         case 'sour': // LAS tools
             if (optarg != NULL)
             {
                 strCRC = std::string(optarg);
                 mEnumOut = TYPE_LAS_TOOL;
             }
             else
                 mEnumOut = TYPE_ERROR_INPUT;
             break;
         case 'dest': // LAS tools
             if (optarg != NULL)
             {
                 epsgDest = std::string(optarg);
                 mEnumOut = TYPE_LAS_TOOL;
             }
             else
                 mEnumOut = TYPE_ERROR_INPUT;
             break;
         case 'N': // LAS tools
             if (optarg != NULL)
             {
                 geoId = std::string(optarg);
                 mEnumOut = TYPE_LAS_TOOL_GEO;
             }
             else
                 mEnumOut = TYPE_ERROR_INPUT;
             break;
         case 'crop':
             if (optarg != NULL) {
                 //*crop[0] = atof(vecArgv[i].c_str());
                 //i++;
                 //*crop[1] = atof(vecArgv[i].c_str());
                 //i++;
                 //*crop[2] = atof(vecArgv[i].c_str());
                 //i++;
                 //*crop[3] = atof(vecArgv[i].c_str());
                 //i++;
                 mEnumOut = TYPE_LAS_TOOL_CROP;
             }

        default:
            mEnumOut = TYPE_UNKNOWN_OPTION;
            //printf("unknown options\n");
        }
    }
    if (mEnumOut == TYPE_GET_POSITION || mEnumOut == TYPE_GET_DISTANCE || mEnumOut == TYPE_GET_INV_POSITION || mEnumOut == TYPE_GET_HEIGHT || mEnumOut == TYPE_GET_CVPOSITION ||
        mEnumOut == TYPE_PROJECT_CENTER || mEnumOut == TYPE_PROJECT_MATONPLANE || mEnumOut == TYPE_PROJECT_POSONLOCAL || mEnumOut == TYPE_PROJECT_POSONGLOBAL ||
        mEnumOut == TYPE_PROJECT_CVTOLOCAL || mEnumOut == TYPE_PROJECT_CVTOGLOBAL || mEnumOut == TYPE_PROJECT_FIRST_CENTER)
        return mEnumOut;

    for (; optind < argc; optind++)
    {
        /*
        std::string destinationString;
        std::string sourceString = std::string(argv[optind]);
        // Allocate the destination space
        destinationString.resize(sourceString.size());

        // Convert the source string to lower case
        // storing the result in destination string
        std::transform(sourceString.begin(),
            sourceString.end(),
            destinationString.begin(),
            ::tolower);
        */
        vecURL.push_back(argv[optind]);
    }
    //printf("argument : %s\n", argv[optind]);

    if (vecURL.size() == 0)
        mEnumOut = TYPE_ERROR_INPUT;
    else
    {
        if (mEnumOut == TYPE_NONE)
            mEnumOut = TYPE_CONV;
    }
    return mEnumOut;
}
void Utils::deleteAllFiles(const std::string& strDir)
{
    // These are data types defined in the "dirent" header
    DIR* theFolder = opendir(strDir.c_str());
    if (theFolder == NULL)
        return;
    struct dirent* next_file;
    std::string strtmp;
    while ((next_file = readdir(theFolder)) != NULL)
    {
        strtmp = std::string(next_file->d_name);
        if ((strtmp == ".") || (strtmp == ".."))
            continue;
        strtmp = strDir + "/" + strtmp;
        remove(strtmp.c_str());
    }
    closedir(theFolder);
}
std::string Utils::convertUTF16toUTF8(const std::wstring& ws)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    return conv1.to_bytes(ws);
}
std::wstring Utils::convertUTF8toUTF16(const std::string& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(wstr);
}
bool Utils::ChangeStackSize(size_t sizeStack)
{
    const rlim_t kStackSize = sizeStack;// 16 * 1024 * 1024;   // min stack size = 16 MB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                printf("{\"Status\":\"Error setrlimit returned result = %d\"}\n", result);
                return false;
            }
        }
        return true;
    }
    return false;
}
/*
* https://stackoverflow.com/questions/33908790/why-is-pthread-attr-setstacksize-not-working-for-me
* // Call function f with a 256MB stack.
    static int bigstack(void *(*f)(void *), void* userdata) {

      pthread_t thread;
      pthread_attr_t attr;

      // allocate a 256MB region for the stack.
      size_t stacksize = 256*1024*1024;
      pthread_attr_init(&attr);
      pthread_attr_setstacksize(&attr, stacksize);

      int rc = pthread_create(&thread, &attr, f, userdata);
      if (rc){
        printf("ERROR: return code from pthread_create() is %d\n", rc);
        return 0;
      }
      pthread_join(thread, NULL);
      return 1;

    }
*/
bool Utils::getHeight(const Vec3d& vRefPoint, const std::string& strGeoid, double& dHeight)
{
    double geoidHeightRef = 0.0;
    /*if (strGeoid == std::string("N2000"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint.x(), vRefPoint.y(), TYPE_GEOID_N2000, geoidHeightRef))
            return false;
    }
    else if (strGeoid == std::string("N60"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint.x(), vRefPoint.y(), TYPE_GEOID_N60, geoidHeightRef))
            return false;
    }*/
    if (!GeoidPROJCache::GetInstance().getHeight(false, vRefPoint.x(), vRefPoint.y(), strGeoid, geoidHeightRef))
        return false;
    dHeight = vRefPoint.z() + geoidHeightRef;
    return true;
}
bool Utils::convertPointToGlobalCoordinate(const Vec3d& vPoint, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointGlobal)
{
    CesiumPro mCesiumPro;
    mCesiumPro.setVerticalName(strGeoid);
    if (!mCesiumPro.Create(epsgCode))
        return false;    
    if (!mCesiumPro.ProjConvertCoordinate(vPoint, pointGlobal))
        return false;
    double geoidHeight = 0.0;
    //if (GeoidHeightCache::GetInstance().getHeightLinear(pointGlobal.x(), pointGlobal.y(), strGeoid, geoidHeight))
    if (GeoidPROJCache::GetInstance().getHeight(false, pointGlobal.x(), pointGlobal.y(), strGeoid, geoidHeight))
        pointGlobal.z() += geoidHeight;
    return true;
}
bool Utils::convertPointToLocalCoordinate(const Vec3d& pointGlobal, const std::string& epsgCode, const std::string& strGeoid, Vec3d& vPoint)
{
    CesiumPro mCesiumPro;
    mCesiumPro.setVerticalName(strGeoid);
    if (!mCesiumPro.Create("epsg:4326",epsgCode))
        return false;    
    if (!mCesiumPro.ProjConvertCoordinate(pointGlobal, vPoint))
        return false;
    double geoidHeight = 0.0;
    //if (GeoidHeightCache::GetInstance().getHeightLinear(pointGlobal.x(), pointGlobal.y(), strGeoid, geoidHeight))
    if (GeoidPROJCache::GetInstance().getHeight(false, pointGlobal.x(), pointGlobal.y(), strGeoid, geoidHeight))
        vPoint.z() -= geoidHeight;
    return true;
}

bool Utils::getPointOnGlobalCoordinate(const Vec3d& vRefPoint, const Vec3d& vPoint, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointGlobal)
{
    CesiumPro mCesiumPro;
    if (!mCesiumPro.Create("epsg:4979", epsgCode))
        return false;

    Vec3d vRefPointTran;
    if (!mCesiumPro.ProjConvertCoordinate(vRefPoint, vRefPointTran))
        return false;

    double geoidHeightRef = 0.0;
    /*
    if (strGeoid == std::string("N2000"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint.x(), vRefPoint.y(), TYPE_GEOID_N2000, geoidHeightRef))
            return false;
    }
    else if (strGeoid == std::string("N60"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint.x(), vRefPoint.y(), TYPE_GEOID_N60, geoidHeightRef))
            return false;
    }
    */
    if (!GeoidPROJCache::GetInstance().getHeight(false, vRefPoint.x(), vRefPoint.y(), strGeoid, geoidHeightRef))
        return false;

    double h = vRefPoint[2];
    double H = h - geoidHeightRef;


    double llaRef[3], enuOrg[3];
    llaRef[0] = vRefPoint[0];
    llaRef[1] = vRefPoint[1];
    llaRef[2] = vRefPoint[2];

    CartographicConversions mCartographicConversions;
    if (!mCartographicConversions.lla2enu(enuOrg, llaRef, llaRef))
        return false;

    double enu[3], lla[3];
    enu[1] = vPoint[1] - vRefPointTran[0] + enuOrg[0];
    enu[0] = vPoint[0] - vRefPointTran[1] + enuOrg[1];
    enu[2] = vPoint[2] - H + enuOrg[2];

    if (!mCartographicConversions.enu2lla(lla, enu, llaRef))
        return false;

    pointGlobal[0] = lla[0];
    pointGlobal[1] = lla[1];
    pointGlobal[2] = lla[2];
    return true;
}

bool Utils::getPointOnLocalCoordinate(const Vec3d& vRefPoint, const Vec3d& vPoint, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointLocal)
{
    /* if ((epsgCode == "epsg:4326" || epsgCode == "epsg:4979") && (strGeoid == "WGS84"))
     {
         CartographicConversions mCartographicConversions;
         double lla1[3], xyz1[3];
         lla1[0] = vPoint[0]; lla1[1] = vPoint[1]; lla1[2] = vPoint[2];
         mCartographicConversions.lla2xyz(xyz1, lla1);
         pointLocal[0] = xyz1[0]; pointLocal[1] = xyz1[1]; pointLocal[2] = xyz1[2];
         return true;
     }*/

    CesiumPro mCesiumPro;
    if (!mCesiumPro.Create("epsg:4979", epsgCode))
        return false;
    Vec3d vRefPointTran;
    if (!mCesiumPro.ProjConvertCoordinate(vRefPoint, vRefPointTran))
        return false;
    double llaRef[3], enuOrg[3], llaPoint[3], enuPoint[3];
    llaRef[0] = vRefPoint[0];
    llaRef[1] = vRefPoint[1];
    llaRef[2] = vRefPoint[2];

    llaPoint[0] = vPoint[0];
    llaPoint[1] = vPoint[1];
    llaPoint[2] = vPoint[2];

    CartographicConversions mCartographicConversions;
    if (!mCartographicConversions.lla2enu(enuOrg, llaRef, llaRef))
        return false;
    if (!mCartographicConversions.lla2enu(enuPoint, llaPoint, llaRef))
        return false;
    double de = enuPoint[0] - enuOrg[0];
    double dn = enuPoint[1] - enuOrg[1];
    double du = enuPoint[2] - enuOrg[2];
    pointLocal[0] = vRefPointTran[0] + dn;
    pointLocal[1] = vRefPointTran[1] + de;

    double geoidHeightRef = 0.0;
    /*
    if (strGeoid == std::string("N2000"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint.x(), vRefPoint.y(), TYPE_GEOID_N2000, geoidHeightRef))
            return false;
    }
    else if (strGeoid == std::string("N60"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint.x(), vRefPoint.y(), TYPE_GEOID_N60, geoidHeightRef))
            return false;
    }
    */
    if (!GeoidPROJCache::GetInstance().getHeight(false, vRefPoint.x(), vRefPoint.y(), strGeoid, geoidHeightRef))
        return false;

    double h = vRefPoint[2];
    double H = h - geoidHeightRef;
    pointLocal[2] = H + du;
    return true;
}
double Utils::getDistanceOnLocalCoordinate(const Vec3d& vRefPoint1, const Vec3d& vRefPoint2, const Vec3d& vPoint1, const Vec3d& vPoint2, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointLocal1, Vec3d& pointLocal2)
{
    /*
    if ((epsgCode == "epsg:4326" || epsgCode == "epsg:4979") && (strGeoid == "WGS84"))
    {
        CartographicConversions mCartographicConversions;
        double lla1[3], xyz1[3];
        double lla2[3], xyz2[3];

        lla1[0] = vPoint1[0]; lla1[1] = vPoint1[1]; lla1[2] = vPoint1[2];
        lla2[0] = vPoint2[0]; lla2[1] = vPoint2[1]; lla2[2] = vPoint2[2];
        mCartographicConversions.lla2xyz(xyz1, lla1);
        mCartographicConversions.lla2xyz(xyz2, lla2);

        pointLocal1[0] = xyz1[0]; pointLocal1[1] = xyz1[1]; pointLocal1[2] = xyz1[2];
        pointLocal2[0] = xyz2[0]; pointLocal2[1] = xyz2[1]; pointLocal2[2] = xyz2[2];

        double dx = pointLocal2[0] - pointLocal1[0];
        double dy = pointLocal2[1] - pointLocal1[1];
        double dz = pointLocal2[2] - pointLocal1[2];
        return sqrt(dx * dx + dy * dy + dz * dz);
    }
    */
    CesiumPro mCesiumPro;
    if (!mCesiumPro.Create("epsg:4979", epsgCode))
        return -1.0;
    Vec3d vRefPointTran1, vRefPointTran2;
    if (!mCesiumPro.ProjConvertCoordinate(vRefPoint1, vRefPointTran1))
        return -1.0;
    if (!mCesiumPro.ProjConvertCoordinate(vRefPoint2, vRefPointTran2))
        return -1.0;

    double llaRef1[3], llaRef2[3], enuOrg1[3], enuOrg2[3], llaPoint1[3], enuPoint1[3], llaPoint2[3], enuPoint2[3];
    llaRef1[0] = vRefPoint1[0];
    llaRef1[1] = vRefPoint1[1];
    llaRef1[2] = vRefPoint1[2];

    llaRef2[0] = vRefPoint2[0];
    llaRef2[1] = vRefPoint2[1];
    llaRef2[2] = vRefPoint2[2];

    llaPoint1[0] = vPoint1[0];
    llaPoint1[1] = vPoint1[1];
    llaPoint1[2] = vPoint1[2];

    llaPoint2[0] = vPoint2[0];
    llaPoint2[1] = vPoint2[1];
    llaPoint2[2] = vPoint2[2];

    CartographicConversions mCartographicConversions;
    if (!mCartographicConversions.lla2enu(enuOrg1, llaRef1, llaRef1))
        return -1.0;
    if (!mCartographicConversions.lla2enu(enuOrg2, llaRef2, llaRef2))
        return -1.0;
    if (!mCartographicConversions.lla2enu(enuPoint1, llaPoint1, llaRef1))
        return -1.0;
    double de1 = enuPoint1[0] - enuOrg1[0];
    double dn1 = enuPoint1[1] - enuOrg1[1];
    double du1 = enuPoint1[2] - enuOrg1[2];
    pointLocal1[0] = vRefPointTran1[0] + dn1;
    pointLocal1[1] = vRefPointTran1[1] + de1;

    if (!mCartographicConversions.lla2enu(enuPoint2, llaPoint2, llaRef2))
        return -1.0;
    double de2 = enuPoint2[0] - enuOrg2[0];
    double dn2 = enuPoint2[1] - enuOrg2[1];
    double du2 = enuPoint2[2] - enuOrg2[2];

    pointLocal2[0] = vRefPointTran2[0] + dn2;
    pointLocal2[1] = vRefPointTran2[1] + de2;

    double geoidHeightRef = 0.0;
    /*
    if (strGeoid == std::string("N2000"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint1.x(), vRefPoint1.y(), TYPE_GEOID_N2000, geoidHeightRef))
            return -1.0;
    }
    else if (strGeoid == std::string("N60"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint1.x(), vRefPoint1.y(), TYPE_GEOID_N60, geoidHeightRef))
            return -1.0;
    }
    */
    if (!GeoidPROJCache::GetInstance().getHeight(false, vRefPoint1.x(), vRefPoint1.y(), strGeoid, geoidHeightRef))
        return -1.0;

    double H = vRefPoint1[2] - geoidHeightRef;
    pointLocal1[2] = H + du1;


    geoidHeightRef = 0.0;
    /*
    if (strGeoid == std::string("N2000"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint2.x(), vRefPoint2.y(), TYPE_GEOID_N2000, geoidHeightRef))
            return -1.0;
    }
    else if (strGeoid == std::string("N60"))
    {
        if (!GeoidHeightCache::GetInstance().getHeightLinear(vRefPoint2.x(), vRefPoint2.y(), TYPE_GEOID_N60, geoidHeightRef))
            return -1.0;
    }
    */
    if (!GeoidPROJCache::GetInstance().getHeight(false, vRefPoint1.x(), vRefPoint1.y(), strGeoid, geoidHeightRef))
        return -1.0;

    H = vRefPoint2[2] - geoidHeightRef;
    pointLocal2[2] = H + du2;

    double dx = pointLocal2[0] - pointLocal1[0];
    double dy = pointLocal2[1] - pointLocal1[1];
    double dz = pointLocal2[2] - pointLocal1[2];
    return sqrt(dx * dx + dy * dy + dz * dz);
}
//////////////////////// merge model
/////// get model center
bool Utils::getModelCenter(const Vec3d& vInsertPoint, const Matrixd& computedTransform, Vec3d& vModelCenter)
{
    /*
    var m_firstLon=26.5012578;
    var m_firstLat=60.470542;
    var m_firstAlt=0;
    computedTransform
    (-0.44621745927378903,   -0.7786760188065324,  0.441082346931787, 2817739.979551118)
        (0.8949245661167451, -0.388254885232364,   0.21992763594864428, 1407784.9520909644)
        (0,                   0.49287097888677994, 0.8701024067149724, 5527484.862865507)
        (0,                   0,                    0, 1)

    Debug
    matENUtoECEF
        (-0.44621745927378903, -0.7786760188065324, 0.441082346931787, 2820439.949237701)
        (0.8949245661167451, -0.388254885232364, 0.21992763594864428, 1406296.7939791288)
        (0, 0.49287097888677994, 0.8701024067149724, 5526503.422889957)
        (0, 0, 0, 1)
    matInv
        (-0.446217459273789, 0.8949245661167451, -2.7755575615628914e-17, 0)
        (-0.7786760188065324, -0.388254885232364, 0.49287097888677994, 18357.399443406146)
        (0.441082346931787, 0.21992763594864426, 0.8701024067149724, -6361953.730507008)
        (0, 0, 0, 0.9999999999999999)
    matTran
        (1, -1.3679917723241062e-17, 3.605382472710794e-18, 2536.562866211017)
        (0, 1, 0, 2008.3402709960938)
        (-2.7755575615628914e-17, 0, 1, -9.668585777282715)
        (0, 0, 0, 0.9999999999999999)
        modelCenter
        (2536.562866211017, 2008.3402709960938, -9.668585777282715)
    */
    Matrixd local2world;
    CesiumEllipsoidModel  ellipsoidModel;
    ellipsoidModel.computeLocalToWorldTransformFromLatLongHeight(vInsertPoint[0] * CS_DREEG_TO_RADIAN, vInsertPoint[1] * CS_DREEG_TO_RADIAN, vInsertPoint[2], local2world);
    Matrixd matInv;
    if (!matInv.getInverseMatrix(local2world))
        return false;
    Matrixd matTran;
    matTran.mult(computedTransform, matInv);
    matTran.getTranslate(vModelCenter);
    return true;
}
////// get 
bool Utils::getMatrixTransformOnPlane(const Vec3d& vRefPoint, const Vec3d& vRefPointOnPlane, const Vec3d& localOrg, const Vec3d& vModelCenter,
    double heading, double pitch, double roll, double scale, Matrixd& matTransform, Vec3d& globalOrg)
{
    Vec3d vTran, xyzCenter;
    vTran[0] = localOrg[0] + vModelCenter[0] - vRefPointOnPlane[0];
    vTran[1] = localOrg[1] + vModelCenter[1] - vRefPointOnPlane[1];
    vTran[2] = localOrg[2] + vModelCenter[2] - vRefPointOnPlane[2];
    Matrixd matRotate;
    matRotate.makeRotate(-roll * CS_DREEG_TO_RADIAN, Vec3d(1, 0, 0), -pitch * CS_DREEG_TO_RADIAN, Vec3d(0, 1, 0), -heading * CS_DREEG_TO_RADIAN, Vec3d(0, 0, 1));

    xyzCenter=matRotate.preMult(vTran);
    xyzCenter[0] *= scale;
    xyzCenter[1] *= scale;
    xyzCenter[2] *= scale;

    Matrixd matTranslate;
    matTranslate.makeTranslate(xyzCenter);

    Matrixd matRotateTranslate;
    matRotateTranslate.mult(matRotate, matTranslate);

    Matrixd local2world;
    CesiumEllipsoidModel  ellipsoidModel;
    ellipsoidModel.computeLocalToWorldTransformFromLatLongHeight(vRefPoint[0] * CS_DREEG_TO_RADIAN, vRefPoint[1] * CS_DREEG_TO_RADIAN, vRefPoint[2], local2world);

    Matrixd matPlace;
    matPlace.mult(matRotateTranslate, local2world);

    Matrixd matScale;
    matScale.makeScale(scale, scale, scale);

    matTransform.mult(matScale, matPlace);

    return getPositionOnGlobal(vRefPoint, vRefPointOnPlane, localOrg, globalOrg);
}
bool Utils::getPositionOnGlobal(const Vec3d& vRefPoint, const Vec3d& vRefPointOnPlane, const Vec3d& localPoint, Vec3d& globalPoint)
{
    double llaRef[3], enu[3];
    llaRef[0] = vRefPoint[0];
    llaRef[1] = vRefPoint[1];
    llaRef[2] = vRefPoint[2];
    CartographicConversions mCartographicConversions;
    double xyz[3];
    enu[0] = localPoint[0] - vRefPointOnPlane[0];
    enu[1] = localPoint[1] - vRefPointOnPlane[1];
    enu[2] = localPoint[2] - vRefPointOnPlane[2];
    if (!mCartographicConversions.enu2xyz(xyz, enu, llaRef))
        return false;
    globalPoint[0] = xyz[0];
    globalPoint[1] = xyz[1];
    globalPoint[2] = xyz[2];
    return true;
}
bool Utils::getPositionOnLocal(const Vec3d& vRefPoint, const Vec3d& vRefPointOnPlane, const Vec3d& globalPoint, Vec3d& localPoint)
{
    double llaRef[3];
    llaRef[0] = vRefPoint[0];
    llaRef[1] = vRefPoint[1];
    llaRef[2] = vRefPoint[2];
    double xyz[3], enu[3];
    xyz[0] = globalPoint[0];
    xyz[1] = globalPoint[1];
    xyz[2] = globalPoint[2];

    CartographicConversions mCartographicConversions;
    if (!mCartographicConversions.xyz2enu(enu, xyz, llaRef))
        return false;
    localPoint[0] = enu[0] + vRefPointOnPlane[0];
    localPoint[1] = enu[1] + vRefPointOnPlane[1];
    localPoint[2] = enu[2] + vRefPointOnPlane[2];
    return true;
}

/*
// Call function f with a 256MB stack.
static int bigstack(void *(*f)(void *), void* userdata)
{

      pthread_t thread;
      pthread_attr_t attr;

      // allocate a 256MB region for the stack.
      size_t stacksize = 256*1024*1024;
      pthread_attr_init(&attr);
      pthread_attr_setstacksize(&attr, stacksize);

      int rc = pthread_create(&thread, &attr, f, userdata);
      if (rc){
        printf("ERROR: return code from pthread_create() is %d\n", rc);
        return 0;
      }
      pthread_join(thread, NULL);
      return 1;

 }
*/
/*
struct timespec tStartTime, tEndTime;
clock_gettime(CLOCK_MONOTONIC, &tStartTime);
clock_gettime(CLOCK_MONOTONIC, &tEndTime);
uint32_t nTimeRun=TimeElapsedMs(tStartTime, tEndTime);
printf("Process in : %d ms\n", nTimeRun);
*/