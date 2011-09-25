// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

/*! \file stimdefs.h
 *  \author Christoph Schmidt-Hieber
 *  \date 2008-01-16
 *  \brief Common definitions and classes.
 * 
 * 
 *  Header file for common definitions and classes. 
 */

#ifndef _STIMDEFS_H_
#define _STIMDEFS_H_

#include <boost/function.hpp>
#include <vector>
#include <map>
#include <string>

#ifdef _MSC_VER
#pragma warning( disable : 4251 )  // Disable warning messages
#pragma warning( disable : 4996 )  // Disable warning messages
#endif

//! Defines dll export or import functions for Windows
#ifdef _WINDOWS
    #ifdef STFDLL
        #define StfDll __declspec( dllexport )
    #else
        #define StfDll __declspec( dllimport )
    #endif
#else
    #define StfDll
#endif

#ifndef MODULE_ONLY
    #include <wx/wxprec.h>

    #ifdef __BORLANDC__
        #pragma hdrstop
    #endif

    #ifndef WX_PRECOMP
        #include <wx/wx.h>
    #endif

    #include <wx/wfstream.h>
#else
    typedef std::string wxString;
    typedef int wxWindow;
    #define wxT(x)  x
    #define wxCHECK_VERSION(major,minor,release) 0
#endif


class Recording;
class Channel;
class Section;

//! The stimfit namespace.
/*! All essential core functions and classes are in this namespace. 
 *  Its purpose is to reduce name mangling problems.
 */
namespace stf {

/*! \addtogroup stfgen
 *  @{
 */

#ifndef MODULE_ONLY

    
//! Get a Recording, do something with it, return the new Recording.
typedef boost::function<Recording(const Recording&,const Vector_double&,std::map<std::string, double>&)> PluginFunc;

//! User-defined plugin
/*! Class used for extending Stimfit's functionality: 
 *  The client supplies a new menu entry and an ExtFunc 
 *  that will be called upon selection of that entry.
 */
struct Plugin {
    //! Constructor
    /*! \param menuEntry_ Menu entry string for this plugin.
     *  \param pluginFunc_ Function to be executed by this plugin.
     *  \param input_ Dialog entries required by this plugin.
     */
    Plugin(
            const wxString& menuEntry_,
            const PluginFunc& pluginFunc_,
            const UserInput& input_=UserInput()
    ) : menuEntry(menuEntry_),pluginFunc(pluginFunc_),input(input_)
    {
        id = n_plugins;
        n_plugins++;
    }
    
    //! Destructor
    ~Plugin() { }

    int id;                /*!< The plugin id; set automatically upon construction, so don't touch. */
    static int n_plugins;  /*!< Static plugin counter. Initialised in plugins/plugins.cpp. */
    wxString menuEntry;    /*!< Menu entry string for this plugin. */
    PluginFunc pluginFunc; /*!< The function to be executed by this plugin. */
    UserInput input;       /*!< Dialog entries */
};

#ifdef WITH_PYTHON

//! User-defined Python extension
/*! Class used for extending Stimfit's functionality: 
 *  The client supplies a new menu entry and a Python function 
 *  that will be called upon selection of that entry.
 */
struct Extension {
    //! Constructor
    /*! \param menuEntry_ Menu entry string for this extension.
     *  \param pyFunc_ Python function to be called.
     *  \param description_  Description for this function.
     *  \param requiresFile_ Whether a file needs to be open for this function to work
     */
    Extension(const std::string& menuEntry_, void* pyFunc_,
              const std::string& description_, bool requiresFile_) :
        menuEntry(menuEntry_), pyFunc(pyFunc_),
        description(description_), requiresFile(requiresFile_)
    {
        id = n_extensions;
        n_extensions++;
    }
    
    //! Destructor
    ~Extension() { }

    int id;                /*!< The extension id; set automatically upon construction, so don't touch. */
    static int n_extensions;  /*!< Static extension counter. Initialised in extensions/extensions.cpp. */
    std::string menuEntry;    /*!< Menu entry string for this extension. */
    void* pyFunc;     /*!< Python function to be called. */
    std::string description;  /*!< Description for this function. */
    bool requiresFile;     /*!< Whether a file needs to be open for this function to work */
};
#endif

//! Resource manager for ifstream objects.
struct ifstreamMan {
    
    //! Constructor
    /*! See fstream documentation for details */
    ifstreamMan( const wxString& filename )
    : myStream( filename, wxT("r") ) 
    {}
    
    //! Destructor
    ~ifstreamMan() { myStream.Close(); }
    
