#include <stdlib.h>
#include <math.h>
#include <valarray>
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <Python.h>
#include <wx/wxPython/wxPython.h>

#include "stfswig.h"

#include "./../app/app.h"
#include "./../app/doc.h"
#include "./../app/view.h"
#include "./../app/graph.h"
#include "./../app/frame.h"
#include "./../app/dlgs/cursorsdlg.h"
#include "./../core/recording.h"
#include "./../core/fitlib.h"

std::vector< std::valarray<double> > gVector;

void ShowExcept(const std::exception& e) {
    wxString msg;
    msg << wxT("Caught an exception in the python module:\n")
        << wxString( e.what(), wxConvLocal );
    wxGetApp().ExceptMsg( msg );
    return;
}

void ShowError( const wxString& msg ) {
    wxString fullmsg;
    fullmsg << wxT("Error in the python module:\n")
            << msg;
    wxGetApp().ErrorMsg( msg );
    return;
}

wxStfDoc* actDoc() {
    return wxGetApp().GetActiveDoc();
}

bool check_doc( ) {
    if (actDoc() == NULL)  {
        ShowError( wxT("Couldn't find open file") );
        return false;
    }
    return true;
}

bool refresh_graph() {
    if ( !check_doc() ) return false;
    
    wxStfView* pView=(wxStfView*)actDoc()->GetFirstView();
    if ( !pView ) {
        ShowError( wxT("Pointer to view is zero") );
        return false;
    }

    wxStfGraph* pGraph = pView->GetGraph();
    if ( !pGraph ) {
        ShowError( wxT("Pointer to graph is zero") );
        return false;
    }
    pGraph->Refresh();
    return true;
}

void _get_trace_fixedsize( double* outvec, int size, int trace, int channel ) {
    if ( !check_doc() ) return;

    if ( trace == -1 ) {
        trace = actDoc()->GetCurSec();
    }
    if ( channel == -1 ) {
        channel = actDoc()->GetCurCh();
    }
    
    // Do the range checking here so the copying will be faster:
    try {
        if ( size > (int)actDoc()->at(channel).at(trace).size() ) {
            ShowError( wxT("Index out of range in get_trace_fixedsize()") );
            return;
        }
    }
    catch ( const std::out_of_range& e) {
        ShowExcept( e );
        return;
    }
    
    // We can now safely perform a fast copy without range checking:
    std::copy( &(*actDoc())[channel][trace][0],
         &(*actDoc())[channel][trace][size],
         outvec);
}

void new_window( double* invec, int size ) {
    if ( !check_doc() ) return;

    std::valarray< double > va(size);
    std::copy( &invec[0], &invec[size], &va[0] );
    Section sec(va);
    Channel ch(sec);
    ch.SetYUnits( actDoc()->at( actDoc()->GetCurCh() ).GetYUnits() );
    Recording new_rec( ch );
    new_rec.SetXScale( actDoc()->GetXScale() );
    wxStfDoc* testDoc = wxGetApp().NewChild( new_rec, actDoc(), wxT("From python") );
    if ( testDoc == NULL ) {
        ShowError( wxT("Failed to create a new window.") );
    }
}

void _new_window_gVector( ) {
    if ( !check_doc() ) return;

    Channel ch( gVector.size() );
    for ( std::size_t n_c = 0; n_c < gVector.size(); ++n_c ) {
        ch.InsertSection( Section(gVector[n_c]), n_c );
    }
    ch.SetYUnits( actDoc()->at( actDoc()->GetCurCh() ).GetYUnits() );
    Recording new_rec( ch );
    new_rec.SetXScale( actDoc()->GetXScale() );
    wxStfDoc* testDoc = wxGetApp().NewChild( new_rec, actDoc(), wxT("From python") );
    if ( testDoc == NULL ) {
        ShowError( wxT("Failed to create a new window.") );
    }
}

