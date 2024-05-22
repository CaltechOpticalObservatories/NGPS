package edu.jhu.htm.app;
/*	 Filename:	   echoDomain.java

	 Read a domain file and echo it to screen. Any special domain file
	 keywords are converted to the standard convex format.


	 Author:		 Peter Z. Kunszt

	 Date:		   October 15, 1999



 (c) Copyright The Johns Hopkins University 1999
 All Rights Reserved

 The software and information contained herein are proprietary to The
 Johns Hopkins University, Copyright 1999.  This software is furnished
 pursuant to a written license agreement and may be used, copied,
 transmitted, and stored only in accordance with the terms of such
 license and with the inclusion of the above copyright notice.  This
 software and information or any other copies thereof may not be
 provided or otherwise made available to any other person.

*/

import edu.jhu.htm.core.*;
import edu.jhu.htm.parsers.*;
import java.io.*;

/**

  This example code echoes a domain file. It converts special formats
  to the default one.

  It can be invoked by
  <pre>
  java edu.jhu.htm.app.echoDomain domainfile
  </pre>

  where
  <pre>
	 domainfile: file containing a domain
  </pre>

  <p>
  see the description in intersect for domainfile formats and examples.
  There are a couple of example domain files in the directory
  jhu/htm.app.

@see intersect
@author Peter Kunszt
@version 1.0
*/

public class echoDomain {
   public static void main (String[] argv) throws Exception{


	   //
	   // Initialization
	   //

	  String usage="Usage: echoDomain domainfile";

	  // initialize
	  if (argv.length < 1) {
	 throw new Exception(usage);
	  }

	  Domain domain = new Domain();
	  DomainParser parser = new LegacyParser();
	  String domainFile = argv[0];
	  domain = parser.parseFile(domainFile);
	  System.out.println(domain.toString());

   }
}
