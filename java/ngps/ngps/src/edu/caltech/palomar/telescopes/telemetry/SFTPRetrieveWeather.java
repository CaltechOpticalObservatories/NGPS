/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import com.ibm.inifile.IniFile;
import java.io.File;
import java.util.regex.Pattern;
import org.apache.commons.vfs2.AllFileSelector;
import org.apache.commons.vfs2.FileObject;
import org.apache.commons.vfs2.FileSystem;
import org.apache.commons.vfs2.FileSystemException;
import org.apache.commons.vfs2.FileSystemManager;
import org.apache.commons.vfs2.FileSystemOptions;
import org.apache.commons.vfs2.FileType;
import org.apache.commons.vfs2.Selectors;
import org.apache.commons.vfs2.UserAuthenticator;
import org.apache.commons.vfs2.VFS;
import org.apache.commons.vfs2.auth.StaticUserAuthenticator;
import org.apache.commons.vfs2.impl.DefaultFileSystemConfigBuilder;
import org.apache.commons.vfs2.impl.DefaultFileSystemManager;
import org.apache.commons.vfs2.impl.StandardFileSystemManager;
import org.apache.commons.vfs2.provider.local.LocalFile;
import org.apache.commons.vfs2.provider.sftp.SftpFileSystemConfigBuilder;
// import of the Jsch library
import com.jcraft.jsch.Channel;
import com.jcraft.jsch.ChannelExec;
import com.jcraft.jsch.Session;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.ChannelSftp;
import com.jcraft.jsch.UIKeyboardInteractive;
import com.jcraft.jsch.UserInfo;
import static edu.caltech.palomar.telescopes.telemetry.ScpFrom.checkAck;
import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.security.Security;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JTextField;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

/**
 *
 * @author developer
 */
