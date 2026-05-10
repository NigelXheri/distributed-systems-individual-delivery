package srvwww;

// Imports required for RabbitMQ
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.Consumer;
import com.rabbitmq.client.DefaultConsumer;
import com.rabbitmq.client.Envelope;
import com.rabbitmq.client.AMQP;

// Imports required for RMI
import java.io.IOException;
import java.rmi.Naming;
import java.rmi.RemoteException;

// Imports required for file reading
import java.io.File;
import java.io.FileWriter;
import java.io.FileReader;
import java.io.BufferedWriter;
import java.io.BufferedReader;

// Imports required for getting current date and time
import java.util.Date;

// Imports for working with lists
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.ListIterator;

// Blocking queue to communicate between the QueryReceiver thread and Worker threads
import java.util.concurrent.ArrayBlockingQueue;

// Import required to access client methods
import client.ClientInterface;

// ===================================================================
// The following two classes are threads that will run concurrently:
//
// - QueryReceiver is the thread that waits for messages from RabbitMQ and inserts
//   the received messages into the internal queue (blocking queue).
// - Worker are the threads that read from the internal queue and resolve requests.

// QueryReceiver Class. Receives messages via RabbitMQ and puts them into the blocking 
// event queue to be processed by the Worker threads.
class QueryReceiver extends Thread {
    private final static String RABBIT_QUEUE_NAME = "nx"; // TODO (change queue name)
    private ArrayBlockingQueue<String> qrequest;  // Blocking queue to communicate with workers

    // The constructor receives a reference to the blocking queue
    // which allows communication with the Worker threads.
    public QueryReceiver(ArrayBlockingQueue<String> qrequest) {
        this.qrequest = qrequest;
    }

    // The run function is executed when the thread starts
    public void run() {
        ConnectionFactory factory = new ConnectionFactory();
        factory.setHost("localhost");
        try {
            // Connect to RabbitMQ
            // TODO
            Connection connection = factory.newConnection();
            Channel channel = connection.createChannel();
            channel.queueDeclare(RABBIT_QUEUE_NAME, false, false, false, null);
            


            // Wait for requests in the RabbitMQ queue
            Consumer consumer = new DefaultConsumer(channel) {
                @Override
                public void handleDelivery(String consumerTag, Envelope envelope, AMQP.BasicProperties properties,
                        byte[] body)
                        throws IOException {
                    // ************************************************************
                    // Reception and handling of the message arriving via RabbitMQ
                    // ************************************************************

                    // Convert the received message into a string and put it in the blocking queue
                    String msg = new String(body, "UTF-8");
                    System.out.println("QueryReceiver: Received query = " + msg);

                    // TODO
                    try {
                        qrequest.put(msg);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }  

                }
            };
            System.out.println("QueryReceiver: Waiting for WWW requests to arrive");
            // Start waiting for messages in the RabbitMQ queue
            // TODO
            channel.basicConsume(RABBIT_QUEUE_NAME, true, consumer);


        } catch (Exception e) { // No exception handling, simply abort
            e.printStackTrace();
            System.exit(7);
        }
    }
}
// Worker class waits in a blocking queue for the QueryReceiver thread to
// send a query to be processed.

class Worker extends Thread {

    private ArrayBlockingQueue<String> queue; // Blocking queue to communicate with QueryReceiver thread
    private FileSystemActions fsa;           // Object to perform actions on files and folders
    private String droot;                    // Root directory from which download requests will be served
    private String log_file;                 // Name of the log file

    // The constructor receives a reference to the blocking queue 
    // and the server configuration parameters

    public Worker(ArrayBlockingQueue<String> queue, String droot, String log_filename) {
        this.queue = queue;
        this.droot = droot;
        this.log_file = log_filename;
        this.fsa = new FileSystemActions();
    }