void new_window_matrix( double* invec, int traces, int size ) {
    if ( !check_doc() ) return;
    Channel ch( traces );
    for (int n = 0; n < traces; ++n) {
        std::size_t offset = n * size;
        std::valarray< double > va(size);
        std::copy( &invec[offset], &invec[offset+size], &va[0] );
        Section sec(va);
        ch.InsertSection(sec, n);
    }
    ch.SetYUnits( actDoc()->at( actDoc()->GetCurCh() ).GetYUnits() );
    Recording new_rec( ch );
    new_rec.SetXScale( actDoc()->GetXScale() );
    wxStfDoc* testDoc = wxGetApp().NewChild( new_rec, actDoc(), wxT("From python") );
    if ( testDoc == NULL ) {
        ShowError( wxT("Failed to create a new window.") );
    }
}

bool new_window_selected_this( ) {
    if ( !check_doc() ) return false;

    if ( !actDoc()->OnNewfromselectedThis( ) ) {
        return false;
    }
    return true;
}

bool new_window_selected_all( ) {
    if ( !check_doc() ) return false;
    
    try {
        wxCommandEvent wce;
        wxGetApp().OnNewfromselected( wce );
    }
    catch ( const std::exception& e) {
        ShowExcept( e );
        return false;
    }
    return true;
}

int get_size_trace( int trace, int channel ) {
    if ( !check_doc() ) return 0;

    if ( trace == -1 ) {
        trace = actDoc()->GetCurSec();
    }
    if ( channel == -1 ) {
        channel = actDoc()->GetCurCh();
    }
    
    int size = 0;
    try {
        size = actDoc()->at(channel).at(trace).size();
    }
    catch ( const std::out_of_range& e) {
        ShowExcept( e );
        return 0;
    }
    return size;
}

int get_size_channel( int channel ) {
    if ( !check_doc() ) return 0;

    if ( channel == -1 ) {
        channel = actDoc()->GetCurCh();
    }
    
    int size = 0;
    try {
        size = actDoc()->at(channel).size();
    }
    catch ( const std::out_of_range& e) {
        ShowExcept( e );
        return 0;
    }
    return size;
}

const char* get_recording_time( ) {
    if ( !check_doc() ) return 0;
    return actDoc()->GetTime().utf8_str();
}

const char* get_recording_date( ) {
    if ( !check_doc() ) return 0;
    return actDoc()->GetDate().utf8_str();
}

bool select_trace( int trace ) {
    if ( !check_doc() ) return false;
    int max_size = (int)actDoc()->at(actDoc()->GetCurCh()).size();
    if (trace < -1 || trace >= max_size) {
        wxString msg;
        msg << wxT("Select a trace with a zero-based index between 0 and ") << max_size-1;
        ShowError( msg );
        return false;
    }
    if ((int)actDoc()->GetSelectedSections().size() == max_size) {
        ShowError(wxT("No more traces can be selected\nAll traces are selected"));
        return false;
    }
    if ( trace == -1 ) {
        trace = actDoc()->GetCurSec();
    }

    // control whether trace has already been selected:
    bool already=false;
    for (c_st_it cit = actDoc()->GetSelectedSections().begin();
         cit != actDoc()->GetSelectedSections().end() && !already;
         ++cit) { 
        if ((int)*cit == trace) {
            already = true;
        }
    }

    // add trace number to selected numbers, print number of selected traces
    if (!already) {
        actDoc()->SelectTrace( trace );
        //String output in the trace navigator
        wxStfChildFrame* pFrame = (wxStfChildFrame*)actDoc()->GetDocumentWindow();
        if ( !pFrame ) {
            ShowError( wxT("Pointer to frame is zero") );
            return false;
        }
        pFrame->SetSelected(actDoc()->GetSelectedSections().size());
    } else {
        ShowError( wxT("Trace is already selected") );
        return false;
    }
    return true;
}

void select_all( ) {
    if ( !check_doc() ) return;
    
    wxCommandEvent wce;
    actDoc()->Selectall( wce );
}

void unselect_all( ) {
    if ( !check_doc() ) return;
    
    wxCommandEvent wce;
    actDoc()->Deleteselected( wce );
}

