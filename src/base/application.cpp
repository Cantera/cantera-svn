#include "application.h"

#include "cantera/base/ctexceptions.h"
#include "cantera/base/ctml.h"
#include "cantera/base/stringUtils.h"
#include "cantera/base/xml.h"

#include <map>
#include <string>
#include <fstream>

using std::string;
using std::endl;

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _MSC_VER
#pragma comment(lib, "advapi32")
#endif

// If running multiple threads in a cpp application, the Application class
// is the only internal object that is single instance with static data.
// Synchronize access to those data structures Using macros to avoid
// polluting code with a lot of ifdef's

#ifdef THREAD_SAFE_CANTERA

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
//! Mutex for input directory access
static boost::mutex  dir_mutex;

//! Mutex for access to string messages
static boost::mutex  msg_mutex;

//! Mutex for creating singeltons within the application object
static boost::mutex  app_mutex;

// Mutex for controlling access to the log file
//static boost::mutex  log_mutex;

//! Mutex for controlling access to XML file storage
static boost::mutex  xml_mutex;

//! Macro for locking input directory access
#define DIR_LOCK() boost::mutex::scoped_lock   d_lock(dir_mutex)

//! Macro for locking access to string messages
#define MSG_LOCK() boost::mutex::scoped_lock   m_lock(msg_mutex)

//! Macro for locking creating singletons in the application state
#define APP_LOCK() boost::mutex::scoped_lock   a_lock(app_mutex)

//! Macro for locking XML file writing
#define XML_LOCK() boost::mutex::scoped_lock   x_lock(xml_mutex)

#ifdef WITH_HTML_LOGS
//static boost::mutex  html_mutex; // html logs
//#define HTML_LOCK() boost::mutex::scoped_lock   h_lock(html_mutex)
#endif

#if defined(BOOST_HAS_WINTHREADS)
#include <windows.h>
typedef unsigned int cthreadId_t ;
class thread_equal
{
public:
    bool operator()(cthreadId_t L, cthreadId_t R) {
        return L == R ;
    }
} ;
cthreadId_t getThisThreadId()
{
    return ::GetCurrentThreadId() ;
}
#elif defined(BOOST_HAS_PTHREADS)

typedef pthread_t cthreadId_t ;
class thread_equal
{
public:
    bool operator()(cthreadId_t L, cthreadId_t R) {
        return pthread_equal(L, R) ;
    }
} ;
cthreadId_t getThisThreadId()
{
    return pthread_self() ;
}
#elif defined(BOOST_HAS_MPTASKS)
typedef MPTaskID  cthreadId_t ;
class thread_equal
{
public:
    bool operator()(cthreadId_t L, cthreadId_t R) {
        return L == R ;
    }
} ;
cthreadId_t getThisThreadId()
{
    return MPCurrentTaskID() ;
}
#endif

#else
#define DIR_LOCK()
#define MSG_LOCK()
#define APP_LOCK()
//#define LOG_LOCK()
#define XML_LOCK()

#ifdef WITH_HTML_LOGS
//#define HTML_LOCK()
#endif

#endif

