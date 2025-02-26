/**
 * @file    fits.h
 * @brief   fits interface functions to CCFits
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 * template class for FITS I/O operations using CCFits.
 * This file includes the complete template class for FITS operations
 * using the CCFits library. If you're looking for the FITS keyword
 * database you're in the wrong place -- that's in common. This is just
 * FITS file operations.
 *
 */
#ifndef CAMERA_FITS_H
#define CAMERA_FITS_H

#include <CCfits/CCfits>
#include <fstream>         // for ofstream
#include <thread>
#include <atomic>
#include <string>
#include "common.h"
#include "build_date.h"
#include "utilities.h"
#include "logentry.h"

const int FITS_WRITE_WAIT = 5000;                   ///< approx time (in msec) to wait for a frame to be written

/***** FITS_file **************************************************************/
/**
 * @class FITS_file
 * @brief template class for FITS I/O operations using CCFits
 *
 */
class FITS_file {
  private:
    std::mutex fits_mutex;                          ///< used to block writing_file semaphore in multiple threads
    std::unique_ptr<CCfits::FITS> pFits;            ///< pointer to FITS data container
    std::atomic<bool> writing_file;                 ///< semaphore indicates file is being written
    std::atomic<bool> error;                        ///< indicates an error occured in a file writing thread
    std::atomic<bool> file_open;                    ///< semaphore indicates file is open
    std::atomic<int> threadcount;                   ///< keep track of number of write_image_thread threads
    std::atomic<int> framen;                        ///< internal frame counter for multi-extensions
    CCfits::ExtHDU* imageExt;                       ///< image extension header unit
    std::vector<CCfits::ExtHDU*> pExtension;        ///< image extension header unit
    std::string fits_name;
    enum class HDUTYPE { Primary, Extension };

  public:
    std::atomic<int> extension;
    bool iserror() { return this->error; };         ///< allows outsiders access to errors that occurred in a fits writing thread
    bool isopen()  { return this->file_open; };     ///< allows outsiders access file open status
    FITS_file() {                                   ///< constructor
      this->extension = 0;
      this->threadcount = 0;
      this->framen = 0;
      this->writing_file = false;
      this->error = false;
      this->file_open = false;
    };

    ~FITS_file() {                                  ///< deconstructor
    };


void foo ( HDUTYPE hdu ) { if ( hdu == HDUTYPE::Primary ); }

