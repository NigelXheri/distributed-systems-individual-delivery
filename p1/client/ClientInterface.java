package client;

import java.rmi.Remote;
import java.rmi.RemoteException;

/*
Remote interface of the Client that must make the setXXXXXXX() methods public so that the www server can invoke them via RMI.
*/
public interface ClientInterface extends Remote {
    // TODO
    void setExistFile(Boolean answer) throws RemoteException;
    void setGetSize(Long val) throws RemoteException;
    void setGetChunk(byte[] chunk) throws RemoteException;

}