    // The run method is executed when the thread starts
    public void run() {

        String request = "";

        try {
            while (true) { // Infinite loop
                // Wait for request in the blocking queue
                // TODO
                request = queue.take();

                // Process query by splitting its fields by the space character " "
                String parts[] = request.split(" ");
                if (parts.length < 3 || parts.length > 5) {
                    System.out.println(
                            String.format("Request ignored for not having the expected format '%s'", request));
                } else {
                    try {

                        // Locate the RMI client to which the response must be returned. 
                        // Its name is in the first part of the message received from the queue.
                        // TODO
                        ClientInterface client = (ClientInterface) Naming.lookup(parts[0]);

                        if (parts[1].equals("EXISTFILE"))
                        {
                            // Check if the file exists
                            if (fsa.fileExists(droot + "/" + parts[2])) {
                                // TODO
                                client.setExistFile(true);

                            } else {
                                // TODO
                                client.setExistFile(false);

                            }
                        } else if (parts[1].equals("GETSIZE")) {
                            // Get the file size
                            // TODO
                            Long size = fsa.getFileSize(droot + "/" + parts[2]);
                            client.setGetSize(size);

                        } else if (parts[1].equals("GETCHUNK")) {
                            long offset = Long.parseLong(parts[3]);
                            int length = Integer.parseInt(parts[4]);
                            // TODO
                            byte[] chunk = fsa.getChunk(droot + "/" + parts[2], offset, length);
                            client.setGetChunk(chunk);
                        }
                    } catch (Exception e) {
                        // Any exception is simply printed
                        System.out.println("Error in SrvWWW when returning response to client: " + e.getMessage());
                        e.printStackTrace();
                    }

                    // Register the query in the log file
                    Date date = new Date();
                    try {
                        String log_line;
                        // Prepare the line to be written
                        // TODO
                        log_line = date.toString() + "," + request + "\n";

                        BufferedWriter bw = new BufferedWriter(new FileWriter(log_file, true));
                        bw.write(log_line);
                        bw.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(8);
        }
    }
}

// Main class that instantiates the receiver thread and the worker threads,
// and starts them up.
public class SrvWWW {

    public static void main(String[] argv) throws Exception {

        String log_filename = null; // To register the processed queries
        FileSystemActions fsa = new FileSystemActions();   

        // The values of these variables are read from the command line
        int queue_size = 0; // Size of the array blocking queue
        int num_workers = 0; // Number of worker threads

        // Internal synchronization queue between the QueryReceiver thread and the Workers
        ArrayBlockingQueue<String> internal_queue;

        // Variable representing the event receiver thread
        QueryReceiver query_receiver;
        // Variable representing the worker threads
        Worker[] workers;

        // Command line reading
        //
        if (argv.length < 4) {
            System.out.println("Usage: SrvWWW <queue_size> <num_workers> <document_root> <log_file>");
            System.exit(1);
        }
        try {
            queue_size = Integer.parseInt(argv[0]);
            num_workers = Integer.parseInt(argv[1]);
        } catch (NumberFormatException e) {
            System.out.println("The arguments queue_size and num_workers must be integers");
            System.exit(2);
        }

        if (queue_size < 1) {
            System.out.println("queue_size must be a value >= 1");
            System.exit(5);
        }
        if (num_workers < 1) {
            System.out.println("num_workers must be a value >= 1");
            System.exit(6);
        }

        if (!fsa.directoryExists(argv[2])) {
            System.out.println("The directory " + argv[2] + " does not exist");
            System.exit(7);
        }
        log_filename = argv[3];

        // First, the internal synchronization queue between threads is created
        // TODO
        internal_queue = new ArrayBlockingQueue<String>(queue_size);

        // Create the Worker threads and start them
        workers = new Worker[num_workers];
        // TODO
        for (int i = 0; i < num_workers; i++) {
            workers[i] = new Worker(internal_queue, argv[2], log_filename);
            workers[i].start();
        }

        // Create the query receiver thread and start it
        query_receiver = new QueryReceiver(internal_queue);
        query_receiver.start();

        // We wait for the event receiver thread to finish (it never will,
        // it must be stopped with Ctrl+C)
        query_receiver.join();
    }
}