PyObject* get_selected_indices() {
    if ( !check_doc() ) return NULL;
    
    if ( actDoc()->GetSelectedSections().empty() ) {
        ShowError( wxT("No selected traces") );
        return NULL;
    }
    PyObject* retObj = PyTuple_New( (int)actDoc()->GetSelectedSections().size() );
    c_st_it cit;
    int n=0;
    for ( cit = actDoc()->GetSelectedSections().begin(); cit != actDoc()->GetSelectedSections().end(); ++cit ) {
        PyTuple_SetItem(retObj, n++, PyInt_FromLong( (long)*cit ) );
    }
    
    // The main program apparently takes the ownership of the tuple;
    // no reference count decrement should be performed here.
    return retObj;
}

bool set_trace( int trace ) {
    if ( !actDoc()->SetSection( trace ) ) {
        return false;
    }
    wxGetApp().OnPeakcalcexecMsg();

    wxStfChildFrame* pFrame = (wxStfChildFrame*)actDoc()->GetDocumentWindow();
    if ( !pFrame ) {
        ShowError( wxT("Pointer to frame is zero") );
        return false;
    }
    pFrame->SetCurTrace( trace );

    return refresh_graph();
}

int get_trace_index() {
    if ( !check_doc() )
        return -1;
    
    return actDoc()->GetCurSec();
}

int get_channel_index( bool active ) {
    if ( !check_doc() )
        return -1;
    
    if ( active )
        return actDoc()->GetCurCh();
    else
        return actDoc()->GetSecCh();        
}

bool subtract_base( ) {
    if ( !check_doc() ) return false;
    
    return actDoc()->SubtractBase();
}

bool file_open( const char* filename ) {
    wxString wxFilename( filename, wxConvLocal );
    return wxGetApp().OpenFilePy( wxFilename );
}

bool file_save( const char* filename ) {
    if ( !check_doc() ) return false;

    wxString wxFilename( filename, wxConvLocal );
    return actDoc()->OnSaveDocument( wxFilename );
}

bool close_all( ) {
    return wxGetApp().CloseAll();
}

bool close_this( ) {
    if ( !check_doc() ) return false;
    return actDoc()->DeleteAllViews( );
}

double peak_index( bool active ) {
    if ( !check_doc() ) return -1.0;

    if ( active ) {
        return actDoc()->GetMaxT();
    } else {
        // Test whether a second channel is available at all:
        if ( actDoc()->size() < 2 ) {
            ShowError( wxT("No second channel found") );
            return -1.0;
        }
        return actDoc()->GetAPMaxT();
    }
}

double maxrise_index( bool active ) {
    if ( !check_doc() ) return -1.0;

    if ( active ) {
        return actDoc()->GetMaxRiseT();
    } else {
        // Test whether a second channel is available at all:
        if ( actDoc()->size() < 2 ) {
            ShowError( wxT("No second channel found") );
            return -1.0;
        }
        return actDoc()->GetAPMaxRiseT();
    }
}

double foot_index( bool active ) {
    if ( !check_doc() ) return -1.0;

    if ( active ) {
        return  actDoc()->GetT20Real() - (actDoc()->GetT80Real() - actDoc()->GetT20Real()) / 3.0;
    } else {
        ShowError( wxT("At this time, foot_index() is only implemented for the active channel") );
        return -1.0;
    }
}

double t50left_index( bool active ) {
    if ( !check_doc() ) return -1.0;

    if ( active ) {
        return actDoc()->GetT50LeftReal();
    } else {
        // Test whether a second channel is available at all:
        if ( actDoc()->size() < 2 ) {
            ShowError( wxT("No second channel found") );
            return -1.0;
        }
        return actDoc()->GetAPT50LeftReal();
    }
}

double t50right_index( bool active ) {
    if ( !check_doc() ) return -1.0;

    if ( active ) {
        return actDoc()->GetT50RightReal();
    } else {
        ShowError( wxT("At this time, t50right_index() is only implemented for the active channel") );
        return -1.0;
    }
}

bool update_cursor_dialog( ) {
    if (wxGetApp().GetCursorsDialog()!=NULL && wxGetApp().GetCursorsDialog()->IsShown()) {
        try {
            wxGetApp().GetCursorsDialog()->UpdateCursors();
        }
        catch (const std::runtime_error& e) {
            ShowExcept( e );
            // We don't necessarily need to return false here.
        }
    }

    return refresh_graph();
}