namespace Cantera {

Application::Messages::Messages() :
    errorMessage(0),
    errorRoutine(0),
    logwriter(0)
#ifdef WITH_HTML_LOGS
    ,xmllog(0),
    current(0),
    loglevel(0),
    loglevels(0),
    loggroups(0)
#endif
{
    // install a default logwriter that writes to standard
    // output / standard error
    logwriter = new Logger();
}

Application::Messages::Messages(const Messages& r) :
    errorMessage(r.errorMessage),
    errorRoutine(r.errorRoutine),
    logwriter(0)
#ifdef WITH_HTML_LOGS
    , xmllog(r.xmllog),
    current(r.current),
    loglevel(r.loglevel),
    loglevels(r.loglevels),
    loggroups(r.loggroups)
#endif
{
    // install a default logwriter that writes to standard
    // output / standard error
    logwriter = new Logger(*(r.logwriter));
}

Application::Messages& Application::Messages::operator=(const Messages& r) {
    if (this == &r) {
        return *this;
    }
    errorMessage = r.errorMessage;
    errorRoutine = r.errorRoutine;
    logwriter = new Logger(*(r.logwriter));
#ifdef WITH_HTML_LOGS
    xmllog = r.xmllog;
    current = r.current;
    loglevel = r.loglevel;
    loglevels = r.loglevels;
    loggroups = r.loggroups;
#endif
    return *this;
}

Application::Messages::~Messages() {
    delete logwriter;
#ifdef WITH_HTML_LOGS
    if (xmllog) {
        write_logfile("orphan");
    }
#endif
}

// Set an error condition in the application class without throwing an exception
void Application::Messages::addError(std::string r, std::string msg)
{
    errorMessage.push_back(msg);
    errorRoutine.push_back(r);
}

// Return the number of errors encountered so far
int Application::Messages::getErrorCount()
{
    return static_cast<int>(errorMessage.size()) ;
}

void Application::Messages::setLogger(Logger* _logwriter)
{
    if (logwriter == _logwriter) {
        return ;
    }
    if (logwriter != 0) {
        delete logwriter;
        logwriter = 0 ;
    }
    logwriter = _logwriter;
}

// Return an integer specifying the application environment.
int Application::Messages::getUserEnv()
{
    return logwriter->env() ;
}

// Write an error message and terminate execution
void Application::Messages::logerror(const std::string& msg)
{
    logwriter->error(msg) ;
}

#ifdef WITH_HTML_LOGS

// Write a message to the screen
void Application::Messages::writelog(const char* pszmsg)
{
    logwriter->write(pszmsg) ;
}

// Write a message to the screen
void Application::Messages::writelog(const std::string& msg)
{
    logwriter->write(msg);
}

// Write an endl to the screen and flush output
void Application::Messages::writelogendl()
{
    logwriter->writeendl();
}

void Application::Messages::beginLogGroup(std::string title, int _loglevel /*=-99*/)
{
    // loglevel is a member of the Messages class.
    if (_loglevel != -99) {
        loglevel = _loglevel;
    } else {
        loglevel--;
    }
    if (loglevel <= 0) {
        return;
    }
    // Add the current loglevel to the vector of loglevels
    loglevels.push_back(loglevel);
    // Add the title of the current logLevel to the vector of titles
    loggroups.push_back(title);
    // If we haven't started an XML tree for the log file, do so here
    if (xmllog == 0) {
        // The top of this tree will have a zero pointer.
        xmllog = new XML_Node("html");
        current = &xmllog->addChild("ul");
    }
    // Add two children to the XML tree.
    current = &current->addChild("li","<b>"+title+"</b>");
    current = &current->addChild("ul");
}

void Application::Messages::addLogEntry(std::string tag, std::string value)
{
    if (loglevel > 0 && current) {
        current->addChild("li",tag+": "+value);
    }
}

void Application::Messages::addLogEntry(std::string tag, doublereal value)
{
    if (loglevel > 0 && current) {
        current->addChild("li",tag+": "+fp2str(value));
    }
}

void Application::Messages::addLogEntry(std::string tag, int value)
{
    if (loglevel > 0 && current) {
        current->addChild("li",tag+": "+int2str(value));
    }
}

void Application::Messages::addLogEntry(std::string msg)
{
    if (loglevel > 0 && current) {
        current->addChild("li",msg);
    }
}

void Application::Messages::endLogGroup(std::string title)
{
    if (loglevel <= 0) {
        return;
    }
    AssertThrowMsg(current, "Application::Messages::endLogGroup",
                   "Error while ending a LogGroup. This is probably due to an unmatched"
                   " beginnning and ending group");
    current = current->parent();
    AssertThrowMsg(current, "Application::Messages::endLogGroup",
                   "Error while ending a LogGroup. This is probably due to an unmatched"
                   " beginnning and ending group");
    current = current->parent();
    // Get the loglevel of the previous level and get rid of
    // vector entry in loglevels.
    loglevel = loglevels.back();
    loglevels.pop_back();
    if (title != "" && title != loggroups.back()) {
        writelog("Logfile error."
                 "\n   beginLogGroup: "+ loggroups.back()+
                 "\n   endLogGroup:   "+title+"\n");
        write_logfile("logerror");
    } else if (loggroups.size() == 1) {
        write_logfile(loggroups.back()+"_log");
        loggroups.clear();
        loglevels.clear();
    } else {
        loggroups.pop_back();
    }
}

void Application::Messages::write_logfile(std::string file)
{
    if (!xmllog) {
        return;
    }
    std::string::size_type idot = file.rfind('.');
    std::string ext = "";
    std::string nm = file;
    if (idot != std::string::npos) {
        ext = file.substr(idot, file.size());
        nm = file.substr(0,idot);
    } else {
        ext = ".html";
        nm = file;
    }

    // see if file exists. If it does, find an integer that
    // can be appended to the name to create the name of a file
    // that does not exist.
    std::string fname = nm + ext;
    std::ifstream f(fname.c_str());
    if (f) {
        int n = 0;
        while (1 > 0) {
            n++;
            fname = nm + int2str(n) + ext;
            std::ifstream f(fname.c_str());
            if (!f) {
                break;
            }
        }
    }

    // Now we have a file name that does not correspond to any
    // existing file. Open it as an output stream, and dump the
    // XML (HTML) tree to it.

    if (xmllog) {
        std::ofstream f(fname.c_str());
        // go to the top of the tree, and write it all.
        xmllog->root().write(f);
        f.close();
        writelog("Log file " + fname + " written.\n");
        delete xmllog;
        xmllog = 0;
        current = 0;
    }
}

#endif // WITH_HTML_LOGS

Application::Application() :
    inputDirs(0),
    stop_on_error(false),
    options(),
    tmp_dir("."),
    xmlfiles(),
    m_sleep("1"),
    pMessenger() {
#if !defined( THREAD_SAFE_CANTERA )
    pMessenger = std::auto_ptr<Messages>(new Messages());
#endif
    // if TMP or TEMP is set, use it for the temporary
    // directory
    char* ctmpdir = getenv("CANTERA_TMPDIR");
    if (ctmpdir != 0) {
        tmp_dir = std::string(ctmpdir);
    } else {
        char* tmpdir = getenv("TMP");
        if (tmpdir == 0) {
            tmpdir = getenv("TEMP");
        }
        if (tmpdir != 0) {
            tmp_dir = std::string(tmpdir);
        }
    }

    // if SLEEP is set, use it as the sleep time
    char* sleepstr = getenv("SLEEP");
    if (sleepstr != 0) {
        m_sleep = std::string(sleepstr);
    }

    // install a default logwriter that writes to standard
    // output / standard error
    //      logwriter = new Logger();
    //#ifdef WITH_HTML_LOGS
    //      // HTML log files
    //      xmllog = 0;
    //      current = 0;
    //      loglevel = 0;
    //#endif
    setDefaultDirectories();
#if defined(THREAD_SAFE_CANTERA)
    Unit::units() ;
#endif
}

Application* Application::Instance() {
    APP_LOCK();
    if (Application::s_app == 0) {
        Application::s_app = new Application();
    }
    return s_app;
}

Application::~Application()
{
    std::map<std::string, XML_Node*>::iterator pos;
    for (pos = xmlfiles.begin(); pos != xmlfiles.end(); ++pos) {
        pos->second->unlock();
        delete pos->second;
        pos->second = 0;
    }
}

void Application::ApplicationDestroy() {
    APP_LOCK();
    if (Application::s_app != 0) {
        delete Application::s_app;
        Application::s_app = 0;
    }
}

void Application::thread_complete()
{
#if defined(THREAD_SAFE_CANTERA)
    pMessenger.removeThreadMessages() ;
#endif
}


XML_Node* Application::get_XML_File(std::string file, int debug)
{
    XML_LOCK();
    std::string path = "";
    path = findInputFile(file);
#ifdef _WIN32
    // RFB: For Windows make the path POSIX compliant so code looking for directory
    // separators is simpler.  Just look for '/' not both '/' and '\\'
    std::replace_if(path.begin(), path.end(),
                    std::bind2nd(std::equal_to<char>(), '\\'), '/') ;
#endif

    string ff = path;
    if (xmlfiles.find(path)
            == xmlfiles.end()) {
        /*
         * Check whether or not the file is XML. If not, it will
         * be first processed with the preprocessor. We determine
         * whether it is an XML file by looking at the file extension.
         */
        string::size_type idot = path.rfind('.');
        string ext;
        if (idot != string::npos) {
            ext = path.substr(idot, path.size());
        } else {
            ext = "";
            idot = path.size();
        }
        if (ext != ".xml" && ext != ".ctml") {
            /*
             * We will assume that we are trying to open a cti file.
             * First, determine the name of the xml file, ff, derived from
             * the cti file.
             * In all cases, we will write the xml file to the current
             * directory.
             */
            string::size_type islash = path.rfind('/');
            if (islash != string::npos) {
                ff = string("./")+path.substr(islash+1,idot-islash - 1) + ".xml";
            } else {
                ff = string("./")+path.substr(0,idot) + ".xml";
            }
            if (debug > 0) {
                writelog("get_XML_File(): Expected location of xml file = " +
                         ff + "\n");
            }
            /*
             * Do a search of the existing XML trees to determine if we have
             * already processed this file. If we have, return a pointer to
             * the processed xml tree.
             */
            if (xmlfiles.find(ff) != xmlfiles.end()) {
            if (debug > 0) {
                writelog("get_XML_File(): File, " + ff +
                         ", was previously read." +
                         " Retrieving the stored xml tree.\n");
            }
                return xmlfiles[ff];
            }
            /*
             * Ok, we didn't find the processed XML tree. Do the conversion
             * to xml, possibly overwriting the file, ff, in the process.
             */
            ctml::ct2ctml(path.c_str(),debug);
        } else {
            ff = path;
        }
        /*
         * Take the XML file ff, open it, and process it, creating an
         * XML tree, and then adding an entry in the map. We will store
         * the absolute pathname as the key for this map.
         */
        std::ifstream s(ff.c_str());

        XML_Node* x = new XML_Node("doc");
        if (s) {
            x->build(s);
            x->lock();
            xmlfiles[ff] = x;
        } else {
            string estring = "cannot open "+ff+" for reading.";
            estring += "Note, this error indicates a possible configuration problem.";
            throw CanteraError("get_XML_File", estring);
        }
    }

    /*
     * Return the XML node pointer. At this point, we are sure that the
     * lookup operation in the return statement will return a valid
     * pointer.
     */
    return xmlfiles[ff];
}


void Application::close_XML_File(std::string file)
{
    XML_LOCK();
    if (file == "all") {
        std::map<string, XML_Node*>::iterator
        b = xmlfiles.begin(),
        e = xmlfiles.end();
        for (; b != e; ++b) {
            b->second->unlock();
            delete b->second;
            xmlfiles.erase(b->first);
        }
    } else if (xmlfiles.find(file) != xmlfiles.end()) {
        xmlfiles[file]->unlock();
        delete xmlfiles[file];
        xmlfiles.erase(file);
    }
}

#ifdef _WIN32
long int Application::readStringRegistryKey(const std::string& keyName, const std::string& valueName,
        std::string& value, const std::string& defaultValue)
{

    HKEY key;
    long open_error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName.c_str(), 0, KEY_READ, &key);
    if (open_error != ERROR_SUCCESS) {
        return open_error;
    }
    value = defaultValue;
    CHAR buffer[1024];
    DWORD bufferSize = sizeof(buffer);
    ULONG error;
    error = RegQueryValueEx(key, valueName.c_str(), 0, NULL, (LPBYTE) buffer, &bufferSize);
    if (ERROR_SUCCESS == error) {
        value = buffer;
    }
    RegCloseKey(key);
    return error;
}
#endif

