package edu.caltech.palomar.instruments.gui.ds9;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.NoSuchElementException;
import java.util.Vector; 

public class DS9Accessor
{
	Runtime runtime = null;

	public DS9Accessor()
	{
		runtime = Runtime.getRuntime();
	}

	public boolean isDS9Ready( boolean showErrMsg ) throws IOException
	{
		Vector<String> vRetVals = doCmd( "xpaaccess ds9" );
		boolean bIsReady = false;

		try
		{
			String yesNo = vRetVals.firstElement();

			if ( yesNo.indexOf( "yes" ) >= 0 )
			{
				bIsReady = true;
			}
		}
		catch( NoSuchElementException nsee ) {}

		if ( !bIsReady && showErrMsg )
			System.err.println( "DS9 may not be running. Please start the program." );

		return bIsReady;
	}

	public void clearAllFrames() 
	{
		try
		{
			doCmd( "xpaset -p ds9 frame clear all" );
		}
		catch ( IOException ioe ) {}
	}

	public void showFits( String file ) throws IOException
	{
		Vector<String> vRetVals =
				doCmd( "xpaset -p ds9 file "+ file );

//		printOutput( vRetVals );
	}
        public void showRegions(String file) throws IOException{
 		Vector<String> vRetVals = doCmd( "xpaset -p ds9 regions load "+ file );           
        }

	public Vector<String> doCmd( String ds9cmd ) throws IOException
	{
		Process proc = runtime.exec( ds9cmd );
		int dFailCount = 0;

		// Put a BufferedReader on the proc output
		InputStream inputstream = proc.getInputStream();
		InputStreamReader inputstreamreader = new InputStreamReader( inputstream );
		BufferedReader bufferedreader = new BufferedReader(inputstreamreader);
    
		// Read the proc output
		Vector<String> vRetVals = new Vector<String>();
		String line;

		while ( ( line = bufferedreader.readLine() ) != null )
		{
			vRetVals.add( line );

			dFailCount++;
			if ( ( dFailCount++ ) > 1000 )
			{
				System.err.println( "Failed to read DS9 return values!" );
				break;
			}
		}

		// Check for proc failure
		try
		{
			if ( proc.waitFor() != 0 )
			{
				vRetVals.add( "exit value = " + proc.exitValue() );
			}
		}
		catch ( InterruptedException e )
		{
			System.err.println( e );
		}

		return vRetVals;
	}

	private void printOutput( Vector<String> outputVector )
	{
		if ( outputVector != null )
		{
			for ( int i=0; i<outputVector.size(); i++ )
			{
				System.out.println( "( DS9Accessor ) -> vec[ " + i + " ]: "
									+ outputVector.get( i ) );
			}
		}
	}
}