double get_fit_start( bool is_time ) {
    if ( !check_doc() ) return -1;

    if ( !is_time )
        return actDoc()->GetFitBeg();
    else
        return (double)actDoc()->GetFitBeg() * actDoc()->GetXScale();
}

bool set_fit_start( double pos, bool is_time ) {
    if ( !check_doc() ) return false;

    if ( is_time )
        pos /= actDoc()->GetXScale();
    
    int posInt = stf::round( pos );
    // range check:
    if ( posInt < 0 || posInt >= (int)actDoc()->cur().size() ) {
        ShowError( wxT("Value out of range in set_fit_start()") );
        return false;
    }
    //conversion of pixel on screen to time (inversion of xFormat())
    if (wxGetApp().GetCursorsDialog() != NULL && wxGetApp().GetCursorsDialog()->GetStartFitAtPeak()) {
        ShowError(
                wxT("Fit will start at the peak. Change cursor settings (Edit->Cursor settings) to set manually.") );
        return false;
    }
    
    actDoc()->SetFitBeg( posInt );

    return update_cursor_dialog();
}

double get_fit_end( bool is_time ) {
    if ( !check_doc() ) return -1;

    if ( !is_time )
        return actDoc()->GetFitEnd();
    else
        return (double)actDoc()->GetFitEnd() * actDoc()->GetXScale();
}

bool set_fit_end( double pos, bool is_time ) {
    if ( !check_doc() ) return false;

    if ( is_time )
        pos /= actDoc()->GetXScale();
    
    int posInt = stf::round( pos );

    // range check:
    if ( posInt < 0 || posInt >= (int)actDoc()->cur().size() ) {
        ShowError( wxT("Value out of range in set_fit_end()") );
        return false;
    }
    //conversion of pixel on screen to time (inversion of xFormat())
    if (wxGetApp().GetCursorsDialog() != NULL && wxGetApp().GetCursorsDialog()->GetStartFitAtPeak()) {
        ShowError(
                wxT("Fit will start at the peak. Change cursor settings (Edit->Cursor settings) to set manually.") );
        return false;
    }
    
    actDoc()->SetFitEnd( posInt );

    return update_cursor_dialog();
}

double get_peak_start( bool is_time ) {
    if ( !check_doc() ) return -1;

    if ( !is_time )
        return actDoc()->GetPeakBeg();
    else
        return (double)actDoc()->GetPeakBeg() * actDoc()->GetXScale();
}

bool set_peak_start( double pos, bool is_time ) {
    if ( !check_doc() ) return false;

    if ( is_time )
        pos /= actDoc()->GetXScale();
    
    int posInt = stf::round( pos );

    // range check:
    if ( posInt < 0 || posInt >= (int)actDoc()->cur().size() ) {
        ShowError( wxT("Value out of range in set_peak_start()") );
        return false;
    }
    
    actDoc()->SetPeakBeg( posInt );

    return update_cursor_dialog();
}

double get_peak_end( bool is_time ) {
    if ( !check_doc() ) return -1;

    if ( !is_time )
        return actDoc()->GetPeakEnd();
    else
        return (double)actDoc()->GetPeakEnd() * actDoc()->GetXScale();
}

bool set_peak_end( double pos, bool is_time ) {
    if ( !check_doc() ) return false;

    if ( is_time )
        pos /= actDoc()->GetXScale();
    
    int posInt = stf::round( pos );

    // range check:
    if ( posInt < 0 || posInt >= (int)actDoc()->cur().size() ) {
        ShowError( wxT("Value out of range in set_peak_end()") );
        return false;
    }
    
    actDoc()->SetPeakEnd( posInt );

    return update_cursor_dialog();
}

bool set_peak_mean( int pts ) {
    if ( !check_doc() ) return false;

    // range check (-1 is a legal value!):
    if ( pts == 0 || pts < -1 ) {
        ShowError( wxT("Value out of range in set_peak_mean()") );
        return false;
    }
    
    actDoc()->SetPM( pts );

    return update_cursor_dialog();
}