void Application::setTmpDir(std::string tmp)
{
    APP_LOCK();
    tmp_dir = tmp ;
}

std::string Application::getTmpDir()
{
    APP_LOCK();
    return tmp_dir ;
}

std::string Application::sleep()
{
    APP_LOCK();
    return m_sleep ;
}

void Application::Messages::popError()
{
    if (static_cast<int>(errorMessage.size()) > 0) {
        errorRoutine.pop_back() ;
        errorMessage.pop_back() ;
    }
}

std::string Application::Messages::lastErrorMessage()
{
    if (static_cast<int>(errorMessage.size()) > 0) {
        string head =
            "\n\n************************************************\n"
            "                Cantera Error!                  \n"
            "************************************************\n\n";
        return head+string("\nProcedure: ")+errorRoutine.back()
               +string("\nError:   ")+errorMessage.back();
    } else  {
        return "<no Cantera error>";
    }
}

void Application::Messages::getErrors(std::ostream& f)
{
    int i = static_cast<int>(errorMessage.size());
    if (i == 0) {
        return;
    }
    f << endl << endl;
    f << "************************************************" << endl;
    f << "                   Cantera Error!                  " << endl;
    f << "************************************************" << endl
      << endl;
    int j;
    for (j = 0; j < i; j++) {
        f << endl;
        f << "Procedure: " << errorRoutine[j] << endl;
        f << "Error:     " << errorMessage[j] << endl;
    }
    f << endl << endl;
    errorMessage.clear();
    errorRoutine.clear();
}