public class SFTPRetrieveWeather {
    private FileSystemManager fsManager = null;
    private FileSystemOptions opts      = null;
    private FileObject        src       = null; // used for cleanup in release()
    private FileObject        sftpFile;
    public  IniFile           myIniFile;
    public   java.lang.String USERDIR          = System.getProperty("user.dir");
    public   java.lang.String SEP              = System.getProperty("file.separator");
    public   java.lang.String CONFIG           = new java.lang.String("config");
    public   java.lang.String FILE_NAME        = new java.lang.String();
    public   java.lang.String HOST_NAME        = new java.lang.String();
    public   java.lang.String USER_NAME        = new java.lang.String();
    public   java.lang.String PASSWORD         = new java.lang.String();
    public   java.lang.String LOCAL_ROOT_DIR   = new java.lang.String();
    public   java.lang.String LOCAL_DIR        = new java.lang.String();
    public   java.lang.String REMOTE_DIRECTORY = new java.lang.String();                
    public   java.lang.String KNOWN_HOSTS      = new java.lang.String();
    public   static int                        DIR_TIME_OFFSET = 5;
    public   static int                        NOT_RETRIEVED    = 100;
    public   static int                        RETRIEVE_ERROR   = 102;
    public   static int                        RETRIEVED        = 104;
    public   java.lang.String                  ERROR = new java.lang.String("ERROR");
/*================================================================================================
/     SFTPRetrieve()
/=================================================================================================*/
    public SFTPRetrieveWeather(){        
        readConfiguration();
        boolean isTest = false;
        if(isTest){
            test();
        }
    }
/*================================================================================================
/     initialize()
/=================================================================================================*/
public void test(){
//   java.lang.String retrieved_file_1 = retrieveFileJSCH(REMOTE_DIRECTORY+SEP+"weather2001.txt"); 
   retrieveFileSCP("dome2001.txt");
 //  java.lang.String retrieved_file_2 = retrieveFile(REMOTE_DIRECTORY+SEP+"weather2001.txt"); 
}    
/*================================================================================================
/     initialize()
/=================================================================================================*/
private void readConfiguration(){
    try{
    java.lang.String sectionName = new java.lang.String();
    sectionName = "CONNECTIONS";
       FILE_NAME = USERDIR+SEP+CONFIG+SEP+"sftp_settings.ini";
       myIniFile = new com.ibm.inifile.IniFile(FILE_NAME);
       myIniFile.setSectionized(true); 
       HOST_NAME                 = myIniFile.getProperty("HOST_NAME",sectionName);
       USER_NAME                 = myIniFile.getProperty("USER_NAME",sectionName);
       PASSWORD                  = myIniFile.getProperty("PASSWORD",sectionName);  
       LOCAL_ROOT_DIR            = myIniFile.getProperty("LOCAL_ROOT_DIR",sectionName);
       REMOTE_DIRECTORY          = myIniFile.getProperty("REMOTE_DIRECTORY",sectionName);
       KNOWN_HOSTS               = myIniFile.getProperty("KNOWN_HOSTS",sectionName);
       LOCAL_DIR                 = LOCAL_ROOT_DIR;    
    }catch(Exception e){
        System.out.println(e.toString());
    }   
}
/*================================================================================================
/     process()
/=================================================================================================*/
public java.lang.String retrieveFile(java.lang.String absolute_file_name){
   java.lang.String response = new java.lang.String();
   StandardFileSystemManager manager = new StandardFileSystemManager();
   try{       
        String file_name = absolute_file_name.substring(absolute_file_name.lastIndexOf(SEP)+1,absolute_file_name.length());
        manager.init();
        File       local_file = new File(LOCAL_DIR+file_name);
        FileObject local_file_object  = manager.resolveFile(local_file.getAbsolutePath());
        FileObject remote_file_object = manager.resolveFile(createConnectionString(HOST_NAME,USER_NAME,PASSWORD,absolute_file_name),createDefaultOptions());
        local_file_object.copyFrom(remote_file_object,Selectors.SELECT_SELF);
        response = LOCAL_DIR + SEP + file_name;
   }catch(Exception e){
       System.out.println(e.toString());
       response = "ERROR"; 
   }
   finally{
      manager.close();
   }   
   return response; 
}  
/*================================================================================================
/     process()
/=================================================================================================*/
public java.lang.String retrieveFileJSCH(java.lang.String absolute_file_name){
   java.lang.String response = new java.lang.String();
   long start = System.currentTimeMillis();
   JSch ssh;
   Session session;
   ChannelSftp sftpchannel;
   try{
        ssh = new JSch();
//        String knownHostPublicKey = "weather.palomar.caltech.edu,198.202.125.157 ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAIEAu/KADvggbp9dFV0tMDIrjt0vzgPV2fyfJ/OM5d6Id5jrhr2W8UZNojj8ccTzsiRjwUlFu7yn+Jd8tnj0CVfVGvYEQY/lRXUPOzVjBLrKKb33kuRLST9AHRvGsqLxmAzL9S00EUYGlhnwfLhwwyb9fmruF43e967966sGAsjsF8s=";
//        ssh.setKnownHosts(new java.io.ByteArrayInputStream(knownHostPublicKey.getBytes()));
        ssh.setKnownHosts("/home/developer/.ssh/known_hosts");
//        Security.insertProviderAt(new BouncyCastleProvider(), 1);
        session = ssh.getSession(USER_NAME,HOST_NAME,22);
        session.setPassword(PASSWORD);
        java.util.Properties config = new java.util.Properties();
        config.put("StrictHostKeyChecking", "no");
        session.setConfig(config);
        String kex = session.getConfig("kex");
        System.out.println("old kex:" + kex);
        kex = kex.replace(",diffie-hellman-group-exchange-sha1", "");
        session.setConfig("kex", kex);
//        session.setConfig("StrictHostKeyChecking", "no");
        System.out.println("new kex:" + session.getConfig("kex"));     

 //       session.setConfig(config);
        session.connect();
        sftpchannel = (ChannelSftp)session.openChannel("sftp");
        sftpchannel.connect();
        byte[] buffer = new byte[1024];
        BufferedInputStream bis = new BufferedInputStream(sftpchannel.get(absolute_file_name));
        String file_name = absolute_file_name.substring(absolute_file_name.lastIndexOf(SEP)+1,absolute_file_name.length());
        File                 local_file  = new File(LOCAL_DIR+SEP+file_name);
                             response    = local_file.getAbsolutePath();
        OutputStream         os          = new FileOutputStream(local_file);
        BufferedOutputStream bos         = new BufferedOutputStream(os);
        int readCount;
        while((readCount = bis.read(buffer))>0){
           bos.write(buffer,0,readCount);
        }
        bis.close();
        bos.close();        
        sftpchannel.disconnect();
        session.disconnect();
   }catch(Exception e){
      response = ERROR;
      e.printStackTrace();
   }   
   finally{   
       
   }   
   long end = System.currentTimeMillis();
   System.out.println("File Retrieve Time = "+(end - start));
   return response; 
    
}
/*================================================================================================
/     process()
/=================================================================================================*/
public static String createConnectionString(String hostName,String username, String password, String remoteFilePath) {
        return "sftp://" + username + ":" + password + "@" + hostName + remoteFilePath;
}
//  Method to setup default SFTP config:
/*================================================================================================
/     process()
/=================================================================================================*/
public static FileSystemOptions createDefaultOptions() throws FileSystemException {
    // Create SFTP options
    FileSystemOptions opts = new FileSystemOptions();
    // SSH Key checking
    SftpFileSystemConfigBuilder.getInstance().setStrictHostKeyChecking(opts, "no");
    // Root directory set to user home
    SftpFileSystemConfigBuilder.getInstance().setUserDirIsRoot(opts, true);
    // Timeout is count by Milliseconds
    SftpFileSystemConfigBuilder.getInstance().setTimeout(opts, 10000);
    return opts;
}
/*================================================================================================
/     process()
/=================================================================================================*/
public void retrieveFileSCP(java.lang.String remote_file){
    FileOutputStream fos=null;
    try{
      String rfile = REMOTE_DIRECTORY+SEP+remote_file;
      String lfile = LOCAL_ROOT_DIR+SEP+remote_file;
      String prefix=null;
      if(new File(lfile).isDirectory()){ 
        prefix=lfile+File.separator;
      }
      JSch jsch=new JSch();
      jsch.setKnownHosts(KNOWN_HOSTS);
      Session session=jsch.getSession(USER_NAME,HOST_NAME, 22);
      // username and password will be given via UserInfo interface.
      UserInfo ui=new MyUserInfo();
      session.setUserInfo(ui);
      session.setPassword(PASSWORD);
      
      String kex = session.getConfig("kex");
      System.out.println("old kex:" + kex);
      kex = kex.replace(",diffie-hellman-group-exchange-sha1", "");
      session.setConfig("kex", kex);
      java.util.Properties config = new java.util.Properties();
      config.put("StrictHostKeyChecking", "no");
      System.out.println("new kex:" + session.getConfig("kex"));      
            
      session.connect();

      // exec 'scp -f rfile' remotely
      rfile=rfile.replace("'", "'\"'\"'");
      rfile="'"+rfile+"'";
      String command="scp -f "+rfile;
      Channel channel=session.openChannel("exec");
      ((ChannelExec)channel).setCommand(command);
      // get I/O streams for remote scp
      OutputStream out=channel.getOutputStream();
      InputStream in=channel.getInputStream();

      channel.connect();

      byte[] buf=new byte[1024];

      // send '\0'
      buf[0]=0; out.write(buf, 0, 1); out.flush();

      while(true){
	int c=checkAck(in);
        if(c!='C'){
	  break;
	}
        in.read(buf, 0, 5);

        long filesize=0L;
        while(true){
          if(in.read(buf, 0, 1)<0){
            // error
            break; 
          }
          if(buf[0]==' ')break;
          filesize=filesize*10L+(long)(buf[0]-'0');
        }
        String file=null;
        for(int i=0;;i++){
          in.read(buf, i, 1);
          if(buf[i]==(byte)0x0a){
            file=new String(buf, 0, i);
            break;
  	  }
        }
        buf[0]=0; out.write(buf, 0, 1); out.flush();
        // read a content of lfile
        fos=new FileOutputStream(prefix==null ? lfile : prefix+file);
        int foo;
        while(true){
          if(buf.length<filesize) foo=buf.length;
	  else foo=(int)filesize;
          foo=in.read(buf, 0, foo);
          if(foo<0){
            // error 
            break;
          }
          fos.write(buf, 0, foo);
          filesize-=foo;
          if(filesize==0L) break;
        }
        fos.close();
        fos=null;

	if(checkAck(in)!=0){
	  System.exit(0);
	}

        // send '\0'
        buf[0]=0; out.write(buf, 0, 1); out.flush();
      }

      session.disconnect();

    }
    catch(Exception e){
      System.out.println(e);
      try{if(fos!=null)fos.close();}catch(Exception ee){}
    }   
}
 public static class MyUserInfo implements UserInfo, UIKeyboardInteractive{
    public String getPassword(){ return passwd; }
    public boolean promptYesNo(String str){
      Object[] options={ "yes", "no" };
      int foo=JOptionPane.showOptionDialog(null, 
             str,
             "Warning", 
             JOptionPane.DEFAULT_OPTION, 
             JOptionPane.WARNING_MESSAGE,
             null, options, options[0]);
       return foo==0;
    }
  
