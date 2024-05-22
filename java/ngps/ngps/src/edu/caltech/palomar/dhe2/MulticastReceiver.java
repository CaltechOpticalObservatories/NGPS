package edu.caltech.palomar.dhe2;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.DatagramSocket;
import java.net.DatagramPacket;

public class MulticastReceiver {
    protected MulticastSocket socket = null;
    protected byte[] buf = new byte[256];

    public MulticastReceiver(){
       run(); 
    }
    
    public void run() {
       try{
       socket = new MulticastSocket(1300);
        InetAddress group = InetAddress.getByName("239.1.1.234");
        socket.joinGroup(group);
        while (true) {
            DatagramPacket packet = new DatagramPacket(buf, buf.length);
            socket.receive(packet);
            String received = new String(packet.getData(), 0, packet.getLength());
            if ("end".equals(received)) {
                break;
            }
            System.out.println(received);
        }
        socket.leaveGroup(group);
        socket.close();           
       }catch(Exception e){
           
       } 
        
 
    }
       public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         *
        //</editor-fold>

        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
               new MulticastReceiver();
            }
        });
    }

}