    //! The managed stream.
    wxFFile myStream;
};

//! Resource manager for ofstream objects.
struct ofstreamMan {

    //! Constructor
    /*! See fstream documentation for details */
    ofstreamMan( const wxString& filename )
    : myStream( filename, wxT("w") ) 
    {}
    
    //! Destructor
    ~ofstreamMan() { myStream.Close(); }

    //! The managed stream.
    wxFFile myStream;
};

#else
#endif // Module only


//! Text file import filter settings
struct txtImportSettings {
  txtImportSettings() : hLines(1),toSection(true),firstIsTime(true),ncolumns(2),
        sr(20),yUnits(wxT("mV")),yUnitsCh2(wxT("pA")),xUnits(wxT("ms")) {}

    int hLines;            /*!< Number of header lines. */
    bool toSection;        /*!< Import columns into separate sections rather than separate channels. */
    bool firstIsTime;      /*!< First column contains time. */
    int ncolumns;          /*!< Number of columns. */
    double sr;             /*!< Sampling rate. */
    wxString yUnits;    /*!< y units string. */
    wxString yUnitsCh2; /*!< y units string of second channel. */
    wxString xUnits;    /*!< x units string. */
};

//! File types
enum filetype {
    atf,    /*!< Axon text file. */
    abf,    /*!< Axon binary file. */
    axg,    /*!< Axograph binary file. */
    ascii,  /*!< Generic text file. */
    cfs,    /*!< CED filing system. */
    igor,   /*!< Igor binary wave. */
    son,    /*!< CED Son files. */
    hdf5,   /*!< hdf5 files. */
    heka,   /*!< heka files. */
#ifdef WITH_BIOSIG
    biosig, /*!< biosig files. */
#endif
    none    /*!< Undefined file type. */
};


//! Add decimals if you are not satisfied.
const double PI=3.14159265358979323846;

//! Does what it says.
/*! \param toRound The double to be rounded.
 *  \return The rounded integer.
 */
int round(double toRound);

/*@}*/

} // end of namespace

inline int stf::round(double toRound) {
    return toRound <= 0.0 ? int(toRound-0.5) : int(toRound+0.5);
}

// Doxygen-links to documentation of frequently used wxWidgets-classes

/*! \defgroup wxwidgets wxWidgets classes
 *  @{
 */

/*! \class wxApp
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxapp.html (wxWidgets documentation)
 */

/*! \class wxCheckBox
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxcheckbox.html (wxWidgets documentation)
 */

/*! \class wxCommandEvent
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxcommandevent.html (wxWidgets documentation)
 */

/*! \class wxDialog
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxdialog.html (wxWidgets documentation)
 */

/*! \class wxDocMDIChildFrame
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxdocmdichildframe.html (wxWidgets documentation)
 */

/*! \class wxDocMDIParentFrame
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxdocmdiparentframe.html (wxWidgets documentation)
 */

/*! \class wxDocument
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxdocument.html (wxWidgets documentation)
 */

/*! \class wxGrid
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxgrid.html (wxWidgets documentation)
 */

/*! \class wxGridCellCoordsArray
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxgridcellcoordsarray.html (wxWidgets documentation)
 */

/*! \class wxGridTableBase
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxgridtablebase.html (wxWidgets documentation)
 */

/*! \class wxPoint
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxpoint.html (wxWidgets documentation)
 */

/*! \class wxPrintout
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxprintout.html (wxWidgets documentation)
 */

/*! \class wxSize
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxsize.html (wxWidgets documentation)
 */

/*! \class wxString
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxstring.html (wxWidgets documentation)
 */

/*! \class wxThread
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxthread.html (wxWidgets documentation)
 */

/*! \class wxView
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxview.html (wxWidgets documentation)
 */

/*! \class wxWindow
 *  \brief See http://www.wxwidgets.org/manuals/stable/wx_wxwindow.html (wxWidgets documentation)
 */

/*@}*/


/*! \defgroup stdcpp C++ standard library classes
 *  @{
 */

/*! \namespace std
 *  \brief The namespace of the C++ standard library (libstdc++).
 */

/*! \class std::map
 *  \brief See http://www.sgi.com/tech/stl/Map.html (SGI's STL documentation)
 */

/*! \class std::vector
 *  \brief See http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/classstd_1_1valarray.html (gcc's libstdc++ documentation)
 */

/*! \class std::vector
 *  \brief See http://www.sgi.com/tech/stl/Vector.html (SGI's STL documentation)
 */

/*@}*/

#endif