bool set_peak_direction( const char* direction ) {
    if ( !check_doc() ) return false;

    if ( strcmp( direction, "up" ) == 0 ) {
        actDoc()->SetDirection( stf::up );
        return update_cursor_dialog();
    }

    if ( strcmp( direction, "down" ) == 0 ) {
        actDoc()->SetDirection( stf::down );
        return update_cursor_dialog();
    }

    if ( strcmp( direction, "both" ) == 0 ) {
        actDoc()->SetDirection( stf::both );
        return update_cursor_dialog();
    }

    wxString msg;
    msg << wxT("\"") << wxString::FromAscii(direction) << wxT("\" is not a valid direction\n");
    msg << wxT("Use \"up\", \"down\" or \"both\"");
    ShowError( msg );
    return false;

}

double get_base_start( bool is_time ) {
    if ( !check_doc() ) return -1;

    if ( !is_time )
        return actDoc()->GetBaseBeg();
    else
        return (double)actDoc()->GetBaseBeg() * actDoc()->GetXScale();
}

bool set_base_start( double pos, bool is_time ) {
    if ( !check_doc() ) return false;

    if ( is_time )
        pos /= actDoc()->GetXScale();
    
    int posInt = stf::round( pos );

    // range check:
    if ( posInt < 0 || posInt >= (int)actDoc()->cur().size() ) {
        ShowError( wxT("Value out of range in set_base_start()") );
        return false;
    }
    
    actDoc()->SetBaseBeg( posInt );

    return update_cursor_dialog();
}

double get_base_end( bool is_time ) {
    if ( !check_doc() ) return -1;

    if ( !is_time )
        return actDoc()->GetBaseEnd();
    else
        return (double)actDoc()->GetBaseEnd() * actDoc()->GetXScale();
}

bool set_base_end( double pos, bool is_time ) {
    if ( !check_doc() ) return false;

    if ( is_time )
        pos /= actDoc()->GetXScale();
    
    int posInt = stf::round( pos );

    // range check:
    if ( posInt < 0 || posInt >= (int)actDoc()->cur().size() ) {
        ShowError( wxT("Value out of range in set_base_end()") );
        return false;
    }
    
    actDoc()->SetBaseEnd( posInt );

    return update_cursor_dialog();
}

double get_sampling_interval( ) {
    if ( !check_doc() ) return -1.0;

    return actDoc()->GetXScale();
}

bool set_sampling_interval( double si ) {
    if ( !check_doc() ) return false;

    if (si <= 0) {
        ShowError( wxT("New sampling interval needs to be greater than 0.") );
        return false;
    }

    actDoc()->SetXScale( si );

    return refresh_graph();
}

bool measure( ) {
    if ( !check_doc() )
        return false;

    // check cursor positions:
    if ( actDoc()->GetPeakBeg() > actDoc()->GetPeakEnd() ) {
        ShowError( wxT("Peak window cursors are reversed; will abort now.") );
        return false;
    }
    
    if ( actDoc()->GetBaseBeg() > actDoc()->GetBaseEnd() ) {
        ShowError( wxT("Base window cursors are reversed; will abort now.") );
        return false;
    }

    if ( actDoc()->GetFitBeg() > actDoc()->GetFitEnd() ) {
        ShowError( wxT("Fit window cursors are reversed; will abort now.") );
        return false;
    }
    
    wxStfChildFrame* pFrame = (wxStfChildFrame*)actDoc()->GetDocumentWindow();
    if ( !pFrame ) {
        ShowError( wxT("Pointer to frame is zero") );
        return false;
    }
    
    wxGetApp().OnPeakcalcexecMsg();
    pFrame->UpdateResults();
    return true;
}

double get_base( ) {
    if ( !check_doc() ) return 0.0;
    
    return actDoc()->GetBase();
}

double get_peak( ) {
    if ( !check_doc() ) return 0.0;
    
    return actDoc()->GetPeak();
}

