package edu.caltech.palomar.util.general; 
//=== File Prolog =============================================================
//	This code was developed by UCLA, Department of Physics and Astronomy,
//	for the Stratospheric Observatory for Infrared Astronomy (SOFIA) project.
//
//--- Contents ----------------------------------------------------------------
//	DataManifestXML  - The DataManifestXML class is used to
//                         create and manipulate SOFIA DCS Data Manifest Files
//--- Description -------------------------------------------------------------
//
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//
//      07/10/02        Li Sun
//        Original Implementation
//
//--- DISCLAIMER---------------------------------------------------------------
//
//	This software is provided "as is" without any warranty of any kind, either
//	express, implied, or statutory, including, but not limited to, any
//	warranty that the software will conform to specification, any implied
//	warranties of merchantability, fitness for a particular purpose, and
//	freedom from infringement, and any warranty that the documentation will
//	conform to the program, or any warranty that the software will be error
//	free.
//
//	In no event shall UCLA be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
import java.util.*;
import java.io.*;
import java.text.*;
/*=============================================================================================
/   OutputFile
/=============================================================================================*/
public final class OutputFile
{
   public long       max_filesize;
   public int        max_filenumber;
   private BufferedOutputStream	  _ostream = null;            // the stream which the temp write to
   private File			  _file = null;
   private File 		  _dir = null;
   private String		  _baseFileName = null;
   private String		  _fullFileName = null;
   private String		  _basePath = null;
   private String		  _fullPath = null;
   private String		  _fullPathAndFile = null;

   private Object                 _lock  = new Object();      // Synchronize Lock
/*=============================================================================================
/         OutputFile(String directoryPath,String baseFileName, long size, int num)
/=============================================================================================*/
   public OutputFile(String directoryPath,String baseFileName, long size, int num)
   {
     _baseFileName = baseFileName;
     _basePath = directoryPath;
     setMaxFileSize(size);
     setFileNumber(num);
     OpenFile();
   }
/*=============================================================================================
/   setMaxFileSize(long size)
/=============================================================================================*/
  public void setMaxFileSize(long size)
   {
     this.max_filesize = size;
   }
/*=============================================================================================
/    getMaxFileSize()
/=============================================================================================*/
   public long getMaxFileSize()
   {
     return this.max_filesize;
   }
/*=============================================================================================
/  setFileNumber(int num)
/=============================================================================================*/
   public void setFileNumber(int num)
   {
     this.max_filenumber = num;
   }
/*=============================================================================================
/   getFileNumber()
/=============================================================================================*/
   public int getFileNumber()
   {
     return this.max_filenumber;
   }

   public boolean PrintToFile(String msg)
   {

     synchronized(_lock)
     {
       //if the file size is larger then MAX_FILESIZE, then renmaing it to a new file
       if (_file.length() > this.getMaxFileSize())
       {
         if ( SwitchFiles() != true )
         {
           return false;
         }
       }
     }

     print(_ostream, msg + "\n" );

     return true;
   }

/*=============================================================================================
/       print(BufferedOutputStream os, String msg)
/=============================================================================================*/
 protected void print(BufferedOutputStream os, String msg)
   {
    try
    {
      if ( ( msg == null) || ( os == null ) )
      {
        return;
      }
      else
      {
        os.write( msg.getBytes() );
        os.flush();
      }
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
   } // end print

/*=============================================================================================
/        boolean OpenFile()
/=============================================================================================*/
  private boolean OpenFile()
   {
       _fullPath = _basePath;

      try
      {
        // Create a File object for directory expressed the _fullPath
        _dir = new File(_fullPath);

        // Check if the directory already exists
        if ( _dir.isDirectory() == true )
        {
           ;
        }
        else
        {
          // no directory, then create one
	 if ( _dir.mkdirs() != true )
         {
	   return false;
	 }
        }

        File[] existingFiles = _dir.listFiles(new FilenameFilter(){
                               public boolean accept(File dir,String name)
                               {
                                  if (name.indexOf(_baseFileName) == 0)
                                  {
                                    return true;
                                  }

                                  else
                                    return false;
                               }});

        if (existingFiles.length > this.getFileNumber())
        {
          // keep files number less or equal MAX_FILENUM
          for (int i = 0; i <= existingFiles.length - this.getFileNumber(); i++)
          {
            existingFiles[i].delete();
          }
        }

        long currentTime = System.currentTimeMillis();
        java.lang.Long currentTimeLong = Long.valueOf(currentTime);
        String currentTimeString = currentTimeLong.toString();
        int timeLength = currentTimeString.length();
        String timeTag = currentTimeString.substring((timeLength - 8),timeLength);

        _fullFileName    = _baseFileName + "_" + timeTag;

        _fullPathAndFile = _fullPath + System.getProperty("file.separator") + _fullFileName;

        // open the file and ostream
        _ostream = new BufferedOutputStream(new FileOutputStream(_fullPathAndFile, true), 8196);

        // create the file
        _file = new File(_fullPathAndFile);

        if (_file.canWrite() != true)
        {
         return false;
        }

     }
     catch (IOException e)
     {
       return false;
     }

     return true;
   }
/*=============================================================================================
/  void CloseLogFile()
/=============================================================================================*/
   public void CloseLogFile()
   {
      this.CloseFile(false);
   }
/*=============================================================================================
/    boolean CloseFile(boolean isSwitch)
/=============================================================================================*/
   private boolean CloseFile(boolean isSwitch)
   {
    if (_ostream != null)
    {
      // close output stream
      try
      {
        _ostream.flush();
        _ostream.close();

        // If switch requested, renaming the file
        if(isSwitch)
        {
          long currentTime = System.currentTimeMillis();
          java.lang.Long currentTimeLong = Long.valueOf(currentTime);
          String currentTimeString = currentTimeLong.toString();
          int timeLength = currentTimeString.length();
          String timeTag = currentTimeString.substring((timeLength - 4),timeLength);

          String newFileName = _baseFileName + "_" + timeTag;

          File newFile = new File(newFileName);

          _file.renameTo(newFile);
        }
      }
      catch (IOException e)
      {
        return false;
      }
      finally
      {
        _ostream = null;
        _dir     = null;
        _file    = null;

      }
    }
    return true;
  }

/*=============================================================================================
/   boolean SwitchFiles()
/=============================================================================================*/
   public boolean SwitchFiles()
   {
    // Close the current file
    if ( CloseFile(true) != true )
    {
      return false;
    }

    // Open a new File
    if ( OpenFile() != true )
    {
      return false;
    }

   return true;
  }

}
