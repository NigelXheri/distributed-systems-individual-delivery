package client;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.concurrent.ArrayBlockingQueue;

/*
The ClientImpl class exposes the following methods via RMI:

setXXXXXXXXXX(): these will be invoked from the WWW server to respond to each client message.

Additionally, it provides the getXXXXXXXXXX() methods so they can be called
from the client's processDownloads() method, in order to obtain the response to the last message sent via the Rabbit queue.

These methods are executed in different threads (e.g., setXXXXXXXXX() is
executed in the thread that handles RMI requests, while getXXXXXXXXX() is
executed in the client thread. They communicate with each other through
a blocking queue. getXXXXXXXX() attempts to read from that queue, and if it is empty,
it blocks. setXXXXXXXXX() puts the response to the last message sent to the 
Rabbit queue into this queue, thus unblocking the thread that was waiting in 
getXXXXXXXXX()).
*/

public class ClientImpl extends UnicastRemoteObject implements ClientInterface {
    private ArrayBlockingQueue<Object> queue; // Blocking queue for thread communication

    public ClientImpl() throws RemoteException {
        super();
        // The queue through which the threads executing the methods of this class communicate
        queue = new ArrayBlockingQueue<Object>(1); // A size of 1 is sufficient
    }

    @Override
    public void setExistFile(Boolean answer) throws RemoteException {
        // This method is invoked via RMI from the WWW server
        // It receives the response to the query and simply saves it in the internal queue
        try {
            // TODO
            queue.put(answer);
            
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void setGetSize(Long val) throws RemoteException {
        // This method is invoked via RMI from the WWW server
        // It receives the response to the query and simply saves it in the internal queue
        try {
            // TODO
            queue.put(val);

        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void setGetChunk(byte[] chunk) throws RemoteException {
        // This method is invoked via RMI from the WWW server
        // It receives the response to the query and simply saves it in the internal queue
        try {
            // TODO
            queue.put(chunk);
            
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    public Boolean getExistFile() throws InterruptedException {
        // This method is invoked from the client (processDownloads method)
        // It waits for the response to the last query to appear and returns its value
        // TODO
        return (Boolean) queue.take();
        
    }


    public Long getSize() throws InterruptedException {
        // This method is invoked from the client (processDownloads method)
        // It waits for the response to the last query to appear and returns its value
        // TODO
        return (Long) queue.take();
        
    }    

    public byte[] getChunk() throws InterruptedException {
        // This method is invoked from the client (processDownloads method)
        // It waits for the response to the last query to appear and returns its value
        // TODO
        return (byte[]) queue.take();
        
    }    

}