void _gVector_resize( std::size_t size ) {
    gVector.resize( size );   
}

void _gVector_at( double* invec, int size, int at ) {
    std::valarray< double > va(size);
    std::copy( &invec[0], &invec[size], &va[0] );

    gVector[at].resize( va.size() );
    gVector[at] = va;
}

void align_selected(  double (*alignment)( bool ), bool active ) {
    if ( !check_doc() ) return;
    wxStfDoc* pDoc = actDoc();
    
    //store current section:
    std::size_t section_old = pDoc->GetCurSec();

    if ( pDoc->GetSelectedSections().empty() ) {
        ShowError( wxT("No selected traces") );
        return;
    }

    //initialize the lowest and the highest index:
    std::size_t min_index=0;
    try {
        min_index=pDoc->get()[pDoc->GetSecCh()].at(pDoc->GetSelectedSections().at(0)).size()-1;
    }
    catch (const std::out_of_range& e) {
        wxString msg(wxT("Error while aligning\nIt is safer to re-start the program\n"));
        msg+=wxString( e.what(), wxConvLocal );
        ShowError( msg );
        return;
    }

    std::size_t max_index=0;
    std::vector<int> shift( pDoc->GetSelectedSections().size(), 0 );
    int_it it = shift.begin();
    //loop through all selected sections:
    for (c_st_it cit = pDoc->GetSelectedSections().begin(); 
         cit != pDoc->GetSelectedSections().end() && it != shift.end();
         cit++)
    {
        //Set the selected section as the current section temporarily:
        pDoc->SetSection(*cit);
        if ( pDoc->GetPeakAtEnd() ) {
            pDoc->SetPeakEnd((int)pDoc->get()[pDoc->GetSecCh()][*cit].size()-1);
        }
        // Calculate all variables for the current settings
        // APMaxSlopeT will be calculated for the second (==inactive)
        // channel, so channels may not be changed!
        try {
            pDoc->Measure();
        }
        catch (const std::out_of_range& e) {
            ShowExcept( e );
            return;
        }

        //check whether the current index is a max or a min,
        //and if so, store it:
        double alignIndex = alignment( active );
        *it = stf::round( alignIndex );
        if (alignIndex > max_index) {
            max_index=alignIndex;
        }
        if (alignIndex < min_index) {
            min_index=alignIndex;
        }
        it++;
    }
    // now that max and min indices are known, calculate the number of 
    // points that need to be shifted:
    for (int_it it2 = shift.begin(); it2 != shift.end(); it2++) {
        (*it2) -= (int)min_index;
    }
    //restore section and channel settings:
    pDoc->SetSection( section_old );
    
    int new_size=(int)(pDoc->get()[0][pDoc->GetSelectedSections()[0]].size()-(max_index-min_index));

    Recording Aligned( pDoc->size(), pDoc->GetSelectedSections().size(), new_size );

    ch_it chan_it;       // Channel iterator
    std::size_t n_ch = 0;
    for ( ch_it chan_it = pDoc->get().begin();
          chan_it != pDoc->get().end();
          ++chan_it )
    {
        Channel ch( pDoc->GetSelectedSections().size() );
        ch.SetChannelName( pDoc->at(n_ch).GetChannelName() );
        ch.SetYUnits(  pDoc->at(n_ch).GetYUnits() );
        std::size_t n_sec = 0;
		int_it it3 = shift.begin();
        for ( c_st_it sel_it = pDoc->GetSelectedSections().begin(); 
              sel_it != pDoc->GetSelectedSections().end() && it3 != shift.end();
              ++sel_it )
        {
            std::valarray<double> va( new_size );
            std::copy( &(chan_it->at( *sel_it ).get_w()[ 0 + (*it3) ]), 
                       &(chan_it->at( *sel_it ).get_w()[ (*it3) + new_size ]),
                       &va[0] );
            Section sec(va);
            ch.InsertSection(sec, n_sec++);
            ++it3;
        }
        Aligned.InsertChannel( ch, n_ch++ );
    }
    
    wxString title( pDoc->GetTitle() );
    title += wxT(", aligned");
    Aligned.CopyAttributes( *pDoc );
    wxStfDoc* testDoc = wxGetApp().NewChild(Aligned, pDoc, title);
    if ( testDoc == NULL ) {
        ShowError( wxT("Failed to create a new window.") );
    }

    return;
}

