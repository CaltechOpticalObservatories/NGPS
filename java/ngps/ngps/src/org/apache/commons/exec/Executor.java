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
 *
 */

package org.apache.commons.exec;

import java.io.File;
import java.io.IOException;
import java.util.Map;

/**
 * The main abstraction to start an external process.
 *
 * The interface allows to
 * <ul>
 *  <li>set a current working directory for the subprocess</li>
 *  <li>provide a set of environment variables passed to the subprocess</li>
 *  <li>capture the subprocess output of stdout and stderr using an ExecuteStreamHandler</li>
 *  <li>kill long-running processes using an ExecuteWatchdog</li>
 *  <li>define a set of expected exit values</li>
 *  <li>terminate any started processes when the main process is terminating using a ProcessDestroyer</li>
 * </ul>
 *
 * The following example shows the basic usage:
 *
 * <pre>
 * Executor exec = new DefaultExecutor();
 * CommandLine cl = new CommandLine("ls -l");
 * int exitvalue = exec.execute(cl);
 * </pre>
 */

public interface Executor {

    /** Invalid exit code. */
    int INVALID_EXITVALUE = 0xdeadbeef;

    /**
     * Define the exit code of the process to considered
     * successful.
     *
     * @param value the exit code representing successful execution
     */
    void setExitValue(final int value);

    /**
     * Define the exit code of the process to considered
     * successful. The caller can pass one of the following values
     * <ul>
     *  <li>an array of exit values to be considered successful</li>
     *  <li>an empty array for auto-detect of successful exit codes</li>
     *  <li>null to indicate to skip checking of exit codes</li>
     * </ul>
     *
     * @param values a list of the exit codes
     */
    void setExitValues(final int[] values);

    /**
     * Checks whether <code>exitValue</code> signals a failure. If no
     * exit values are set than the default conventions of the OS is
     * used. e.g. most OS regard an exit code of '0' as successful
     * execution and everything else as failure.
     *
     * @param exitValue the exit value (return code) to be checked
     * @return <code>true</code> if <code>exitValue</code> signals a failure
     */
    boolean isFailure(final int exitValue);

    /**
     * Get the StreamHandler used for providing input and
     * retriving the output.
     * 
     * @return the StreamHandler 
     */
    ExecuteStreamHandler getStreamHandler();

    /**
     * Set the StreamHandler used for providing input and
     * retriving the output.
     *
     * @param streamHandler the StreamHandler
     */

    void setStreamHandler(ExecuteStreamHandler streamHandler);

    /**
     * Get the watchdog used to kill of processes running,
     * typically, too long time.
     *
     * @return the watchdog
     */
    ExecuteWatchdog getWatchdog();

    /**
     * Set the watchdog used to kill of processes running, 
     * typically, too long time.
     *
     * @param watchDog the watchdog
     */
    void setWatchdog(ExecuteWatchdog watchDog);

    /**
     * Set the handler for cleanup of started processes if the main process
     * is going to terminate.
     *
     * @return the ProcessDestroyer
     */
    ProcessDestroyer getProcessDestroyer();

    /**
     * Get the handler for cleanup of started processes if the main process
     * is going to terminate.
     *
     * @param processDestroyer the ProcessDestroyer
     */
    void setProcessDestroyer(ProcessDestroyer processDestroyer);

    /**
     * Get the working directory of the created process.
     *
     * @return the working directory
     */
    File getWorkingDirectory();

    /**
     * Set the working directory of the created process. The
     * working directory must exist when you start the process.
     *
     * @param dir the working directory
     */
    void setWorkingDirectory(File dir);

    /**
     * Methods for starting synchronous execution. The child process inherits
     * all environment variables of the parent process.
     *
     * @param command the command to execute
     * @return process exit value
     * @throws ExecuteException execution of subprocess failed
     */
    int execute(CommandLine command)
        throws ExecuteException, IOException;

    /**
     * Methods for starting synchronous execution.
     *
     * @param command the command to execute
     * @param environment The environment for the new process. If null, the
     *          environment of the current process is used.
     * @return process exit value
     * @throws ExecuteException execution of subprocess failed
     */
    int execute(CommandLine command, Map environment)
        throws ExecuteException, IOException;
    
    /**
     * Methods for starting asynchronous execution. The child process inherits
     * all environment variables of the parent process. Result provided to
     * callback handler.
     *
     * @param command the command to execute
     * @param handler capture process termination and exit code
     * @throws ExecuteException execution of subprocess failed
     */
    void execute(CommandLine command, ExecuteResultHandler handler)
        throws ExecuteException, IOException;

    /**
     * Methods for starting asynchronous execution. The child process inherits
     * all environment variables of the parent process. Result provided to
     * callback handler.
     *
     * @param command the command to execute
     * @param environment The environment for the new process. If null, the
     *          environment of the current process is used.
     * @param handler capture process termination and exit code 
     * @throws ExecuteException execution of subprocess failed     
     */
    void execute(CommandLine command, Map environment, ExecuteResultHandler handler)
        throws ExecuteException, IOException;
}
