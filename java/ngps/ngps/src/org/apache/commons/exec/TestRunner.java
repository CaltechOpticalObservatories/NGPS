/* 
 * Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package org.apache.commons.exec;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestResult;
import junit.framework.TestSuite;
import org.apache.commons.exec.environment.EnvironmentUtilTest;
import org.apache.commons.exec.util.MapUtilTest;
import org.apache.commons.exec.CommandLineTest;
import java.io.File;
import java.util.Map;
import java.util.HashMap;
import org.apache.commons.exec.environment.EnvironmentUtils;

/**
 * A stand-alone JUnit invocation to allow running JUnit tests without
 * having ANT or M2 installed.
 */
public class TestRunner extends TestCase {

    public static Test suite() {
        TestSuite suite = new TestSuite("TestRunner");
        suite.addTestSuite(CommandLineTest.class);
        suite.addTestSuite(DefaultExecutorTest.class);
        suite.addTestSuite(EnvironmentUtilTest.class);
        suite.addTestSuite(MapUtilTest.class);
        suite.addTestSuite(TestUtilTest.class);
        return suite;
    }

    public static void main(String[] args) {
        String line = "labview";
        Map env = new HashMap();
            env.put("DISPLAY=", "'farscape:16.0'");
        CommandLine commandLine = CommandLine.parse(line);
        commandLine.addArgument("/home/developer/GUIS/GUIS/TSPEC/SPEC/NIRGUI/NIRGUI_Main.vi");
        DefaultExecutor executor = new DefaultExecutor();


//        File labviewDirectory = new File("/home/developer/GUIS/GUIS/TSPEC/SPEC/NIRGUI/");
//        executor.setWorkingDirectory(labviewDirectory);
        try{
           Map      procEnvironment        = EnvironmentUtils.getProcEnvironment();
           HashMap  applicationEnvironment = new HashMap();
           int      exitValue              = executor.execute(commandLine,env);

        }catch(Exception e){
            System.out.println(e.toString());
        }
//        String line1 = "open";
//        CommandLine commandLine1 = CommandLine.parse(line1);
//        commandLine1.addArgument("X11.app");
//        commandLine1.addArgument("/Applications/National Instruments/LabVIEW 2010/LabVIEW.app");
//        DefaultExecutor executor1 = new DefaultExecutor();
//        File xtermDirectory = new File("/Applications/Utilities/");
//        executor.setWorkingDirectory(xtermDirectory);
//        try{
//           int exitValue = executor.execute(commandLine1);
//           executor.setWorkingDirectory(labviewDirectory);
//           exitValue = executor.execute(commandLine);
//        }catch(Exception e){
//            System.out.println(e.toString());
//        }



//        Test test = TestRunner.suite();
//        junit.textui.TestRunner testRunner = new junit.textui.TestRunner(System.out);
//        TestResult testResult = testRunner.doRun(test);

//        if(!testResult.wasSuccessful()) {
//            System.exit(1);
//        }

        // not calling System.exit(0) here to ensure that the application
        // properly terminates (e.g. not waiting for any background threads
        // indicating serious problems
        return;
    }
}
