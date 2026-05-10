package srvwww;

// Required import to verify the existence of a file
import java.io.IOException;
import java.io.RandomAccessFile;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;

class FileSystemActions {

    public boolean directoryExists(String path) {
        if (path == null || path.isEmpty()) {
            return false;
        }
        File file = new File(path);
        return file.exists() && file.isDirectory();
    }

    public boolean fileExists(String path) {
        if (path == null || path.isEmpty()) {
            return false;
        }
        File file = new File(path);
        return file.exists() && file.isFile();
    }

    public long getFileSize(String path) {
        if (path == null || path.isEmpty()) {
            return -1;
        }
        File file = new File(path);
        if (file.exists() && file.isFile()) {
            return file.length();
        }
        return 0;
    }

    public byte[] getChunk(String path, long offset, int length) {
        RandomAccessFile reader = null;

        if (path == null || path.isEmpty()) {
            return null;
        }
        try {
            reader = new RandomAccessFile(path, "r");
            if (offset < 0 || length <= 0) {
                throw new IOException("The offset is less than 0 or the chunk size is less than or equal to 0");
            }
            reader.seek(offset);
            if ((offset + length) > reader.length()) {
                length = (int) (reader.length() - offset); // Adjust chunk size if it exceeds the file size  
            }
            byte[] buffer = new byte[length];
            int bytesRead = reader.read(buffer, 0, length);
            if (bytesRead == -1) {
                reader.close();
                return null; // No bytes read, possibly the offset is greater than the file size
            }
            if (bytesRead < length) {
                byte[] tempBuffer = new byte[bytesRead];
                System.arraycopy(buffer, 0, tempBuffer, 0, bytesRead);
                buffer = tempBuffer; // Adjust buffer size if fewer bytes than requested were read
            }
            reader.close();
            return buffer;
        } catch (IOException e) {
            e.printStackTrace();
            return null; // Error reading the file
        }
    }
}