    /***** FITS_file::open_file ***********************************************/
    /**
     * @brief      opens a FITS file
     * @param[in]  writekeys  true to write keywords before exposure
     * @param[in]  info       reference to camera_info class
     * @return     ERROR or NO_ERROR
     *
     * This uses CCFits to create a FITS container, opens the file and writes
     * primary header data to it.
     *
     */
    long open_file( bool writekeys, Camera::Information & info ) {
this->foo(HDUTYPE::Primary);
      std::string function = "FITS_file::open_file";
      std::stringstream message;

      long axes[2];          // local variable of image axes size
      int num_axis;          // local variable for number of axes

      const std::lock_guard<std::mutex> lock(this->fits_mutex);

      // This is probably a programming error, if file_open is true here
      //
      if (this->file_open) {
        message.str(""); message << "ERROR: FITS file \"" << info.fits_name << "\" already open";
        logwrite(function, message.str());
        return (ERROR);
      }

      // Check that we can write the file, because CCFits will crash if it cannot
      //
      std::ofstream checkfile ( info.fits_name.c_str() );
      if ( checkfile.is_open() ) {
        checkfile.close();
        std::remove( info.fits_name.c_str() );
      }
      else {
        message.str(""); message << "ERROR unable to create file \"" << info.fits_name << "\"";
        logwrite(function, message.str());
        return(ERROR);
      }

      if (info.ismex) {      // special num_axis, axes for multi-extensions, no data associated with primary header
        num_axis = 0;
        axes[0] = 0;
        axes[1] = 0;
      }
      else {                 // or regular for flat fits files
        num_axis = 2;
        axes[0] = info.axes[0];
        axes[1] = info.axes[1];
      }

      if (!info.type_set) { // This is a programming error, means datatype is uninitialized.
        message.str(""); message << "ERROR: FITS datatype for " << info.fits_name << " is uninitialized. Call set_axes()";
        logwrite( function, message.str() );
        this->close_file( writekeys, info );
        return( ERROR );
      }

      try {
        // Create a new FITS object, specifying the data type and axes for the primary image.
        // Simultaneously create the corresponding file.
        //
        this->pFits.reset( new CCfits::FITS(info.fits_name, info.datatype, num_axis, axes) );
        this->file_open = true;    // file is open now
        this->make_camera_header(info);

        // Iterate through the system-defined FITS keyword databases and add them to the primary header.
        //
        logwrite( function, "writing systemkeys.primary() keys before exposure" );
        for ( auto const &keydb : info.systemkeys.primary().keydb ) {
//        message.str(""); message << "[DEBUG]: adding info.systemkeys.primary \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
          this->add_key( HDUTYPE::Primary, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
        }
        logwrite( function, "writing telemkeys.primary() keys before exposure" );
        for ( auto const &keydb : info.telemkeys.primary().keydb ) {
//        message.str(""); message << "[DEBUG]: adding info.telemkeys.primary \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
          this->add_key( HDUTYPE::Primary, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
        }
        logwrite( function, "writing userkeys.primary() keys before exposure" );
        for ( auto const &keydb : info.userkeys.primary().keydb ) {
//        message.str(""); message << "[DEBUG]: adding info.userkeys.primary \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
          this->add_key( HDUTYPE::Primary, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
        }

/***
        // If specified, iterate through the user-defined FITS keyword databases and add them to the primary header.
        //
        if ( writekeys ) {
          logwrite( function, "writing user-defined keys before exposure" );
          for ( auto const &keydb : info.userkeys.keydb ) {
            message.str(""); message << "[DEBUG]: adding info.userkeys \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
            this->add_key( true, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
          }
        }
***/

        // create empty extensions
        //
//      pExtension[i] = this->pFits->addImage(extname[i], info.datatype, axes);
      }
      catch (CCfits::FITS::CantCreate){
        message.str(""); message << "ERROR: unable to open FITS file \"" << info.fits_name << "\"";
        logwrite(function, message.str());
        return(ERROR);
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR opening FITS file \"" << info.fits_name << "\": " << e.what();
        logwrite(function, message.str());
        return(ERROR);
      }

      message.str(""); message << "opened file \"" << info.fits_name << "\" for FITS write";
      logwrite(function, message.str());

      // must reset variables as when container was constructed
      // each time a new file is opened
      //
      this->threadcount = 0;
      this->framen = 0;
      this->writing_file = false;
      this->error = false;

      this->fits_name = info.fits_name;

      return (0);
    }
    /***** FITS_file::open_file ***********************************************/


    /***** FITS_file::close_file **********************************************/
    /**
     * @brief      closes fits file
     * @param[in]  writekeys  true to write keywords after exposure
     * @param[in]  info       reference to camera_info class
     *
     * Before closing the file, DATE and CHECKSUM keywords are added.
     * Nothing called returns anything so this doesn't return anything.
     *
     */
    void close_file( bool writekeys, Camera::Information & info ) {
      std::string function = "FITS_file::close_file";
      std::stringstream message;

      // Nothing to do if not open
      //
      if ( ! this->file_open ) {
#ifdef LOGLEVEL_DEBUG
        logwrite(function, "[DEBUG] no open FITS file to close");
#endif
        return;
      }

      // try to get the mutex
      //
      std::unique_lock<std::mutex> lock( this->fits_mutex, std::defer_lock );

      if ( !lock.try_lock() ) {
        message.str(""); message << "another thread is already closing " << info.fits_name;
        logwrite( function, message.str() );
        return;
      }

      // If aborted then close and remove the file
      //
      if ( info.abortexposure ) {
        try { this->pFits->destroy(); }
        catch ( CCfits::FitsError& error ) {
          message.str(""); message << "ERROR closing aborted file: " << error.message();
          logwrite( function, message.str() );
          this->file_open = false;   // must set this false on exception
          return;
        }
        this->file_open = false;
        std::remove( info.fits_name.c_str() );
        logwrite( function, "removed aborted file" );
        this->fits_name="";
        return;
      }

      try {
        // Iterate through the system-defined FITS keyword databases and add them to the primary header.
        //
        logwrite( function, "writing systemkeys.primary() keys after exposure" );
        for ( auto const &keydb : info.systemkeys.primary().keydb ) {
//        message.str(""); message << "[DEBUG]: adding info.systemkeys.primary \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
          this->add_key( HDUTYPE::Primary, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
        }
        logwrite( function, "writing telemkeys.primary() keys after exposure" );
        for ( auto const &keydb : info.telemkeys.primary().keydb ) {
//        message.str(""); message << "[DEBUG]: adding info.telemkeys.primary \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
          this->add_key( HDUTYPE::Primary, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
        }

/****
        // Write the user keys on close, if specified
        //
        logwrite( function, "writing user-defined keys after exposure" );
        for ( auto const &keydb : info.userkeys.keydb ) {
          message.str(""); message << "[DEBUG]: adding info.userkeys \"" << keydb.second.keyword << "\""; logwrite(function, message.str());
          this->add_key( true, keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
        }
****/

        // Add a header keyword for the time the file was written (right now!)
        //
        this->pFits->pHDU().addKey("DATE", get_timestamp(), "FITS file write time");

        // Write the checksum
        //
        this->pFits->pHDU().writeChecksum();

        // Deallocate the CCfits object and close the FITS file
        //
        this->pFits->destroy();
      }
      catch (CCfits::FitsError& error){
        message.str(""); message << "ERROR writing checksum and closing file: " << error.message();
        logwrite(function, message.str());
        this->file_open = false;   // must set this false on exception
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR closing file: " << e.what();
        logwrite(function, message.str());
        this->file_open = false;   // must set this false on exception
      }

      // Let the world know that the file is closed
      //
      this->file_open = false;

      message.str(""); message << this->fits_name << " closed";
      logwrite(function, message.str());
      this->fits_name="";
      return;
    }
    /***** FITS_file::close_file **********************************************/


    /***** FITS_file::write_image *********************************************/
    /**
     * @brief      spawn threads to write image data to FITS file on disk
     * @param[in]  data  pointer to the data using template type T
     * @param[in]  into  reference to the fits_info class
     * @return     ERROR or NO_ERROR
     *
     * This function spawns a thread to write the image data to disk
     *
     */
    template <class T>
    long write_image(T* data, Camera::Information& info, const std::string extname="" ) {
      std::string function = "FITS::write_image";
      std::stringstream message;

      // The file must have been opened first
      //
      if ( !this->file_open ) {
        message.str(""); message << "ERROR: FITS file \"" << this->fits_name << "\" not open";
        logwrite(function, message.str());
        return (ERROR);
      }

      // copy the data into an array of the appropriate type  //TODO use multiple threads for this??
      //
      std::valarray<T> array( info.section_size );
      for ( long i = 0; i < info.section_size; i++ ) {
        array[i] = data[i];
      }

      // Use a lambda expression to properly spawn a thread without having to
      // make it static. Each thread gets a pointer to the current object this->
      // which must continue to exist until all of the threads terminate.
      // That is ensured by keeping threadcount, incremented for each thread
      // spawned and decremented on return, and not returning from this function
      // until threadcount is zero.
      //
      this->threadcount++;                                   // increment threadcount for each thread spawned

#ifdef LOGLEVEL_DEBUG
      long num_axis = ( info.cubedepth > 1 ? 3 : 2 );
      long axes[num_axis];
      for ( int i=0; i < num_axis; i++ ) axes[i] = info.axes[i];
      message.str("");
      message << "[DEBUG] threadcount=" << this->threadcount << " ismex=" << info.ismex << " section_size=" << info.section_size 
              << " cubedepth=" << info.cubedepth
              << " axes=";
      for ( auto aa : axes ) message << aa << " ";
      message << ". spawning image writing thread for frame " << this->framen << " of file \"" << this->fits_name << "\"";
      logwrite(function, message.str());
#endif
      std::thread([&]() {                                    // create the detached thread here
        if (info.ismex) {
          this->write_mex_thread(array, info, this, extname);
        }
        else {
          this->write_image_thread(array, info, this);
        }
        std::lock_guard<std::mutex> lock(this->fits_mutex);  // lock and
        this->threadcount--;                                 // decrement threadcount
      }).detach();
#ifdef LOGLEVEL_DEBUG
      message.str("");
      message << "[DEBUG] spawned image writing thread for frame " << this->framen << " of file \"" << this->fits_name << "\"";
      logwrite(function, message.str());
#endif

      // wait for all threads to complete
      //
      int last_threadcount = this->threadcount;
      int wait = FITS_WRITE_WAIT;
      while (this->threadcount > 0) {
        usleep(1000);
        if (this->threadcount >= last_threadcount) {         // threads are not completing
          wait--;                                            // start decrementing wait timer
        }
        else {                                               // a thread was completed so things are still working
          last_threadcount = this->threadcount;              // reset last threadcount
          wait = FITS_WRITE_WAIT;                            // reset wait timer
        }
        if (wait < 0) {
          message.str(""); message << "ERROR: timeout waiting for threads."
                                   << " threadcount=" << threadcount 
                                   << " extension=" << info.extension 
                                   << " framen=" << this->framen
                                   << " file=" << this->fits_name;
          logwrite(function, message.str());
          this->writing_file = false;
          return (ERROR);
        }
      }

      if (this->error) {
        message.str("");
        message << "an error occured in one of the FITS writing threads for file \"" << this->fits_name << "\"";
        logwrite(function, message.str());
      }
#ifdef LOGLEVEL_DEBUG
      else {
        message.str("");
        message << "[DEBUG] file \"" << this->fits_name << "\" complete";
        logwrite(function, message.str());
      }
#endif

      return ( this->error ? ERROR : NO_ERROR );
    }
    /***** FITS_file::write_image *********************************************/


    /***** FITS_file::write_image_thread **************************************/
    /**
     * @brief      This is where the data are actually written for flat fits files
     * @param[in]  data  reference to the data
     * @param[in]  info  reference to the camera info structure
     * @param[in]  self  pointer to this-> FITS_file object
     *
     * This is the worker thread, to write the data using CCFits,
     * and must be spawned by write_image.
     *
     */
    template <class T>
    void write_image_thread(std::valarray<T> &data, Camera::Information &info, FITS_file *self) {
      std::string function = "FITS_file::write_image_thread";
      std::stringstream message;

      // This shouldn't happen (unless there's a programming error) but an
      // attempt to call pHDU().write(...) with zero size can cause a fatal 
      // exception (i.e. crash) which doesn't get caught.
      //
      if ( info.section_size == 0 ) {
        message << "ERROR: section_size=0 for file \"" << self->fits_name << "\"";
        logwrite( function, message.str() );
        self->writing_file = false;
        self->error = true;      // tells the calling function that I had an error
        return;
      }

      // This makes the thread wait while another thread is writing images. This
      // function is really for single image writing, it's here just in case.
      //
      int wait = FITS_WRITE_WAIT;
      while (self->writing_file) {
        usleep(1000);
        if (--wait < 0) {
          message.str(""); message << "ERROR: timeout waiting for last frame to complete. "
                                   << "unable to write " << self->fits_name;
          logwrite(function, message.str());
          self->writing_file = false;
          self->error = true;    // tells the calling function that I had an error
          return;
        }
      }

      // Set the FITS system to verbose mode so it writes error messages
      //
      CCfits::FITS::setVerboseMode(true);

      // Lock the mutex and set the semaphore for file writing
      //
      const std::lock_guard<std::mutex> lock(self->fits_mutex);
      self->writing_file = true;

      // write the primary image into the FITS file
      //
      try {
        long fpixel(1);        // start with the first pixel always
        self->pFits->pHDU().write( fpixel, info.section_size, data );
        self->pFits->flush();  // make sure the image is written to disk
      }
      catch (CCfits::FitsError& error){
        message.str(""); message << "ERROR: writing " << info.section_size << " pixels to \"" << self->fits_name << "\": " << error.message();
        logwrite(function, message.str());
        self->writing_file = false;
        self->error = true;    // tells the calling function that I had an error
        return;
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR: writing " << info.section_size << " pixels to \"" << self->fits_name << "\": " << e.what();
        logwrite(function, message.str());
        self->writing_file = false;
        self->error = true;    // tells the calling function that I had an error
        return;
      }

      self->writing_file = false;
      return;
    }
    /***** FITS_file::write_image_thread **************************************/


    /***** FITS_file::write_mex_thread ****************************************/
    /**
     * @brief      This is where the data are actually written for multi-extensions
     * @param[in]  data  reference to the data
     * @param[in]  info  reference to the Camera::Information camera_info structure
     * @param[in]  self  pointer to this-> FITS_file object
     *
     * This is the worker thread, to write the data using CCFits,
     * and must be spawned by write_image.
     *
     */
    template <class T>
    void write_mex_thread(std::valarray<T> &data, Camera::Information &info, FITS_file *self, const std::string extname) {
      std::string function = "FITS_file::write_mex_thread";
      std::stringstream message;

#ifdef LOGLEVEL_DEBUG
///   message.str(""); message << "[DEBUG] info.extension=" << info.extension << " this->extension=" << this->extension << " this->framen=" << this->framen;
///   logwrite( function, message.str() );
#endif

      // This makes the thread wait while another thread is writing images. This
      // thread will only start writing once the extension number matches the
      // number of frames already written.
      //
      int last_threadcount = this->threadcount;
      int wait = FITS_WRITE_WAIT;
      while (this->extension != this->framen) {
        usleep(1000);
        if (this->threadcount >= last_threadcount) {  // threads are not completing
          wait--;                                     // start decrementing wait timer
        }
        else {                                        // a thread was completed so things are still working
          last_threadcount = this->threadcount;       // reset last threadcount
          wait = FITS_WRITE_WAIT;                     // reset wait timer
        }
        if (wait < 0) {
          message.str(""); message << "ERROR: timeout waiting for frame write."
                                   << " threadcount=" << threadcount 
                                   << " info.extension=" << info.extension 
                                   << " this->extension=" << this->extension 
                                   << " framen=" << this->framen;
          logwrite(function, message.str());
          self->writing_file = false;
          self->error = true;    // tells the calling function that I had an error
          return;
        }
      }

      // Set the FITS system to verbose mode so it writes error messages
      //
      CCfits::FITS::setVerboseMode(true);

      // Lock the mutex and set the semaphore for file writing
      //
      const std::lock_guard<std::mutex> lock(self->fits_mutex);
      self->writing_file = true;

      // write the primary image into the FITS file
      // An exception here is fatal, get out.
      //
      long fpixel(1);                // start with the first pixel always
      try {

        long num_axis = ( info.fitscubed > 1 ? 3 : 2 );

        std::vector<long> axes(2);   // addImage() wants a vector, which has the size of the number of axes

        for ( int i=0; i < num_axis; i++ ) axes[i]=info.axes[i];

        message.str(""); message << "adding " << axes[0] << " x " << axes[1] 
                                 << " frame to extension " << this->extension+1 << " (" << extname << ") in file " << self->fits_name;
        logwrite(function, message.str());

        // Add the extension here
        //
        this->imageExt = self->pFits->addImage(extname, info.datatype, axes);

        // Add extension-only keys now
        //
        if (info.datatype == SHORT_IMG) {
          this->imageExt->addKey("BZERO", 32768, "offset for signed short int");
          this->imageExt->addKey("BSCALE", 1, "scaling factor");
        }
      }
      catch ( const CCfits::FitsError& error ) {
        message.str(""); message << "ERROR adding extension " << extname << " to " << self->fits_name << ": " << error.message();
        logwrite(function, message.str());
        self->writing_file = false;
        self->error = true;    // tells the calling function that I had an error
        return;
      }

      // Write header keywords.
      // An exception here is not good, but isn't necessarily fatal
      // and doesn't cause an immediate return;
      //
      try {
        // convenient local variable for all elmokeys
        //
        auto elmokeys = info.systemkeys.elmomap();

        // If this extension name is in the elmokeys map, then add them to this extension
        //
        if ( elmokeys.find(extname) != elmokeys.end() ) {
          // loop through keydb for the extension keys for this channel
          for ( const auto &_db : elmokeys.at(extname).keydb ) {
            this->add_key( HDUTYPE::Extension, _db.second.keyword, _db.second.keytype, _db.second.keyvalue, _db.second.keycomment );
          }
        }

        // Add any keys designated for "all" extensions
        //
        if ( elmokeys.find("all") != elmokeys.end() ) {
          // loop through keydb for the extension keys for this channel
          for ( const auto &_db : elmokeys.at("all").keydb ) {
            this->add_key( HDUTYPE::Extension, _db.second.keyword, _db.second.keytype, _db.second.keyvalue, _db.second.keycomment );
          }
        }

        // If this extension name is in the elmokeys map, then add them to this extension
        //
        if ( info.userkeys.elmomap().find(extname) != info.userkeys.elmomap().end() ) {
          // loop through keydb for the extension keys for this channel
          for ( const auto &_db : info.userkeys.elmomap().at(extname).keydb ) {
            this->add_key( HDUTYPE::Extension, _db.second.keyword, _db.second.keytype, _db.second.keyvalue, _db.second.keycomment );
          }
        }

        // Add any keys designated for "all" extensions
        //
        if ( info.userkeys.elmomap().find("all") != info.userkeys.elmomap().end() ) {
          // loop through keydb for the extension keys for this channel
          for ( const auto &_db : info.userkeys.elmomap().at("all").keydb ) {
            this->add_key( HDUTYPE::Extension, _db.second.keyword, _db.second.keytype, _db.second.keyvalue, _db.second.keycomment );
          }
        }

        // If this extension name is in the elmokeys map, then add them to this extension
        //
        if ( info.telemkeys.elmomap().find(extname) != info.telemkeys.elmomap().end() ) {
          // loop through keydb for the extension keys for this channel
          for ( const auto &_db : info.telemkeys.elmomap().at(extname).keydb ) {
            this->add_key( HDUTYPE::Extension, _db.second.keyword, _db.second.keytype, _db.second.keyvalue, _db.second.keycomment );
          }
        }

        // Add any keys designated for "all" extensions
        //
        if ( info.telemkeys.elmomap().find("all") != info.telemkeys.elmomap().end() ) {
          // loop through keydb for the extension keys for this channel
          for ( const auto &_db : info.telemkeys.elmomap().at("all").keydb ) {
            this->add_key( HDUTYPE::Extension, _db.second.keyword, _db.second.keytype, _db.second.keyvalue, _db.second.keycomment );
          }
        }

        // Add AMPSEC keys
        //
        if ( info.amp_section.size() == 4 ) {
          int x1 = info.amp_section.at( info.extension ).at( 0 );
          int x2 = info.amp_section.at( info.extension ).at( 1 );
          int y1 = info.amp_section.at( info.extension ).at( 2 );
          int y2 = info.amp_section.at( info.extension ).at( 3 );

          message.str(""); message << "[" << x1 << ":" << x2 << "," << y1 << ":" << y2 << "]";
          this->imageExt->addKey( "AMPSEC", message.str(), "amplifier section" );
        }
      }
      catch ( const CCfits::FitsError &error ) {
        message.str(""); message << "ERROR from CCfits writing keys: " << error.message();
        logwrite(function, message.str());
        self->error = true;    // tells the calling function that I had an error
      }
      catch( const std::exception &e ) {
        message.str(""); message << "ERROR exception writing keys: " << e.what();
        logwrite( function, message.str() );
        self->error = true;    // tells the calling function that I had an error
      }

      try {
        // Write and flush to make sure image is written to disk
        //
        this->imageExt->write( fpixel, info.section_size, data );
        self->pFits->flush();
      }
      catch ( const CCfits::FitsError &error ) {
        message.str(""); message << "ERROR CCfits exception finalizing " << info.fits_name << ": " << error.message();
        logwrite(function, message.str());
        self->error = true;    // tells the calling function that I had an error
      }
      catch(...) {
        message.str(""); message << "ERROR unknown exception finalizing " << info.fits_name;
        logwrite( function, message.str() );
        self->error = true;    // tells the calling function that I had an error
      }

      // increment number of frames written
      //
      this->framen++;
      self->writing_file = false;
      return;
    }
    /***** FITS_file::write_mex_thread ****************************************/


    /***** FITS_file::make_camera_header **************************************/
    /**
     * @brief      this writes header info from the camera_info class
     * @param[in]  info  reference to the camera_info class
     * @todo       is this function obsolete?
     *
     * Uses CCFits
     *
     * TODO is this function obsolete?
     *
     */
    void make_camera_header(Camera::Information &info) {
      try {
        // To put just the filename into the header (and not the path), find the last slash
        // and substring from there to the end.
        //
      }
      catch (CCfits::FitsError & err) {
        logwrite( "FITS_file::make_camera_header", "ERROR: "+std::string(err.message()) );
      }
      return;
    }
    /***** FITS_file::make_camera_header **************************************/


    /***** FITS_file::add_key *************************************************/
    /**
     * @brief      wrapper to write keywords to the FITS file header
     * @details    This can throw an exception so it should be caught.
     * @param[in]  hdu      HDUTYPE enum specified Primary or Extension
     * @param[in]  keyword
     * @param[in]  type
     * @param[in]  value
     * @param[in]  comment
     *
     * Uses CCFits addKey template function, this wrapper ensures the correct type is passed.
     *
     */
    void safe_add_key( HDUTYPE hdu, std::string keyword, std::string type, std::string value, std::string comment ) {
      const std::lock_guard<std::mutex> lock(this->fits_mutex);
      this->add_key( hdu, keyword, type, value, comment );
      return;
    }
    void add_key( HDUTYPE hdu, std::string keyword, std::string type, std::string value, std::string comment ) {
      std::string function = "FITS_file::add_key";
      std::stringstream message;

#ifdef LOGLEVEL_DEBUG
///   message << "[DEBUG] keyword=" << keyword << " type=" << type << " value=" << value << " comment=" << comment
///           << " HDUTYPE=" << (hdu == HDUTYPE::Primary?"pri":"ext");
///   logwrite( function, message.str() );
#endif

      // The file must have been opened first
      //
      if ( !this->file_open ) {
        message.str(""); message << "ERROR: adding key " << keyword << "=" << value << ": no FITS file open";
        logwrite( function, message.str() );
        message.str(""); message << function << ": adding key " << keyword << "=" << value << ": no FITS file open";
        throw std::runtime_error( message.str() );
      }

      try {
        if (type.compare("BOOL") == 0) {
          bool boolvalue = ( value == "T" ? true : false );
          ( hdu == HDUTYPE::Primary ? this->pFits->pHDU().addKey( keyword, boolvalue, comment )
                                    : this->imageExt->addKey( keyword, boolvalue, comment ) );
        }
        else if (type.compare("INT") == 0) {
          ( hdu == HDUTYPE::Primary ? this->pFits->pHDU().addKey(keyword, std::stoi(value), comment)
                                    : this->imageExt->addKey(keyword, std::stoi(value), comment) );
        }
        else if (type.compare("LONG") == 0) {
          ( hdu == HDUTYPE::Primary ? this->pFits->pHDU().addKey(keyword, std::stol(value), comment)
                                    : this->imageExt->addKey(keyword, std::stol(value), comment) );
        }
        else if (type.compare("FLOAT") == 0) {
          ( hdu == HDUTYPE::Primary ? this->pFits->pHDU().addKey(keyword, std::stof(value), comment)
                                    : this->imageExt->addKey(keyword, std::stof(value), comment) );
        }
        else if (type.compare("DOUBLE") == 0) {
          ( hdu == HDUTYPE::Primary ? this->pFits->pHDU().addKey(keyword, std::stod(value), comment)
                                    : this->imageExt->addKey(keyword, std::stod(value), comment) );
        }
        else if (type.compare("STRING") == 0) {
          ( hdu == HDUTYPE::Primary ? this->pFits->pHDU().addKey(keyword, value, comment)
                                    : this->imageExt->addKey(keyword, value, comment) );
        }
        else {
          message.str(""); message << "ERROR unknown type: " << type << " for user keyword: " << keyword << "=" << value
                                   << ": expected {INT,LONG,DOUBLE,FLOAT,STRING,BOOL}";
          logwrite(function, message.str());
        }
      }
      // There could be an error converting a value to INT or DOUBLE with stoi or stof
      //
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR parsing value " << value << " for keyword " << keyword << ": " << e.what();
        logwrite( function, message.str() );
        message.str(""); message << function << ": parsing value " << value << " for keyword " << keyword << ": " << e.what();
        throw std::runtime_error( message.str() );
      }
      catch (CCfits::FitsError & err) {
        message.str(""); message << "ERROR adding key " << keyword << "=" << value << " / " << comment << " (" << type << ")"
                                 << " to " << ( hdu == HDUTYPE::Primary ? "primary" : "extension" ) << " :"
                                 << err.message();
        logwrite(function, message.str());
        message.str(""); message << function << ": adding " << keyword << "=" << value << " to "
                                 << ( hdu == HDUTYPE::Primary ? "primary" : "extension" ) << " :" << err.message();
        throw std::runtime_error( message.str() );
      }
      return;
    }
    /***** FITS_file::add_key *************************************************/

};
/***** FITS_file **************************************************************/
#endif
