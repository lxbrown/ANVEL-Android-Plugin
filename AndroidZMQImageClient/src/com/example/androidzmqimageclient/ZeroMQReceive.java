package com.example.androidzmqimageclient;

import android.os.Handler;

import org.jeromq.ZMQ;

//Receive image data
public class ZeroMQReceive implements Runnable {
    private final Handler uiThreadHandler;
    String ip;
    boolean first;
    public static volatile String direction;
    //public static volatile double power;
    //public static volatile int angle;

    public ZeroMQReceive(Handler uiThreadHandler, String ip_) {
        this.uiThreadHandler = uiThreadHandler;
        ip = ip_;
        first = true;
        direction = "s";
    }

    @Override
    public void run() {
    	//Set up socket
        ZMQ.Context context = ZMQ.context(1);
        ZMQ.Socket socket = context.socket(ZMQ.PAIR);
        
        socket.connect("tcp://" + ip + ":9000");
        
        while(!Thread.currentThread().isInterrupted()) {
        	//Read the data into a byte array
            byte[] msg = socket.recv(0);
            
            //String send = power + "_" + angle;
            
            //socket.send(direction.getBytes());
            
            //Pass message to MainActivity
            uiThreadHandler.sendMessage(
                    Util.bundledMessage(uiThreadHandler, msg));
        }
        
        socket.close();
        context.term();
    }
}