    String passwd;
    JTextField passwordField=(JTextField)new JPasswordField(20);

    public String getPassphrase(){ return null; }
    public boolean promptPassphrase(String message){ return true; }
    public boolean promptPassword(String message){
      Object[] ob={passwordField}; 
      int result=
	  JOptionPane.showConfirmDialog(null, ob, message,
					JOptionPane.OK_CANCEL_OPTION);
      if(result==JOptionPane.OK_OPTION){
	passwd=passwordField.getText();
	return true;
      }
      else{ return false; }
    }
    public void showMessage(String message){
      JOptionPane.showMessageDialog(null, message);
    }
    final GridBagConstraints gbc = 
      new GridBagConstraints(0,0,1,1,1,1,
                             GridBagConstraints.NORTHWEST,
                             GridBagConstraints.NONE,
                             new Insets(0,0,0,0),0,0);
    private Container panel;
    public String[] promptKeyboardInteractive(String destination,
                                              String name,
                                              String instruction,
                                              String[] prompt,
                                              boolean[] echo){
      panel = new JPanel();
      panel.setLayout(new GridBagLayout());

      gbc.weightx = 1.0;
      gbc.gridwidth = GridBagConstraints.REMAINDER;
      gbc.gridx = 0;
      panel.add(new JLabel(instruction), gbc);
      gbc.gridy++;

      gbc.gridwidth = GridBagConstraints.RELATIVE;

      JTextField[] texts=new JTextField[prompt.length];
      for(int i=0; i<prompt.length; i++){
        gbc.fill = GridBagConstraints.NONE;
        gbc.gridx = 0;
        gbc.weightx = 1;
        panel.add(new JLabel(prompt[i]),gbc);

        gbc.gridx = 1;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.weighty = 1;
        if(echo[i]){
          texts[i]=new JTextField(20);
        }
        else{
          texts[i]=new JPasswordField(20);
        }
        panel.add(texts[i], gbc);
        gbc.gridy++;
      }

      if(JOptionPane.showConfirmDialog(null, panel, 
                                       destination+": "+name,
                                       JOptionPane.OK_CANCEL_OPTION,
                                       JOptionPane.QUESTION_MESSAGE)
         ==JOptionPane.OK_OPTION){
        String[] response=new String[prompt.length];
        for(int i=0; i<prompt.length; i++){
          response[i]=texts[i].getText();
        }
	return response;
      }
      else{
        return null;  // cancel
      }
    }
  }


    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
       new SFTPRetrieveWeather(); //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(SFTPRetrieveWeather.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>
    }
}