int leastsq_param_size( int fselect ) {
    int npar = 0;
    try {
        npar = (int)wxGetApp().GetFuncLib().at(fselect).pInfo.size();
    }
    catch (const std::out_of_range& e) {
        wxString msg( wxT("Could not retrieve function from library:\n") );
        msg << wxString( e.what(), wxConvLocal );
        ShowError(msg);
        return -1;
    }
    return npar;
}

PyObject* leastsq( int fselect, bool refresh ) {
    if ( !check_doc() ) return false;
   
    wxStfDoc* pDoc = actDoc();
    wxCommandEvent wce;

    int n_params = 0;
    try {
        n_params=(int)wxGetApp().GetFuncLib().at(fselect).pInfo.size();
    }
    catch (const std::out_of_range& e) {
        wxString msg( wxT("Could not retrieve function from library:\n") );
        msg << wxString( e.what(), wxConvLocal );
        ShowError(msg);
        return NULL;
    }

    std::valarray< double > x( pDoc->GetFitEnd() - pDoc->GetFitBeg() );
    //fill array:
    std::copy(&pDoc->cur()[pDoc->GetFitBeg()], &pDoc->cur()[pDoc->GetFitEnd()], &x[0]);
    
    std::valarray< double > params( n_params );            

    // initialize parameters from init function,
    wxGetApp().GetFuncLib().at(fselect).init( x, pDoc->GetBase(), pDoc->GetPeak(),
            pDoc->GetXScale(), params );
    wxString fitInfo;
    int fitWarning = 0;
    std::valarray< double > opts( 6 );
    // Respectively the scale factor for initial \mu,
    // stopping thresholds for ||J^T e||_inf, ||Dp||_2 and ||e||_2,
    // maxIter, maxPass
    opts[0]=5*1E-3; //default: 1E-03;
    opts[1]=1E-17; //default: 1E-17;
    opts[2]=1E-17; //default: 1E-17;
    opts[3]=1E-17; //default: 1E-17;
    opts[4]=64; //default: 64;
    opts[5]=16;
    double chisqr = 0.0;
    try {
        chisqr = stf::lmFit( x, pDoc->GetXScale(), wxGetApp().GetFuncLib().at(fselect),
                opts, params, fitInfo, fitWarning );
        pDoc->cur().SetIsFitted( params, wxGetApp().GetFuncLibPtr(fselect),
                chisqr, pDoc->GetFitBeg(), pDoc->GetFitEnd() );
    }
    
    catch (const std::out_of_range& e) {
        ShowExcept( e );
        return NULL;
    }
    catch (const std::runtime_error& e) {
        ShowExcept( e );
        return NULL;
    }
    catch (const std::exception& e) {
        ShowExcept( e );
        return NULL;
    }

    if ( refresh ) {
        if ( !refresh_graph() ) return NULL;
    }
    
    // Dictionaries apparently grow as needed; no initial size is required.
    PyObject* retDict = PyDict_New( );
    for ( std::size_t n_dict = 0; n_dict < params.size(); ++n_dict ) {
         PyDict_SetItemString( retDict, wxGetApp().GetFuncLib()[fselect].pInfo.at(n_dict).desc.char_str(), 
                PyFloat_FromDouble( params[n_dict] ) );
    }
    PyDict_SetItemString( retDict, "SSE", PyFloat_FromDouble( chisqr ) );
    return retDict;
}