void Application::Messages::logErrors()
{
    int i = static_cast<int>(errorMessage.size());
    if (i == 0) {
        return;
    }
    writelog("\n\n");
    writelog("************************************************\n");
    writelog("                   Cantera Error!                  \n");
    writelog("************************************************\n\n");
    int j;
    for (j = 0; j < i; j++) {
        writelog("\n");
        writelog(string("Procedure: ")+ errorRoutine[j]+" \n");
        writelog(string("Error:     ")+ errorMessage[j]+" \n");
    }
    writelog("\n\n");
    errorMessage.clear();
    errorRoutine.clear();
}



void Application::setDefaultDirectories()
{
    std::vector<string>& dirs = inputDirs;

    // always look in the local directory first
    dirs.push_back(".");

#ifdef _WIN32
    // Under Windows, the Cantera setup utility records the installation
    // directory in the registry. Data files are stored in the 'data' and
    // 'templates' subdirectories of the main installation directory.

    std::string installDir;
    readStringRegistryKey("SOFTWARE\\Cantera\\Cantera 2.0", "InstallDir", installDir, "");
    if (installDir != "") {
        dirs.push_back(installDir + "data");
        dirs.push_back(installDir + "templates");
    }
#endif

#ifdef DARWIN
    //
    // add a default data location for Mac OS X
    //
    dirs.push_back("/Applications/Cantera/data");
#endif

    //
    // if environment variable CANTERA_DATA is defined, then add
    // it to the search path
    //
    if (getenv("CANTERA_DATA") != 0) {
        string datadir = string(getenv("CANTERA_DATA"));
        dirs.push_back(datadir);
    }

    // CANTERA_DATA is defined in file config.h. This file is written
    // during the build process (unix), and points to the directory
    // specified by the 'prefix' option to 'configure', or else to
    // /usr/local/cantera.
#ifdef CANTERA_DATA
    string datadir = string(CANTERA_DATA);
    dirs.push_back(datadir);
#endif
}