bool show_table( PyObject* dict, const char* caption ) {
    if ( !check_doc() ) return false;

    // Check whether the dictionary is intact:
    if ( !PyDict_Check( dict ) ) {
        ShowError( wxT("First argument to ShowTable() is not a dictionary.") );
        return false;
    }
    std::map< wxString, double > pyMap;
    Py_ssize_t n_dict = 0;
    PyObject *pkey = NULL, *pvalue = NULL;
    while ( PyDict_Next( dict, &n_dict, &pkey, &pvalue ) ) {
        if ( !pkey || !pvalue ) {
            ShowError( wxT("Couldn't read from dictionary in ShowTable().") );
            return false;
        }
        wxString key = wxString( PyString_AsString( pkey ), wxConvLocal );
        double value = PyFloat_AsDouble( pvalue );
        pyMap[key] = value;
    }
    stf::Table pyTable( pyMap );

    wxStfChildFrame* pFrame = (wxStfChildFrame*)actDoc()->GetDocumentWindow();
    if ( !pFrame ) {
        ShowError( wxT("Pointer to frame is zero") );
        return false;
    }
    pFrame->ShowTable( pyTable, wxString( caption, wxConvLocal ) );
    return true;
}

bool show_table_dictlist( PyObject* dict, const char* caption, bool reverse ) {
    if ( !check_doc() ) return false;
    
    if ( !reverse ) {
        ShowError( wxT("Row-major order (reverse = False) has not been implemented yet.") );
        return false;
    }
    
    // Check whether the dictionary is intact:
    if ( !PyDict_Check( dict ) ) {
        ShowError( wxT("First argument to ShowTable() is not a dictionary.") );
        return false;
    }
    Py_ssize_t n_dict = 0;
    PyObject *pkey = NULL, *pvalue = NULL;
    std::vector< std::valarray<double> > pyVector;
    std::vector< wxString > pyStrings;
    while ( PyDict_Next( dict, &n_dict, &pkey, &pvalue ) ) {
        if ( !pkey || !pvalue ) {
            ShowError( wxT("Couldn't read from dictionary in ShowTable().") );
            return false;
        }
        pyStrings.push_back( wxString( PyString_AsString( pkey ), wxConvLocal ) );
        if ( !PyList_Check( pvalue ) ) {
            ShowError( wxT("Dictionary values are not (consistently) lists.") );
            return false;
        }
        std::valarray<double> values( PyList_Size( pvalue ) );
        for (int n_list = 0; n_list < (int)values.size(); ++n_list ) {
            PyObject* plistvalue = PyList_GetItem( pvalue, n_list );
            if ( !plistvalue ) {
                ShowError( wxT("Can't read list elements in show_table().") );
                return false;
            }            
            values[n_list] = PyFloat_AsDouble( plistvalue );
        }
        pyVector.push_back( values );
    }
    if ( pyVector.empty() ) {
        ShowError( wxT("Dictionary was empty in show_table().") );
        return false;
    }
    stf::Table pyTable( pyVector[0].size(), pyVector.size() );
    std::vector< std::valarray< double > >::const_iterator c_va_it;
    std::size_t n_col = 0;
    for (  c_va_it = pyVector.begin(); c_va_it != pyVector.end(); ++c_va_it ) {
        try {
            pyTable.SetColLabel( n_col, pyStrings[n_col] );
            for ( std::size_t n_va=0; n_va < (*c_va_it).size(); ++n_va ) {
                pyTable.at( n_va, n_col ) = (*c_va_it)[n_va];
            }
        }
        catch ( const std::out_of_range& e ) {
            ShowExcept( e );
            return false;
        }
        n_col++;
    }
    wxStfChildFrame* pFrame = (wxStfChildFrame*)actDoc()->GetDocumentWindow();
    if ( !pFrame ) {
        ShowError( wxT("Pointer to frame is zero") );
        return false;
    }
    pFrame->ShowTable( pyTable, wxString( caption, wxConvLocal ) );
    return true;
}

bool set_marker(double x, double y) {
    if ( !check_doc() )
        return false;
    try {
        actDoc()->cur().SetPyMarker(stf::PyMarker(x,y));
    }
    catch (const std::out_of_range& e) {
        wxString msg( wxT("Could not set the marker:\n") );
        msg << wxString( e.what(), wxConvLocal );
        ShowError(msg);
        return false;
    }

    return refresh_graph();
}

bool erase_markers() {
    if ( !check_doc() )
        return false;

    actDoc()->cur().ErasePyMarkers();
    return refresh_graph();
}