void Application::addDataDirectory(std::string dir)
{
    DIR_LOCK() ;
    if (inputDirs.size() == 0) {
        setDefaultDirectories();
    }
    string d = stripnonprint(dir);
    size_t m, n = inputDirs.size();

    // don't add if already present
    for (m = 0; m < n; m++) {
        if (d == inputDirs[m]) {
            return;
        }
    }

    inputDirs.push_back(d);
}

std::string Application::findInputFile(std::string name)
{
    DIR_LOCK() ;
    string::size_type islash = name.find('/');
    string::size_type ibslash = name.find('\\');
    string inname;
    std::vector<string>& dirs = inputDirs;

    int nd;
    if (islash == string::npos && ibslash == string::npos) {
        nd = static_cast<int>(dirs.size());
        int i;
        inname = "";
        for (i = 0; i < nd; i++) {
            inname = dirs[i] + "/" + name;
            std::ifstream fin(inname.c_str());
            if (fin) {
                fin.close();
                return inname;
            }
        }
        string msg;
        msg = "\nInput file " + name
              + " not found in director";
        msg += (nd == 1 ? "y " : "ies ");
        for (i = 0; i < nd; i++) {
            msg += "\n'" + dirs[i] + "'";
            if (i < nd-1) {
                msg += ", ";
            }
        }
        msg += "\n\n";
        msg += "To fix this problem, either:\n";
        msg += "    a) move the missing files into the local directory;\n";
        msg += "    b) define environment variable CANTERA_DATA to\n";
        msg += "         point to the directory containing the file.";
        throw CanteraError("findInputFile", msg);
    }

    return name;
}

Application* Application::s_app = 0;

} // namespace Cantera
